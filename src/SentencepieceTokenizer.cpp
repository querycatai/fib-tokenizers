#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

napi_ref JSSentencepieceTokenizer::constructor;

napi_value JSSentencepieceTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "tokenize", nullptr, tokenize, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL(env, napi_define_class(env, "SentencepieceTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL(env, napi_create_reference(env, cons, 1, &constructor));

    return cons;
}

class AddedTokens {
public:
    AddedTokens()
    {
    }

    AddedTokens(const AddedTokens& other)
    {
        content = other.content;
        lstrip = other.lstrip;
        normalized = other.normalized;
        rstrip = other.rstrip;
        single_word = other.single_word;
        special = other.special;
    }

    AddedTokens(NodeValue value)
    {
        NodeOpt opt(value);

        content = opt.Get("content", std::string());
        lstrip = opt.Get("lstrip", false);
        normalized = opt.Get("normalized", false);
        rstrip = opt.Get("rstrip", false);
        single_word = opt.Get("single_word", false);
        special = opt.Get("special", false);
    }

public:
    std::string content;
    bool lstrip;
    bool normalized;
    bool rstrip;
    bool single_word;
    bool special;
};

std::string escapeRegex(const std::string& str)
{
    static const std::regex escape(R"([.^$|()\[\]{}*+?\\])");
    static const std::string format(R"(\\&)");

    return std::regex_replace(str, escape, format, std::regex_constants::format_sed);
}

JSSentencepieceTokenizer::JSSentencepieceTokenizer(NodeArg<JSSentencepieceTokenizer>& args)
    : env_(args.env())
{
    sentence_piece_.LoadFromSerializedProto(args[0]);

    NodeOpt opt(args[1]);

    add_bos_token = opt.Get("add_bos_token", false);
    add_eos_token = opt.Get("add_eos_token", false);

    std::unordered_map<std::string, AddedTokens> added_tokens_decoder;
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

    if (token_to_id.size() == 0) {
        if (unk_token.length() > 0) {
            token_to_id[unk_token] = 0;
            id_to_token[0] = unk_token;
        }

        if (bos_token.length() > 0) {
            token_to_id[bos_token] = 1;
            id_to_token[1] = bos_token;
        }

        if (eos_token.length() > 0) {
            token_to_id[eos_token] = 2;
            id_to_token[2] = eos_token;
        }
    }

    bos_id = convert_token_to_id(bos_token);
    eos_id = convert_token_to_id(eos_token);
    unk_id = convert_token_to_id(unk_token);
    pad_id = convert_token_to_id(pad_token);

    offset = opt.Get("offset", 0);

    if (token_to_id.size() > 0) {
        std::string pattern_str;

        for (auto& [key, value] : token_to_id) {
            if (pattern_str.length() > 0)
                pattern_str += "|";
            pattern_str += escapeRegex(std::string(key));
        }

        has_pattern = true;
        pattern = std::regex(pattern_str);
    }
}

napi_value JSSentencepieceTokenizer::tokenize(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> args(env, info);

    std::string text = args[0];
    std::vector<std::string> tokens;

    sentencepiece::SentencePieceText spt;
    args->sentence_piece_.Encode(text, &spt);
    size_t added_tokens_size = args->added_tokens.size();

    tokens.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++)
        tokens[i] = spt.pieces(i).piece();

    return NodeValue(env, tokens);
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

void JSSentencepieceTokenizer::sentencepiece_encode(std::string_view text, std::vector<int>* ids)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (const auto& sp : spt.pieces()) {
        ids->emplace_back(sp.id());
    }
}

void JSSentencepieceTokenizer::sentencepiece_encode(std::string_view text, std::vector<std::string_view>* ids)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (const auto& sp : spt.pieces()) {
        ids->emplace_back(sp.piece());
    }
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
                sentencepiece_encode(std::string_view(text.data() + lastPos, pos - lastPos), ids);
            }

            push_token(std::string_view(text.data() + pos, m[0].length()), ids);

            lastPos = pos + m[0].length();
            searchStart = m[0].first + m[0].length();
        }
    }

    if (lastPos < text.size()) {
        sentencepiece_encode(std::string_view(text.data() + lastPos, text.size() - lastPos), ids);
    }
}

napi_value JSSentencepieceTokenizer::encode(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> args(env, info);

    std::string text = args[0];
    std::vector<int> ids;

    if (args->add_bos_token)
        ids.emplace_back(args->bos_id);

    args->encode(text, &ids);

    if (args->add_eos_token)
        ids.emplace_back(args->eos_id);

    // sentencepiece::SentencePieceText spt;
    // args->sentence_piece_.Encode(text, &spt);
    // size_t added_tokens_size = args->added_tokens.size();

    // ids.resize(spt.pieces_size());
    // for (int i = 0; i < spt.pieces_size(); i++) {
    //     auto piece = spt.pieces(i);

    //     if (added_tokens_size || args->offset) {
    //         int j;
    //         const std::string& txt = piece.piece();

    //         for (j = 0; j < added_tokens_size; j++) {
    //             if (txt == args->added_tokens[j]) {
    //                 ids[i] = j;
    //                 break;
    //             }
    //         }

    //         if (j == added_tokens_size) {
    //             uint32_t id = piece.id();
    //             ids[i] = id ? id + args->offset : args->unk_id;
    //         }
    //     } else
    //         ids[i] = piece.id();
    // }

    return NodeValue(env, ids);
}

napi_value JSSentencepieceTokenizer::decode(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> args(env, info);

    std::vector<int> ids = args[0];
    std::string text;

    std::vector<std::string> pieces;
    const int num_pieces = args->sentence_piece_.GetPieceSize();
    pieces.reserve(ids.size());
    size_t added_tokens_size = args->added_tokens.size();

    for (const int id : ids) {
        if (id < 0 || id >= num_pieces + args->offset)
            pieces.emplace_back("");
        else if (id < added_tokens_size)
            pieces.emplace_back(args->added_tokens[id]);
        else
            pieces.emplace_back(args->sentence_piece_.IdToPiece(id - args->offset));
    }

    args->sentence_piece_.Decode(pieces, &text);

    return napi_value();
}