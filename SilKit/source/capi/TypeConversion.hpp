/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "silkit/capi/Types.h"

namespace {

inline void assign(std::map<std::string, std::string>& cppLabels, const SilKit_KeyValueList* cLabels)
{
    if (cLabels)
    {
        for (uint32_t i = 0; i < cLabels->numLabels; i++)
        {
            cppLabels.insert({cLabels->labels[i].key, cLabels->labels[i].value});
        }
    }
}

inline void assign(SilKit_KeyValueList** cLabels, const std::map<std::string, std::string>& cppLabels)
{
    size_t numLabels = cppLabels.size();
    *cLabels = (SilKit_KeyValueList*)malloc(sizeof(SilKit_KeyValueList));
    if (*cLabels != NULL)
    {
        (*cLabels)->numLabels = numLabels;
        (*cLabels)->labels = (SilKit_KeyValuePair*)malloc(numLabels * sizeof(SilKit_KeyValuePair));
        if ((*cLabels)->labels != NULL)
        {
            uint32_t i = 0;
            for (auto&& kv : cppLabels)
            {
                (*cLabels)->labels[i++] = { kv.first.c_str(), kv.second.c_str() };
            }
        }
    }
}

inline void assign(std::vector<std::string>& cppStrings, const SilKit_StringList* cStrings)
{
    if (cStrings)
    {
        for (uint32_t i = 0; i < cStrings->numStrings; i++)
        {
            cppStrings.push_back(cStrings->strings[i]);
        }
    }
}

} // anonymous namespace
