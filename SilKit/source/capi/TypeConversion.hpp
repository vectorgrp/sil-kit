// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
