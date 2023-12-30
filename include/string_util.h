#pragma once

#include <string>

inline bool IsCJK(char32_t c)
{
    return (c >= 0x4E00 && c <= 0x9FFF)
        || (c >= 0x3400 && c <= 0x4DBF)
        || (c >= 0x20000 && c <= 0x2A6DF)
        || (c >= 0x2A700 && c <= 0x2B73F)
        || (c >= 0x2B740 && c <= 0x2B81F)
        || (c >= 0x2B820 && c <= 0x2CEAF)
        || (c >= 0xF900 && c <= 0xFAFF)
        || (c >= 0x2F800 && c <= 0x2FA1F);
}

inline bool IsSpace(char32_t c)
{
    if (c == 13 || c == 32 || c == 160 || c == 8239 || c == 8287 || c == 12288) {
        return true;
    }

    if ((c >= 9 && c <= 10) || (c >= 8192 && c <= 8202)) {
        return true;
    }

    return false;
}

inline bool IsPunct(char32_t c)
{
    if (c == 161 || c == 167 || c == 171 || c == 187 || c == 191
        || c == 894 || c == 903 || c == 12336 || c == 12349) {
        return true;
    }

    if ((c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96)
        || (c >= 123 && c <= 126) || (c >= 182 && c <= 183) || (c >= 8208 && c <= 8231)
        || (c >= 8240 && c <= 8259) || (c >= 8261 && c <= 8273) || (c >= 8275 && c <= 8286)
        || (c >= 12289 && c <= 12291) || (c >= 12296 && c <= 12305) || (c >= 12308 && c <= 12319)) {
        return true;
    }

    return false;
}

inline bool IsControl(char32_t c)
{
    if (c == 173 || c == 907 || c == 909 || c == 930 || c == 11930 || c == 173790 || c == 195102) {
        return true;
    }

    if ((c >= 0 && c <= 8) || (c >= 11 && c <= 12) || (c >= 14 && c <= 31) || (c >= 128 && c <= 159)
        || (c >= 888 && c <= 889) || (c >= 896 && c <= 899) || (c >= 8203 && c <= 8207)
        || (c >= 8234 && c <= 8238) || (c >= 8288 && c <= 8302) || (c >= 12020 && c <= 12030)
        || (c >= 40957 && c <= 40958) || (c >= 64110 && c <= 64111) || (c >= 64218 && c <= 64254)
        || (c >= 177973 && c <= 177982) || (c >= 178206 && c <= 178207) || (c >= 183970 && c <= 183982)) {
        return true;
    }

    return false;
}

inline bool IsAccent(char32_t c)
{
    return c >= 0x300 && c <= 0x36F;
}

inline char32_t ToLower(char32_t c)
{
    if ((c >= 'A') && (c <= 'Z')) {
        return c + 'a' - 'A';
    }

    if ((c >= U'À' && (c <= U'Þ'))) {
        return c + U'à' - U'À';
    }

    return c;
}

inline char32_t StripAccent(char32_t c)
{
    const char32_t* tr = U"AAAAAAÆCEEEEIIIIÐNOOOOO×ØUUUUYÞßaaaaaaæceeeeiiiiðnooooo÷øuuuuyþy";
    if (c < 192 || c > 255) {
        return c;
    }

    return tr[c - 192];
}
