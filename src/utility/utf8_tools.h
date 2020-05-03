/*
Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT

Copyright (c) 2020 Ivan Volnov

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

#ifndef UTF8_TOOLS_H
#define UTF8_TOOLS_H

#include <string>


namespace tools::utf8 {


class decoder
{
public:
    bool decode_symbol(uint8_t ch, char32_t &codepoint);
    bool decode_symbol(uint8_t ch);

private:
    uint32_t state = 0;
};


void encode_symbol(char32_t codepoint, std::string &str);

std::u32string decode(const std::string &str);
std::u32string decode(std::string::const_iterator begin, std::string::const_iterator end);

std::string encode(const std::u32string &str);
std::string encode(std::u32string::const_iterator begin, std::u32string::const_iterator end);

size_t strlen(const std::string &str);
size_t strlen(const char *str);
size_t strlen(std::string::const_iterator begin, std::string::const_iterator end);

char32_t at(size_t idx, const std::string &str);
char32_t at(size_t idx, const char *str);

std::string::const_iterator iter_at(size_t idx, const std::string &str);
std::string::const_iterator iter_at(size_t idx, std::string::const_iterator begin, std::string::const_iterator end);


} // namespace tools::utf8

#endif // UTF8_TOOLS_H
