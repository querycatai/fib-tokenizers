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
    // printf("add_token: %s[%d]\n", token.content.c_str(), token.id);

    if (token.content.length() > 0) {
        if (token.id == -1) {
            auto it = special_token_to_id.find(token.content);
            if (it != special_token_to_id.end())
                token.id = it->second.id;
            else {
                token.id = tokenizer->model_token_to_id(token.content);

                if (token.id == model_unk_id && !is_unk && token.content != "<unk>") {
                    if (add_basic_tokens) {
                        do {
                            token.id = special_token_offset++ + offset;
                        } while (special_id_to_token.find(token.id) != special_id_to_token.end());
                    } else
                        token.id = 0;
                } else {
                    token.id += offset;
                }
            }
        }

        // printf("add_token to: %s[%d]\n", token.content.c_str(), token.id);
        auto it = special_tokens.emplace(token.content);
        special_token_to_id.emplace(*(it.first), token);
        special_id_to_token.insert_or_assign(token.id, token.content);
    }
}

void Tokenizer::config_unk_tokens(const Napi::Config& opt)
{
    Napi::Value config_value = opt.Get("unk_token", Napi::Value());
    SpecialToken token = config_value;

    if (token.content.length() > 0) {
        add_token(token, true);
        unk_id = token.id;
    }
}

void Tokenizer::config_basic_tokens(const Napi::Config& opt)
{
    static const char* special_token_keys[] = {
        "bos_token", "eos_token", "pad_token", "mask_token", "sep_token", "cls_token"
    };

    int32_t* special_token_ids[] = {
        &bos_id, &eos_id, &pad_id
    };

    for (int32_t i = 0; i < sizeof(special_token_keys) / sizeof(special_token_keys[0]); i++) {
        Napi::Value config_value = opt.Get(special_token_keys[i], Napi::Value());
        SpecialToken token = config_value;

        if (token.content.length() > 0) {
            add_token(token);

            if (i < sizeof(special_token_ids) / sizeof(special_token_ids[0]))
                *special_token_ids[i] = token.id;
        }
    }
}

void Tokenizer::config_special_tokens(const Napi::Config& opt)
{
    std::vector<SpecialToken> additional_special_tokens;
    additional_special_tokens = opt.Get("additional_special_tokens", additional_special_tokens);

    for (int32_t i = 0; i < additional_special_tokens.size(); i++)
        add_token(additional_special_tokens[i]);
}

void Tokenizer::config_added_tokens(const Napi::Config& opt)
{
    Napi::Value config_value = opt.Get("added_tokens", Napi::Value());

    if (config_value.IsArray()) {
        std::vector<SpecialToken> added_tokens;
        added_tokens = opt.Get("added_tokens", added_tokens);

        for (auto& token : added_tokens)
            add_token(token);
    } else {
        std::unordered_map<std::string, int32_t> added_tokens_map;
        added_tokens_map = opt.Get("added_tokens", added_tokens_map);

        for (auto& [key, value] : added_tokens_map) {
            SpecialToken token(key, value);
            add_token(token);
        }
    }
}

void Tokenizer::config_prefix_suffix(const Napi::Config& opt)
{
    bool add_bos_token = opt.Get("add_bos_token", false);
    if (add_bos_token)
        single_prefix_tokens.emplace_back(bos_id);

    bool add_eos_token = opt.Get("add_eos_token", false);
    if (add_eos_token)
        single_suffix_tokens.emplace_back(eos_id);

    add_eos_if_not_present = opt.Get("add_eos_if_not_present", add_eos_if_not_present);
}

