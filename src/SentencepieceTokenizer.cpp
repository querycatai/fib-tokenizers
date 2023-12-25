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

JSSentencepieceTokenizer::JSSentencepieceTokenizer(NodeArg<JSSentencepieceTokenizer>& arg)
    : env_(arg.env())
{
    sentence_piece_.LoadFromSerializedProto(arg.args(0));

    NodeOpt opt(arg.args(1));
    sentence_piece_.SetEncodeExtraOptions(opt.Get("extra_options", std::string()));
    offset = opt.Get("offset", 0);
    added_tokens = opt.Get("added_tokens", std::vector<std::string>());
    for (size_t i = 0; i < added_tokens.size(); i++) {
        std::string token = added_tokens[i];
        if (token == "<unk>")
            unk_id = i;
    }
}

napi_value JSSentencepieceTokenizer::tokenize(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> obj(env, info);

    std::string text = obj.args(0);
    std::vector<std::string> tokens;

    sentencepiece::SentencePieceText spt;
    obj->sentence_piece_.Encode(text, &spt);
    size_t added_tokens_size = obj->added_tokens.size();

    tokens.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++) {
        auto piece = spt.pieces(i);

        tokens[i] = piece.piece();
    }

    return NodeValue(env, tokens);
}

napi_value JSSentencepieceTokenizer::encode(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> obj(env, info);

    std::string text = obj.args(0);
    std::vector<int> ids;

    sentencepiece::SentencePieceText spt;
    obj->sentence_piece_.Encode(text, &spt);
    size_t added_tokens_size = obj->added_tokens.size();

    ids.resize(spt.pieces_size());
    for (int i = 0; i < spt.pieces_size(); i++) {
        auto piece = spt.pieces(i);

        if (added_tokens_size || obj->offset) {
            int j;
            std::string txt = piece.piece();

            for (j = 0; j < added_tokens_size; j++) {
                if (txt == obj->added_tokens[j]) {
                    ids[i] = j;
                    break;
                }
            }

            if (j == added_tokens_size) {
                uint32_t id = piece.id();
                ids[i] = id ? id + obj->offset : obj->unk_id;
            }
        } else
            ids[i] = piece.id();
    }

    return NodeValue(env, ids);
}

napi_value JSSentencepieceTokenizer::decode(napi_env env, napi_callback_info info)
{
    NodeArg<JSSentencepieceTokenizer> obj(env, info);

    std::vector<int> ids = obj.args(0);
    std::string text;

    std::vector<std::string> pieces;
    const int num_pieces = obj->sentence_piece_.GetPieceSize();
    pieces.reserve(ids.size());
    size_t added_tokens_size = obj->added_tokens.size();

    for (const int id : ids) {
        if (id < 0 || id >= num_pieces + obj->offset)
            pieces.emplace_back("");
        else if (id < added_tokens_size)
            pieces.emplace_back(obj->added_tokens[id]);
        else
            pieces.emplace_back(obj->sentence_piece_.IdToPiece(id - obj->offset));
    }

    obj->sentence_piece_.Decode(pieces, &text);

    return napi_value();
}
