#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

int JSSentencepieceTokenizer::convert_token_to_id(std::string_view token)
{
    auto it = token_to_id.find(token);
    if (it != token_to_id.end())
        return it->second;

    return sentence_piece_.PieceToId(token);
}

void JSSentencepieceTokenizer::push_token(int token, std::vector<int>* ids)
{
    if (offset) {
        if (id_to_token.find(token) != id_to_token.end())
            ids->push_back(token);
        else
            ids->push_back(token + offset);
    } else
        ids->push_back(token);
}

void JSSentencepieceTokenizer::push_token(const SpecialToken& token, std::vector<int>* ids)
{
    push_token(token.id, ids);
}

void JSSentencepieceTokenizer::push_token(const SpecialToken& token, std::vector<std::string>* ids)
{
    ids->push_back(token.content);
}

void JSSentencepieceTokenizer::push_token(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<int>* ids)
{
    int id = piece.id();

    if (offset > 0) {
        if (id == 0)
            ids->push_back(unk_id);
        else if (id == unk_id)
            ids->push_back(unk_id + offset);
        else
            push_token(id, ids);
    } else
        push_token(id, ids);
}

void JSSentencepieceTokenizer::push_token(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<std::string>* ids)
{
    ids->push_back(piece.piece());
}

template <typename T>
void JSSentencepieceTokenizer::sentencepiece_encode(char* text, size_t size, std::vector<T>* ids, int32_t prefix_count)
{
    int32_t start = 0;
    if (!legacy && ids->size() > prefix_count) {
        *--text = '-';
        size++;
        start = 1;
    }

    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(std::string_view(text, size), &spt);

    for (; start < spt.pieces_size(); start++)
        push_token(spt.pieces(start), ids);
}

template <typename T>
void JSSentencepieceTokenizer::encode(std::string& text, std::vector<T>* ids)
{
    size_t lastPos = 0;
    int32_t prefix_count = ids->size();

    if (has_pattern) {
        std::smatch m;
        std::string::const_iterator searchStart(text.cbegin());

        while (std::regex_search(searchStart, text.cend(), m, pattern)) {
            const SpecialToken& token = special_tokens[m[2]];

            size_t pos = (token.lstrip ? m[0].first : m[2].first) - text.cbegin();
            if (pos != lastPos)
                sentencepiece_encode(text.data() + lastPos, pos - lastPos, ids, prefix_count);

            push_token(token, ids);

            searchStart = token.rstrip ? m[0].first + m[0].length() : m[2].first + m[2].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size())
        sentencepiece_encode(text.data() + lastPos, text.size() - lastPos, ids, prefix_count);
}

Napi::Value JSSentencepieceTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<std::string> tokens;

    encode(text, &tokens);

    return to_value(info.Env(), tokens);
}

Napi::Value JSSentencepieceTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<int> ids;

    for (auto& token : prefix_tokens)
        push_token(token, &ids);

    encode(text, &ids);

    if (offset == 0 && add_eos_token && ids.size() > 0 && ids[ids.size() - 1] == eos_id)
        for (int i = 1; i < suffix_tokens.size(); i++)
            push_token(suffix_tokens[i], &ids);
    else
        for (auto& token : suffix_tokens)
            push_token(token, &ids);

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
