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
        "unk_token", "bos_token", "eos_token", "pad_token", "mask_token", "sep_token"
    };

    static const char* special_token_values[] = {
        "<unk>", "<s>", "</s>", "<pad>", "<mask>", "<sep>"
    };

    sentence_piece_.LoadFromSerializedProto(from_value<std::string_view>(info[0]));

    Napi::Config opt(info[1]);

    legacy = opt.Get("legacy", true);
    offset = opt.Get("offset", 0);

    std::unordered_map<std::string, SpecialToken> added_tokens_decoder;
    added_tokens_decoder = opt.Get("added_tokens_decoder", added_tokens_decoder);

    for (auto& [key, value] : added_tokens_decoder) {
        uint32_t id = std::stoul(key);

        auto it = id_to_token.emplace(id, value.content);
        if (value.content.length() > 0)
            token_to_id[it.first->second] = id;
    }

    int vacob_size = sentence_piece_.GetPieceSize();

    std::vector<std::string> additional_special_tokens;
    additional_special_tokens = opt.Get("additional_special_tokens", additional_special_tokens);
    for (auto& stoken : additional_special_tokens)
        if (stoken.length() > 0) {
            int id = convert_token_to_id(stoken);

            if (id == 0) {
                id = vacob_size++;
                auto it = id_to_token.emplace(id, stoken);
                token_to_id[it.first->second] = id;

                SpecialToken token(stoken);
                token.id = id;

                if (special_tokens_map.find(token.content) == special_tokens_map.end())
                    auto it = special_tokens_map.emplace(token.content, token);
            }
        }

    bos_id = sentence_piece_.bos_id();
    if (bos_id >= 0) {
        bos_token = sentence_piece_.IdToPiece(bos_id);
        bos_id = convert_token_to_id(bos_token);
    }

    eos_id = sentence_piece_.eos_id();
    if (eos_id >= 0) {
        eos_token = sentence_piece_.IdToPiece(eos_id);
        eos_id = convert_token_to_id(eos_token);
    }

    unk_id = sentence_piece_.unk_id();
    if (unk_id >= 0) {
        unk_token = sentence_piece_.IdToPiece(unk_id);
        unk_id = convert_token_to_id(unk_token);
    }

    for (int i = 0; i < sizeof(special_tokens) / sizeof(special_tokens[0]); i++) {
        const char* key(special_tokens[i]);
        Napi::Value value = opt.Get(key, Napi::Value());
        SpecialToken token;

        napi_valuetype type = value.Type();
        if (!value.IsUndefined() && !value.IsNull())
            token = value;

        if (token.content.length() > 0) {
            token.id = convert_token_to_id(token.content);
            switch (i) {
            case 0:
                unk_id = token.id;
                break;
            case 1:
                bos_id = token.id;
                break;
            case 2:
                eos_id = token.id;
                break;
            }

            if (special_tokens_map.find(token.content) == special_tokens_map.end())
                auto it = special_tokens_map.emplace(token.content, token);
        }
    }

    std::vector<std::string> config_tokens;

    config_tokens = opt.Get("prefix_tokens", config_tokens);
    for (auto& token : config_tokens)
        prefix_tokens.emplace_back(convert_token_to_id(token));

    add_bos_token = opt.Get("add_bos_token", false);
    if (add_bos_token)
        prefix_tokens.emplace_back(bos_id);

    config_tokens.clear();

    add_eos_token = opt.Get("add_eos_token", false);
    if (add_eos_token)
        suffix_tokens.emplace_back(eos_id);

    config_tokens = opt.Get("suffix_tokens", config_tokens);
    for (auto& token : config_tokens)
        suffix_tokens.emplace_back(convert_token_to_id(token));

    if (special_tokens_map.size() > 0) {
        std::string pattern_str;

        for (auto& [key, value] : special_tokens_map) {
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
    std::string text = from_value<std::string>(info[0]);
    std::vector<std::string> tokens;

    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);
    size_t added_tokens_size = added_tokens.size();

    tokens.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++)
        tokens[i] = spt.pieces(i).piece();

    return to_value(info.Env(), tokens);
}

int JSSentencepieceTokenizer::convert_token_to_id(std::string_view token)
{
    auto it = token_to_id.find(token);
    if (it != token_to_id.end())
        return it->second;

    return sentence_piece_.PieceToId(token);
}

static void push_token(const SpecialToken& token, std::vector<int>* ids)
{
    ids->push_back(token.id);
}

static void push_token(const SpecialToken& token, std::vector<std::string_view>* ids)
{
    ids->push_back(token.content);
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
    if (!legacy && ids->size() > (prefix_tokens.size() > 0 ? 1 : 0)) {
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
            const SpecialToken& token = special_tokens_map[m[1]];

            size_t pos = (token.lstrip ? m[0].first : m[1].first) - text.cbegin();
            if (pos != lastPos)
                sentencepiece_encode(text.data() + lastPos, pos - lastPos, ids);

            push_token(token, ids);

            searchStart = token.rstrip ? m[0].first + m[0].length() : m[1].first + m[1].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size())
        sentencepiece_encode(text.data() + lastPos, text.size() - lastPos, ids);
}

Napi::Value JSSentencepieceTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<int> ids;

    ids.insert(ids.end(), prefix_tokens.begin(), prefix_tokens.end());

    encode(text, &ids);

    if (add_eos_token && ids.size() > 0 && ids[ids.size() - 1] == eos_id)
        ids.insert(ids.end(), suffix_tokens.begin() + 1, suffix_tokens.end());
    else
        ids.insert(ids.end(), suffix_tokens.begin(), suffix_tokens.end());

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

    return to_value(info.Env(), ids);
}

Napi::Value JSSentencepieceTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int> ids = to_array<int>(info[0]);
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

    return to_value(info.Env(), text);
}
