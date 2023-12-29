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

void JSSentencepieceTokenizer::config_tokens_decoder(const Napi::Config& opt)
{
    std::unordered_map<std::string, SpecialToken> added_tokens_decoder;
    added_tokens_decoder = opt.Get("added_tokens_decoder", added_tokens_decoder);

    for (auto& [key, token] : added_tokens_decoder) {
        uint32_t id = std::stoul(key);

        auto it = id_to_token.emplace(id, token.content);
        if (token.content.length() > 0) {
            token.id = id;
            token_to_id[it.first->second] = id;

            if (token.special)
                special_tokens.emplace(token.content, token);
        }
    }
}

void JSSentencepieceTokenizer::config_basic_tokens(const Napi::Config& opt)
{
    static const char* special_token_keys[] = {
        "unk_token", "bos_token", "eos_token", "pad_token", "mask_token", "sep_token"
    };

    bos_id = sentence_piece_.bos_id();
    eos_id = sentence_piece_.eos_id();
    unk_id = sentence_piece_.unk_id();

    std::unordered_map<std::string, Napi::Value> special_tokens_map;
    special_tokens_map = opt.Get("special_tokens_map", special_tokens_map);

    for (int i = 0; i < sizeof(special_token_keys) / sizeof(special_token_keys[0]); i++) {
        Napi::Value config_value = opt.Get(special_token_keys[i], Napi::Value());
        napi_valuetype config_value_type = config_value.Type();

        SpecialToken token = config_value;
        auto it = special_tokens_map.find(special_token_keys[i]);
        if (it != special_tokens_map.end()) {
            Napi::Value special_value = it->second;
            napi_valuetype special_type = special_value.Type();
            SpecialToken special_token = special_value;

            if (token.content.length() > 0 && token.content != special_token.content) {
                token.id = convert_token_to_id(token.content);
                special_tokens.emplace(token.content, token);
            }

            if (config_value_type == napi_undefined || config_value_type == napi_null
                || config_value_type == napi_object || special_type != napi_string)
                token = special_token;
            else
                token.content = special_token.content;
        }

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

            special_tokens.emplace(token.content, token);
        }
    }
}

void JSSentencepieceTokenizer::config_special_tokens(const Napi::Config& opt)
{
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

                special_tokens.emplace(stoken, SpecialToken(stoken, id));
            }
        }
}

void JSSentencepieceTokenizer::config_added_tokens(const Napi::Config& opt)
{
    std::unordered_map<std::string, int> added_tokens_map;
    added_tokens_map = opt.Get("added_tokens", added_tokens_map);

    for (auto& [key, value] : added_tokens_map) {
        special_tokens.emplace(key, SpecialToken(key, value));
    }
}

void JSSentencepieceTokenizer::config_prefix_suffix(const Napi::Config& opt)
{
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
}

void JSSentencepieceTokenizer::config_pattern(const Napi::Config& opt)
{
    static const char* space_tokens[] = {
        " ",
        "\t",
        "\n",
        "\r",
        "\f",
        "\v"
    };

    if (special_tokens.size() > 0) {
        std::string pattern_str;
        std::vector<std::string> keys;

        for (auto& [key, value] : special_tokens)
            keys.emplace_back(key);

        std::sort(keys.begin(), keys.end(), [](const std::string& a, const std::string& b) {
            return a.length() > b.length();
        });

        for (auto& key : keys) {
            if (pattern_str.length() > 0)
                pattern_str += "|";
            pattern_str += escapeRegex(std::string(key));
        }

        std::string space_str;
        for (auto& token : space_tokens) {
            if (special_tokens.find(token) == special_tokens.end()) {
                if (space_str.length() > 0)
                    space_str += "|";
                space_str += escapeRegex(std::string(token));
            }
        }

        has_pattern = true;
        pattern = std::regex("(" + space_str + ")?(" + pattern_str + ")(" + space_str + ")?");
    }
}

JSSentencepieceTokenizer::JSSentencepieceTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSSentencepieceTokenizer>(info)
{
    sentence_piece_.LoadFromSerializedProto(from_value<std::string_view>(info[0]));

    Napi::Config opt(info[1]);

    legacy = opt.Get("legacy", true);
    offset = opt.Get("offset", 0);

    config_tokens_decoder(opt);
    config_basic_tokens(opt);
    config_special_tokens(opt);
    config_added_tokens(opt);
    config_prefix_suffix(opt);
    config_pattern(opt);
}

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
