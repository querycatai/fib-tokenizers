#pragma once
#include "encoding_utils.h"
#include "modelparams.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>
#include <memory>

class PCRERegex;

class BytePairEncodingCore {
public:
    std::unordered_map<std::vector<uint8_t>, int, VectorHash> byte_pair_ranks_;
    std::unordered_map<std::string, int> special_token_mappings_;
    std::shared_ptr<PCRERegex> pattern_string_;

    static std::vector<int> byte_pair_merge(const std::vector<uint8_t>& piece,
        const std::unordered_map<std::vector<uint8_t>, int, VectorHash>& ranks,
        const std::function<int(int, int)>& f);

public:
    BytePairEncodingCore(const std::unordered_map<std::vector<uint8_t>, int, VectorHash>& byte_pair_ranks,
        const std::unordered_map<std::string, int>& special_token_mappings,
        const std::shared_ptr<PCRERegex>& pattern_string);

    std::pair<std::vector<int>, std::vector<int>> encode_native(const std::string& line_to_encode,
        const std::unordered_set<std::string>& allowed_special);
    std::string decode_native(const std::vector<int>& input_tokens_to_decode);
    std::vector<std::string> break_into_specials(std::string const& line_to_encode, const std::unordered_set<std::string>& allowed_special);

    std::vector<std::string> tokenize_native(const std::string& line_to_encode,
        const std::unordered_set<std::string>& allowed_special);

    [[nodiscard]] const std::unordered_map<std::vector<uint8_t>, int, VectorHash>& getBytePairRanks() const;
};

class IResourceReader;

class GptEncoding {
public:
    int max_token_value_;
    std::unordered_map<std::string, int> special_token_mappings_;
    BytePairEncodingCore byte_pair_encoding_core_processor_;

public:
    GptEncoding(const std::string& pattern_string, const std::unordered_map<std::vector<uint8_t>, int, VectorHash>& byte_pair_ranks,
        const std::unordered_map<std::string, int>& special_token_mappings, int explicit_n_vocab);
    static std::shared_ptr<GptEncoding> get_encoding(LanguageModel model, IResourceReader* resource_reader = nullptr);
    std::vector<int> encode(const std::string& line_to_encode, const std::unordered_set<std::string>& allowed_special = {},
        const std::unordered_set<std::string>& disallowed_special = { "all" });
    std::string decode(const std::vector<int>& input_tokens_to_decode);

    std::vector<std::string> tokenize(const std::string& line_to_encode, const std::unordered_set<std::string>& allowed_special = {},
        const std::unordered_set<std::string>& disallowed_special = { "all" });

    [[nodiscard]] const std::unordered_map<std::vector<uint8_t>, int, VectorHash>& get_byte_pair_token_map() const;
};
