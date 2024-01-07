#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

int32_t SentencepieceTokenizerCore::vocab_size() const
{
    return sentence_piece_.GetPieceSize();
}

int32_t SentencepieceTokenizerCore::unk_id() const
{
    return sentence_piece_.unk_id();
}

int32_t SentencepieceTokenizerCore::model_token_to_id(std::string_view token)
{
    return sentence_piece_.PieceToId(token);
}

void SentencepieceTokenizerCore::encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        push_back(spt.pieces(i).id(), i);
}

void SentencepieceTokenizerCore::encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        push_back(spt.pieces(i).piece(), i);
}

void SentencepieceTokenizerCore::decode(const std::vector<int32_t>& ids, std::string& text)
{
    std::vector<absl::string_view> pieces;
    pieces.reserve(ids.size());

    for (const int32_t id : ids)
        pieces.emplace_back(sentence_piece_.IdToPiece(id));

    sentence_piece_.Decode(pieces, &text);
}
