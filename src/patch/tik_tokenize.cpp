#include "tik_tokenize.h"
#include "pcre2_regex.h"

std::vector<std::string> GptEncoding::tokenize(const std::string& line_to_encode, const std::unordered_set<std::string>& allowed_special,
    const std::unordered_set<std::string>& disallowed_special)
{
    // Check for disallowed special tokens
    if (disallowed_special.count("all") > 0) {
        for (const auto& special_token : special_token_mappings_) {
            if (line_to_encode.find(special_token.first) != std::string::npos) {
                throw std::invalid_argument("Disallowed special token found: " + special_token.first);
            }
        }
    }
    // Call the encode_native function from the BytePairEncodingCore class
    return byte_pair_encoding_core_processor_.tokenize_native(line_to_encode, allowed_special);
}

std::vector<std::string> BytePairEncodingCore::tokenize_native(const std::string& line_to_encode,
    const std::unordered_set<std::string>& allowed_special)
{
    std::vector<std::string> tokens;

    auto lines = break_into_specials(line_to_encode, allowed_special);
    for (auto line : lines) {
        auto special_mapping = special_token_mappings_.find(line);
        if (special_mapping != special_token_mappings_.end() && allowed_special.count(line) > 0) {
            tokens.push_back(std::string(special_mapping->first.begin(), special_mapping->first.end()));
        } else {
            auto matches = pattern_string_->get_all_matches(line);
            for (auto token : matches) {
                auto special_mapping = special_token_mappings_.find(token);
                if (special_mapping != special_token_mappings_.end() && allowed_special.count(token) > 0) {
                    if (!token.empty()) {
                        tokens.push_back(std::string(special_mapping->first.begin(), special_mapping->first.end()));
                    }
                } else {
                    std::vector<uint8_t> utf8_encoded(token.begin(), token.end());
                    if (utf8_encoded.size() == 1) {
                        auto rank_iter = byte_pair_ranks_.find(utf8_encoded);
                        if (rank_iter != byte_pair_ranks_.end()) {
                            tokens.push_back(std::string(rank_iter->first.begin(), rank_iter->first.end()));
                        }
                    } else {
                        auto byte_pairs = byte_pair_merge(utf8_encoded, byte_pair_ranks_, [&](int start, int end) {
                            tokens.push_back(std::string(utf8_encoded.begin() + start, utf8_encoded.begin() + end));
                            return 0;
                        });
                    }
                }
            }
        }
    }
    return tokens;
}
