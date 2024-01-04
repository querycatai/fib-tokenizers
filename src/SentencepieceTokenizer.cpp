#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

Napi::Value JSSentencepieceTokenizer::get_all_special_tokens(const Napi::CallbackInfo& info)
{
    std::vector<std::string> tokens;

    for (auto& token : special_tokens)
        tokens.emplace_back(token.second);

    return to_value(info.Env(), tokens);
}

int JSSentencepieceTokenizer::convert_token_to_id(std::string_view token)
{
    auto it = token_to_id.find(token);
    if (it != token_to_id.end())
        return it->second;

    return sentence_piece_.PieceToId(token);
}

void JSSentencepieceTokenizer::put_token(int token, int32_t index, const std::function<void(int, int)>& push_back)
{
    if (token == model_unk_id)
        push_back(unk_id, index);
    else
        push_back(token + offset, index);
}

void JSSentencepieceTokenizer::put_token(const std::string& token, int32_t index, const std::function<void(const std::string&, int)>& push_back)
{
    push_back(token, index);
}

void JSSentencepieceTokenizer::encode(std::string_view text, const std::function<void(int, int)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        put_token(spt.pieces(i).id(), i, push_back);
}

void JSSentencepieceTokenizer::encode(std::string_view text, const std::function<void(const std::string&, int)>& push_back)
{
    sentencepiece::SentencePieceText spt;
    sentence_piece_.Encode(text, &spt);

    for (int32_t i = 0; i < spt.pieces_size(); i++)
        put_token(spt.pieces(i).piece(), i, push_back);
}

template <typename T>
void JSSentencepieceTokenizer::legacy_encode(std::string_view text, std::vector<T>* ids, int32_t prefix_count)
{
    int32_t start = 0;
    std::string temp_string;

    if (!legacy) {
        if (ids->size() > prefix_count)
            temp_string = "_";
        else
            temp_string = "_ ";

        temp_string.append(text);
        text = temp_string;
        start = 1;
    }

    encode(text, [&](const T& token, int index) {
        if (index >= start)
            ids->emplace_back(token);
    });
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
            auto it = special_tokens.find(m[2]);
            if (it == special_tokens.end()) {
                searchStart = m[2].first + 1;
                continue;
            }

            const SpecialToken& token = it->second;

            size_t pos = (token.lstrip ? m[0].first : m[2].first) - text.cbegin();
            if (pos != lastPos)
                legacy_encode(std::string_view(text.data() + lastPos, pos - lastPos), ids, prefix_count);

            ids->emplace_back(token);

            searchStart = token.rstrip ? m[0].first + m[0].length() : m[2].first + m[2].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size())
        legacy_encode(std::string_view(text.data() + lastPos, text.size() - lastPos), ids, prefix_count);
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
        ids.emplace_back(token);

    encode(text, &ids);

    if (add_eos_if_not_present && add_eos_token && ids.size() > 0 && ids[ids.size() - 1] == eos_id)
        for (int i = 1; i < suffix_tokens.size(); i++)
            ids.emplace_back(suffix_tokens[i]);
    else
        for (auto& token : suffix_tokens)
            ids.emplace_back(token);

    return to_value(info.Env(), ids);
}

Napi::Value JSSentencepieceTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int> ids = to_array<int>(info[0]);
    std::string text;

    std::vector<absl::string_view> pieces;
    const int num_pieces = sentence_piece_.GetPieceSize();
    pieces.reserve(ids.size());

    for (const int id : ids)
        if (id != unk_id && id >= offset && id < num_pieces + offset
            && id_to_token.find(id) == id_to_token.end())
            pieces.emplace_back(sentence_piece_.IdToPiece(id - offset));

    sentence_piece_.Decode(pieces, &text);

    return to_value(info.Env(), text);
}