void Tokenizer::config_post_processor(const Napi::Config& opt)
{
    Napi::Value post_processor = opt.Get({ "post_processor", "single" });
    if (post_processor.IsArray()) {
        Napi::Array post_processor_array = post_processor.As<Napi::Array>();
        bool has_sequence = false;

        single_prefix_tokens.clear();
        single_suffix_tokens.clear();

        for (int32_t i = 0; i < post_processor_array.Length(); i++) {
            Napi::Object post_processor_object = post_processor_array.Get(i).As<Napi::Object>();
            Napi::Value v;

            v = post_processor_object.Get("Sequence");
            napi_valuetype type = v.Type();
            if (type != napi_undefined && type != napi_null) {
                if (has_sequence)
                    throw Napi::Error::New(opt.GetEnv(), "Only one Sequence is allowed in single mode post processor.");

                has_sequence = true;
                continue;
            }

            v = post_processor_object.Get("SpecialToken");
            type = v.Type();
            if (type != napi_undefined && type != napi_null) {
                post_processor_object = v.As<Napi::Object>();
                std::string token = post_processor_object.Get("id").As<Napi::String>();

                if (!has_sequence)
                    single_prefix_tokens.emplace_back(convert_token_to_id(token));
                else
                    single_suffix_tokens.emplace_back(convert_token_to_id(token));
            }
        }
    }

    post_processor = opt.Get({ "post_processor", "pair" });
    if (post_processor.IsArray()) {
        Napi::Array post_processor_array = post_processor.As<Napi::Array>();
        int32_t sequence_count = 0;

        for (int32_t i = 0; i < post_processor_array.Length(); i++) {
            Napi::Object post_processor_object = post_processor_array.Get(i).As<Napi::Object>();
            Napi::Value v;

            v = post_processor_object.Get("Sequence");
            napi_valuetype type = v.Type();
            if (type != napi_undefined && type != napi_null) {
                if (sequence_count > 2)
                    throw Napi::Error::New(opt.GetEnv(), "Only two Sequence are allowed in pair mode post processor.");

                sequence_count++;
                continue;
            }

            v = post_processor_object.Get("SpecialToken");
            type = v.Type();
            if (type != napi_undefined && type != napi_null) {
                post_processor_object = v.As<Napi::Object>();
                std::string token = post_processor_object.Get("id").As<Napi::String>();

                switch (sequence_count) {
                case 0:
                    pair_prefix_tokens.emplace_back(convert_token_to_id(token));
                    break;
                case 1:
                    pair_middle_tokens.emplace_back(convert_token_to_id(token));
                    break;
                case 2:
                    pair_suffix_tokens.emplace_back(convert_token_to_id(token));
                    break;
                }
            }
        }
    } else {
        pair_prefix_tokens.insert(pair_prefix_tokens.end(), single_prefix_tokens.begin(), single_prefix_tokens.end());

        pair_middle_tokens.insert(pair_middle_tokens.end(), single_suffix_tokens.begin(), single_suffix_tokens.end());
        pair_middle_tokens.insert(pair_middle_tokens.end(), single_prefix_tokens.begin(), single_prefix_tokens.end());

        pair_suffix_tokens.insert(pair_suffix_tokens.end(), single_suffix_tokens.begin(), single_suffix_tokens.end());
    }
}

static std::string utf16_string(char16_t ch)
{
    std::string str;
    utf8::convert(&ch, 1, str);
    return str;
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

    if (special_token_to_id.size() > 0) {
        std::string pattern_str;
        std::vector<std::string> keys;
        std::vector<char16_t> chars;

        for (auto& [key, value] : special_token_to_id)
            keys.emplace_back(key);

        std::set<std::string> res;
        for (auto& key : keys)
            res.emplace(escapeRegex(std::string(key)));

        for (auto& token : res) {
            std::u16string u16_token;
            utf8::convert(token, u16_token);
            if (u16_token.length() == 1)
                chars.emplace_back(u16_token[0]);
        }

        std::sort(chars.begin(), chars.end());
        char16_t start_char = 0;
        char16_t end_char = 0;
        for (auto& c : chars) {
            if (c == end_char + 1)
                end_char = c;
            else {
                if (end_char > start_char + 1) {
                    res.emplace("[" + utf16_string(start_char) + "-" + utf16_string(end_char) + "]");
                    for (char16_t i = start_char; i <= end_char; i++)
                        res.erase(utf16_string(i));
                }

                start_char = end_char = c;
            }
        }

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
            if (special_token_to_id.find(token) == special_token_to_id.end()) {
                if (space_str.length() > 0)
                    space_str += "|";
                space_str += escapeRegex(std::string(token));
            }
        }

        has_pattern = true;
        pattern = boost::regex("(" + space_str + ")?(" + pattern_str + ")(" + space_str + ")?");
    }
}

void Tokenizer::config_model_input_names(const Napi::Config& opt)
{
    std::vector<std::string> model_input_names;
    model_input_names = opt.Get("model_input_names", model_input_names);

    for (auto& name : model_input_names) {
        if (name == "token_type_ids")
            token_type_ids = true;
        else if (name == "attention_mask")
            attention_mask = true;
        else if (name == "position_ids")
            position_ids = true;
    }
}

void Tokenizer::init(std::shared_ptr<TokenizerCore> Tokenizer_, Napi::Config opt, bool add_basic_tokens_)
{
    tokenizer = Tokenizer_;
    vocab_size = special_token_offset = tokenizer->vocab_size();
    model_unk_id = tokenizer->unk_id();

    add_basic_tokens = add_basic_tokens_;

    legacy = opt.Get("legacy", legacy);
    offset = opt.Get("offset", 0);
    add_prefix_space = opt.Get("add_prefix_space", add_prefix_space);
    do_lower_case = opt.Get("do_lower_case", do_lower_case);

    model_max_length = opt.Get("model_max_length", model_max_length);
    if (model_max_length <= 0)
        model_max_length = 1024;

    std::string padding_side = opt.Get("padding_side", std::string("right"));
    if (padding_side == "left")
        padding_left = true;

    config_tokens_decoder(opt);
    config_added_tokens(opt);
    config_unk_tokens(opt);
    config_special_tokens(opt);
    config_basic_tokens(opt);
    config_prefix_suffix(opt);
    config_post_processor(opt);
    config_pattern(opt);
    config_model_input_names(opt);
}
