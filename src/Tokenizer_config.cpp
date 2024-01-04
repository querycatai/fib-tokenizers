#include "Tokenizer.h"
#include <set>

std::string escapeRegex(const std::string& str)
{
    static const std::regex escape(R"([.^$|()\[\]{}*+?\\])");
    static const std::string format(R"(\\&)");
    static const std::regex escape_digest(R"(\d+)");

    return std::regex_replace(
        std::regex_replace(str, escape, format, std::regex_constants::format_sed),
        escape_digest, R"(\d+)");
}

void Tokenizer::config_tokens_decoder(const Napi::Config& opt)
{
    std::unordered_map<std::string, SpecialToken> added_tokens_decoder;
    added_tokens_decoder = opt.Get("added_tokens_decoder", added_tokens_decoder);

    for (auto& [key, token] : added_tokens_decoder) {
        token.id = std::stoul(key);
        add_token(token);
    }
}

void Tokenizer::add_token(SpecialToken& token, bool is_unk)
{
    if (token.content.length() > 0) {
        if (token.id == -1) {
            auto it = special_tokens.find(token.content);
            if (it != special_tokens.end())
                token.id = it->second.id;
            else {
                token.id = model_token_to_id(token.content);

                if (token.id == model_unk_id && !is_unk && token.content != "<unk>") {
                    if (add_basic_tokens) {
                        do {
                            token.id = special_token_offset++ + offset;
                        } while (id_to_token.find(token.id) != id_to_token.end());
                    } else
                        token.id = 0;
                } else {
                    token.id += offset;
                }
            }
        }

        auto it = id_to_token.emplace(token.id, token.content);
        special_tokens.emplace(it.first->second, token);
    }
}

void Tokenizer::config_basic_tokens(const Napi::Config& opt)
{
    static const char* special_token_keys[] = {
        "unk_token", "bos_token", "eos_token", "pad_token", "mask_token", "sep_token", "cls_token"
    };

    int32_t* special_token_ids[] = {
        &unk_id, &bos_id, &eos_id, &pad_id
    };

    for (int32_t i = 0; i < sizeof(special_token_keys) / sizeof(special_token_keys[0]); i++) {
        Napi::Value config_value = opt.Get(special_token_keys[i], Napi::Value());
        SpecialToken token = config_value;

        if (token.content.length() > 0) {
            add_token(token, i == 0);

            if (i < sizeof(special_token_ids) / sizeof(special_token_ids[0]))
                *special_token_ids[i] = token.id;
        }
    }
}

void Tokenizer::config_special_tokens(const Napi::Config& opt)
{
    std::vector<std::string> additional_special_tokens;
    additional_special_tokens = opt.Get("additional_special_tokens", additional_special_tokens);

    for (int32_t i = 0; i < additional_special_tokens.size(); i++) {
        SpecialToken token(additional_special_tokens[i]);
        add_token(token);
    }
}

void Tokenizer::config_added_tokens(const Napi::Config& opt)
{
    std::unordered_map<std::string, int32_t> added_tokens_map;
    added_tokens_map = opt.Get("added_tokens", added_tokens_map);

    for (auto& [key, value] : added_tokens_map) {
        SpecialToken token(key, value);
        add_token(token);
    }
}

void Tokenizer::config_prefix_suffix(const Napi::Config& opt)
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

    add_eos_if_not_present = opt.Get("add_eos_if_not_present", false);
}

void Tokenizer::config_pattern(const Napi::Config& opt)
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

        std::set<std::string> res;
        for (auto& key : keys)
            res.emplace(escapeRegex(std::string(key)));

        keys.clear();

        for (auto& token : res) {
            keys.emplace_back(token);
        }

        std::sort(keys.begin(), keys.end(), [](const std::string& a, const std::string& b) {
            return a.length() > b.length();
        });

        for (auto& key : keys) {
            if (pattern_str.length() > 0)
                pattern_str += "|";
            pattern_str += key;
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

void Tokenizer::init(Napi::Config opt, int32_t vocab_size_, int32_t unk_id_)
{
    vocab_size = special_token_offset = vocab_size_;
    model_unk_id = unk_id_;

    legacy = opt.Get("legacy", true);
    offset = opt.Get("offset", 0);

    config_tokens_decoder(opt);
    config_added_tokens(opt);
    config_special_tokens(opt);
    config_basic_tokens(opt);
    config_prefix_suffix(opt);
    config_pattern(opt);
}
