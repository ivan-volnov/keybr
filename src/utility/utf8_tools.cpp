/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT

Copyright (c) 2020 Ivan Volnov

Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de>
See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "utf8_tools.h"


constexpr uint32_t UTF8_ACCEPT = 0;
constexpr uint32_t UTF8_REJECT = 12;

constexpr uint8_t utf8d[] = {
    // The first part of the table maps bytes to character classes that
    // to reduce the size of the transition table and create bitmasks.
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

    // The second part is a transition table that maps a combination
    // of a state of the automaton and a character class to a state.
    0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
    12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
    12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
    12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
    12,36,12,12,12,12,12,12,12,12,12,12,
};

bool tools::utf8::decoder::decode_symbol(uint8_t ch, char32_t &codepoint)
{
    uint32_t type = utf8d[ch];
    codepoint = state == UTF8_ACCEPT ? (0xff >> type) & (ch) : (ch & 0x3fu) | (codepoint << 6);
    state = utf8d[256 + state + type];
    if (state == UTF8_REJECT) {
        state = UTF8_ACCEPT;
        return false;
    }
    return state == UTF8_ACCEPT;
}

bool tools::utf8::decoder::decode_symbol(uint8_t ch)
{
    state = utf8d[256 + state + utf8d[ch]];
    if (state == UTF8_REJECT) {
        state = UTF8_ACCEPT;
        return false;
    }
    return state == UTF8_ACCEPT;
}

void tools::utf8::encode_symbol(char32_t codepoint, std::string &str)
{
    if (codepoint <= 0x7f) {
        str.append(1, codepoint);
    }
    else if (codepoint <= 0x7ff ) {
        str.append(1, 0xc0 + (codepoint >> 6));
        str.append(1, 0x80 + (codepoint & 0x3f));
    }
    else if (codepoint <= 0xffff) {
        str.append(1, 0xe0 + (codepoint >> 12));
        str.append(1, 0x80 + ((codepoint >> 6) & 63));
        str.append(1, 0x80 + (codepoint & 63));
    }
    else if (codepoint <= 0x1ffff) {
        str.append(1, 0xf0 + (codepoint >> 18));
        str.append(1, 0x80 + ((codepoint >> 12) & 0x3f));
        str.append(1, 0x80 + ((codepoint >> 6) & 0x3f));
        str.append(1, 0x80 + (codepoint & 0x3f));
    }
}

std::u32string tools::utf8::decode(std::string::const_iterator begin, std::string::const_iterator end)
{
    std::u32string result;
    char32_t codepoint;
    decoder decoder;
    for (; begin != end; ++begin) {
        if (decoder.decode_symbol(*begin, codepoint)) {
            result.append(1, codepoint);
        }
    }
    return result;
}

std::u32string tools::utf8::decode(const std::string &str)
{
    return decode(str.begin(), str.end());
}

std::string tools::utf8::encode(std::u32string::const_iterator begin, std::u32string::const_iterator end)
{
    std::string result;
    for (; begin != end; ++begin) {
        encode_symbol(*begin, result);
    }
    return result;
}

std::string tools::utf8::encode(const std::u32string &str)
{
    return encode(str.begin(), str.end());
}

size_t tools::utf8::strlen(std::string::const_iterator begin, std::string::const_iterator end)
{
    size_t len = 0;
    decoder decoder;
    for (; begin != end; ++begin) {
        if (decoder.decode_symbol(*begin)) {
            ++len;
        }
    }
    return len;
}

size_t tools::utf8::strlen(const std::string &str)
{
    return strlen(str.begin(), str.end());
}

size_t tools::utf8::strlen(const char *str)
{
    size_t len = 0;
    decoder decoder;
    for (char ch; (ch = *str); ++str) {
        if (decoder.decode_symbol(ch)) {
            ++len;
        }
    }
    return len;
}

char32_t tools::utf8::at(size_t idx, const std::string &str)
{
    decoder decoder;
    char32_t codepoint;
    for (auto ch : str) {
        if (decoder.decode_symbol(ch, codepoint) && !idx--) {
            return codepoint;
        }
    }
    throw std::out_of_range("utf8: out of range");
}

char32_t tools::utf8::at(size_t idx, const char *str)
{
    decoder decoder;
    char32_t codepoint;
    for (char ch; (ch = *str); ++str) {
        if (decoder.decode_symbol(ch, codepoint) && !idx--) {
            return codepoint;
        }
    }
    throw std::out_of_range("utf8: out of range");
}

std::string::const_iterator tools::utf8::next(std::string::const_iterator begin, std::string::const_iterator end, size_t n)
{
    decoder decoder;
    while (n > 0 && begin != end) {
        if (decoder.decode_symbol(*begin++)) {
            --n;
        }
    }
    return begin;
}

std::string::const_iterator tools::utf8::next(std::string::const_iterator it, size_t n)
{
    decoder decoder;
    while (n > 0) {
        if (decoder.decode_symbol(*it++)) {
            --n;
        }
    }
    return it;
}
