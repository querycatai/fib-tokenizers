#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

int32_t SentencepieceTokenizer::model_token_to_id(std::string_view token)
{
    return sentence_piece_.PieceToId(token);
}

void SentencepieceTokenizer::encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        put_token(spt.pieces(i).id(), i, push_back);
}

void SentencepieceTokenizer::encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        put_token(spt.pieces(i).piece(), i, push_back);
}

void SentencepieceTokenizer::decode(const std::vector<int32_t>& ids, std::string& text)
{
    std::vector<absl::string_view> pieces;
    pieces.reserve(ids.size());

    for (const int32_t id : ids)
        pieces.emplace_back(sentence_piece_.IdToPiece(id));

    sentence_piece_.Decode(pieces, &text);
}
