#include "SentencepieceTokenizer.h"
#include "sentencepiece.pb.h"

Napi::Function JSSentencepieceTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "SentencepieceTokenizer",
        { InstanceAccessor<&JSSentencepieceTokenizer::get_all_special_tokens>("all_special_tokens"),
            InstanceMethod("tokenize", &JSSentencepieceTokenizer::tokenize, napi_enumerable),
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

            special_tokens.emplace(token.content, token);
        }
    }
}

void JSSentencepieceTokenizer::config_basic_tokens(const Napi::Config& opt)
{
    static const char* special_token_keys[] = {
        "unk_token", "bos_token", "eos_token", "pad_token", "mask_token", "sep_token", "cls_token"
    };

    std::unordered_map<std::string, Napi::Value> special_tokens_map;
    special_tokens_map = opt.Get("special_tokens_map", special_tokens_map);

    int _unk_id = unk_id;

    for (int i = 0; i < sizeof(special_token_keys) / sizeof(special_token_keys[0]); i++) {
        Napi::Value config_value = opt.Get(special_token_keys[i], Napi::Value());
        napi_valuetype config_value_type = config_value.Type();

        SpecialToken token = config_value;
        auto it = special_tokens_map.find(special_token_keys[i]);
        if (it != special_tokens_map.end()) {
            Napi::Value special_value = it->second;
            napi_valuetype special_type = special_value.Type();
            SpecialToken special_token = special_value;

            if (config_value_type == napi_undefined || config_value_type == napi_null
                || config_value_type == napi_object || special_type != napi_string)
                token = special_token;
            else
                token.content = special_token.content;
        }

        if (token.content.length() > 0) {
            auto it = token_to_id.find(token.content);
            if (it != token_to_id.end())
                token.id = it->second;
            else {
                token.id = sentence_piece_.PieceToId(token.content);

                if (i > 0 && token.id == _unk_id && token.content != "<unk>") {
                    do {
                        token.id = vacob_size++ + offset;
                    } while (id_to_token.find(token.id) != id_to_token.end());

                    auto it = id_to_token.emplace(token.id, token.content);
                    token_to_id[it.first->second] = token.id;
                }
            }

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
    std::vector<std::string> additional_special_tokens;
    additional_special_tokens = opt.Get("additional_special_tokens", additional_special_tokens);

    for (int32_t i = additional_special_tokens.size() - 1; i >= 0; i--) {
        std::string& stoken = additional_special_tokens[i];
        if (stoken.length() > 0) {
            int id;

            auto it = token_to_id.find(stoken);
            if (it == token_to_id.end()) {
                id = sentence_piece_.PieceToId(stoken);

                if (id == unk_id && stoken != "<unk>") {
                    do {
                        id = vacob_size++ + offset;
                    } while (id_to_token.find(id) != id_to_token.end());

                    auto it = id_to_token.emplace(id, stoken);
                    token_to_id[it.first->second] = id;

                    special_tokens.emplace(stoken, SpecialToken(stoken, id));
                }
            }
        }
    }
}

void JSSentencepieceTokenizer::config_added_tokens(const Napi::Config& opt)
{
    std::unordered_map<std::string, int> added_tokens_map;
    added_tokens_map = opt.Get("added_tokens", added_tokens_map);

    for (auto& [key, value] : added_tokens_map) {
        auto it = id_to_token.emplace(value, key);
        token_to_id[it.first->second] = value;

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
    vacob_size = sentence_piece_.GetPieceSize();

    bos_id = sentence_piece_.bos_id();
    eos_id = sentence_piece_.eos_id();
    unk_id = sentence_piece_.unk_id();

    Napi::Config opt(info[1]);

    legacy = opt.Get("legacy", true);
    offset = opt.Get("offset", 0);

    config_tokens_decoder(opt);
    config_added_tokens(opt);
    config_special_tokens(opt);
    config_basic_tokens(opt);
    config_prefix_suffix(opt);
    config_pattern(opt);
}
