#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

Napi::Function JSSentencepieceTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "SentencepieceTokenizer",
        { InstanceMethod("tokenize", &JSSentencepieceTokenizer::tokenize, napi_enumerable),
            InstanceMethod("encode", &JSSentencepieceTokenizer::encode, napi_enumerable),
            InstanceMethod("decode", &JSSentencepieceTokenizer::decode, napi_enumerable) });
}

std::string escapeRegex(const std::string& str)
{
    static const std::regex escape(R"([.^$|()\[\]{}*+?\\])");
    static const std::string format(R"(\\&)");

    return std::regex_replace(str, escape, format, std::regex_constants::format_sed);
}

JSSentencepieceTokenizer::JSSentencepieceTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSSentencepieceTokenizer>(info)
{
    static const char* special_tokens[] = {
        "bos_token", "eos_token", "unk_token", "pad_token", "mask_token", "sep_token"
    };

    sentence_piece_.LoadFromSerializedProto(NodeValue(info[0]));

    NodeOpt opt(info[1]);

    add_bos_token = opt.Get("add_bos_token", false);
    add_eos_token = opt.Get("add_eos_token", false);

    std::unordered_map<std::string, SpecialTokens> added_tokens_decoder;
    added_tokens_decoder = opt.Get("added_tokens_decoder", added_tokens_decoder);

    for (auto& [key, value] : added_tokens_decoder) {
        uint32_t id = std::stoul(key);

        auto it = id_to_token.emplace(id, value.content);
        if (value.content.length() > 0)
            token_to_id[it.first->second] = id;
    }

    bos_token = opt.Get("bos_token", std::string("<s>"));
    eos_token = opt.Get("eos_token", std::string("</s>"));
    unk_token = opt.Get("unk_token", std::string("<unk>"));
    pad_token = opt.Get("pad_token", std::string("<pad>"));

    bos_id = convert_token_to_id(bos_token);
    eos_id = convert_token_to_id(eos_token);
    unk_id = convert_token_to_id(unk_token);
    pad_id = convert_token_to_id(pad_token);

    if (token_to_id.size() == 0) {
        if (unk_token.length() > 0) {
            token_to_id[unk_token] = unk_id;
            id_to_token[unk_id] = unk_token;
        }

        if (bos_token.length() > 0) {
            token_to_id[bos_token] = bos_id;
            id_to_token[bos_id] = bos_token;
        }

        if (eos_token.length() > 0) {
            token_to_id[eos_token] = eos_id;
            id_to_token[eos_id] = eos_token;
        }
    }

    offset = opt.Get("offset", 0);
    legacy = opt.Get("legacy", true);

    if (token_to_id.size() > 0) {
        std::string pattern_str;

        for (auto& [key, value] : token_to_id) {
            if (pattern_str.length() > 0)
                pattern_str += "|";
            pattern_str += escapeRegex(std::string(key));
        }

        has_pattern = true;
        pattern = std::regex("\\s?(" + pattern_str + ")\\s?");
    }
}

Napi::Value JSSentencepieceTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text = NodeValue(info[0]);
    std::vector<std::string> tokens;

    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);
    size_t added_tokens_size = added_tokens.size();

    tokens.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++)
        tokens[i] = spt.pieces(i).piece();

    return NodeValue(info.Env(), tokens);
}

int JSSentencepieceTokenizer::convert_token_to_id(std::string_view token)
{
    auto it = token_to_id.find(token);
    if (it != token_to_id.end())
        return it->second;

    return sentence_piece_.PieceToId(token);
}

void JSSentencepieceTokenizer::push_token(std::string_view token, std::vector<int>* ids)
{
    ids->push_back(convert_token_to_id(token));
}

void JSSentencepieceTokenizer::push_token(std::string_view token, std::vector<std::string_view>* ids)
{
    ids->push_back(token);
}

static void push_piece(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<int>* ids)
{
    ids->emplace_back(piece.id());
}

static void push_piece(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<std::string_view>* ids)
{
    ids->emplace_back(piece.piece());
}

template <typename T>
void JSSentencepieceTokenizer::sentencepiece_encode(char* text, size_t size, std::vector<T>* ids)
{
    int32_t start = 0;
    if (!legacy && ids->size() > (add_bos_token ? 1 : 0)) {
        *--text = '-';
        size++;
        start = 1;
    }

    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(std::string_view(text, size), &spt);

    for (; start < spt.pieces_size(); start++)
        push_piece(spt.pieces(start), ids);
}

template <typename T>
void JSSentencepieceTokenizer::encode(std::string& text, std::vector<T>* ids)
{
    size_t lastPos = 0;

    if (has_pattern) {
        std::smatch m;
        std::string::const_iterator searchStart(text.cbegin());

        while (std::regex_search(searchStart, text.cend(), m, pattern)) {
            size_t pos = m[0].first - text.cbegin();

            if (pos != lastPos) {
                sentencepiece_encode(text.data() + lastPos, pos - lastPos, ids);
            }

            pos = m[1].first - text.cbegin();
            std::string_view token(text.data() + pos, m[1].length());
            push_token(token, ids);

            searchStart = m[0].first + m[0].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size()) {
        sentencepiece_encode(text.data() + lastPos, text.size() - lastPos, ids);
    }
}

Napi::Value JSSentencepieceTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = NodeValue(info[0]);
    std::vector<int> ids;

    if (add_bos_token)
        ids.emplace_back(bos_id);

    encode(text, &ids);

    if (add_eos_token) {
        if (ids.size() == 0 || ids[ids.size() - 1] != eos_id)
            ids.emplace_back(eos_id);
    }

    // sentencepiece::SentencePieceText spt;
    // sentence_piece_.Encode(text, &spt);
    // size_t added_tokens_size = added_tokens.size();

    // ids.resize(spt.pieces_size());
    // for (int i = 0; i < spt.pieces_size(); i++) {
    //     auto piece = spt.pieces(i);

    //     if (added_tokens_size || offset) {
    //         int j;
    //         const std::string& txt = piece.piece();

    //         for (j = 0; j < added_tokens_size; j++) {
    //             if (txt == added_tokens[j]) {
    //                 ids[i] = j;
    //                 break;
    //             }
    //         }

    //         if (j == added_tokens_size) {
    //             uint32_t id = piece.id();
    //             ids[i] = id ? id + offset : unk_id;
    //         }
    //     } else
    //         ids[i] = piece.id();
    // }

    return NodeValue(info.Env(), ids);
}

Napi::Value JSSentencepieceTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int> ids = NodeValue(info[0]);
    std::string text;

    std::vector<std::string> pieces;
    const int num_pieces = sentence_piece_.GetPieceSize();
    pieces.reserve(ids.size());
    size_t added_tokens_size = added_tokens.size();

    for (const int id : ids) {
        if (id < 0 || id >= num_pieces + offset)
            pieces.emplace_back("");
        else if (id < added_tokens_size)
            pieces.emplace_back(added_tokens[id]);
        else
            pieces.emplace_back(sentence_piece_.IdToPiece(id - offset));
    }

    sentence_piece_.Decode(pieces, &text);

    return NodeValue(info.Env(), text);
}
