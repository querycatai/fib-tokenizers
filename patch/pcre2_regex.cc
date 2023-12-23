#include "pcre2_regex.h"

#include <stdexcept>

#define PCRE2_CODE_UNIT_WIDTH 0
#include "pcre.h"

class PCRERegex::Impl {
public:
    Impl(const std::string& pattern, int flags);
    Impl(Impl&& other);
    ~Impl();

    std::vector<std::string> get_all_matches(const std::string& text) const;
    std::vector<std::pair<std::string::size_type, std::string::size_type>> all_matches(const std::string& text) const;
    bool contains(const std::string& text) const;
    void replace_all(std::string& text, const std::string& replacement) const;

private:
    pcre* regex_ = nullptr;
};

PCRERegex::Impl::Impl(const std::string& pattern, int flags)
{
    const char* error;
    int error_offset;
    flags |= PCRE_UCP | PCRE_UTF8;
    regex_ = pcre_compile(pattern.c_str(), flags, &error, &error_offset, nullptr);
    if (!regex_) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "PCRE compilation failed at offset %d: %s", error_offset, error);
        throw std::runtime_error(buffer);
    }
}

PCRERegex::Impl::~Impl()
{
    if (regex_) {
        pcre_free(regex_);
    }
}

std::vector<std::string> PCRERegex::Impl::get_all_matches(const std::string& text) const
{
    std::vector<std::string> matches;
    auto pairs = all_matches(text);
    for (auto& x : pairs) {
        matches.emplace_back(text.substr(x.first, x.second));
    }
    return matches;
}

std::vector<std::pair<std::string::size_type, std::string::size_type>>
PCRERegex::Impl::all_matches(const std::string& text) const
{
    std::vector<std::pair<std::string::size_type, std::string::size_type>> result;
    const char* text_ptr = text.c_str();
    int text_length = text.size();
    int ovector[30];
    int start_offset = 0;
    int rc;

    do {
        rc = pcre_exec(regex_, nullptr, text_ptr, text_length, start_offset, 0, ovector, sizeof(ovector) / sizeof(ovector[0]));
        if (rc >= 0) {
            int match_length = ovector[1] - ovector[0];
            result.emplace_back(ovector[0], match_length);
            start_offset = ovector[1];
        }
    } while (rc >= 0 && start_offset < text_length);

    return result;
}

bool PCRERegex::Impl::contains(const std::string& text) const
{
    return !all_matches(text).empty();
}

void PCRERegex::Impl::replace_all(std::string& text, const std::string& replacement) const
{
    std::string result;
    std::string::size_type last = 0;
    auto pairs = all_matches(text);
    for (auto& x : pairs) {
        result.append(text.substr(last, x.first - last));
        last = x.first + x.second;
        result.append(replacement);
    }
    result.append(text.substr(last));
    text = result;
}

PCRERegex::PCRERegex(const std::string& pattern, int flags)
    : impl_(std::make_shared<Impl>(pattern, flags))
{
}

PCRERegex::PCRERegex(const std::string& pattern)
    : impl_(std::make_shared<Impl>(pattern, 0))
{
}

PCRERegex::PCRERegex(PCRERegex&& other)
    : impl_(std::move(other.impl_))
{
}

PCRERegex::~PCRERegex()
{
}

std::vector<std::string> PCRERegex::get_all_matches(const std::string& text) const
{
    return impl_->get_all_matches(text);
}

void PCRERegex::replace_all(std::string& text, const std::string& replacement) const
{
    impl_->replace_all(text, replacement);
}

bool PCRERegex::contains(const std::string& text) const
{
    return impl_->contains(text);
}

std::vector<std::pair<std::string::size_type, std::string::size_type>> PCRERegex::all_matches(const std::string& text) const
{
    return impl_->all_matches(text);
}