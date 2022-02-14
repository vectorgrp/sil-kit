/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#include "ib/capi/Types.h"

static void assign(std::map<std::string, std::string>& cppLabels, const ib_KeyValueList* cLabels)
{
    if (cLabels)
    {
        for (uint32_t i = 0; i < cLabels->numLabels; i++)
        {
            cppLabels.insert({cLabels->labels[i].key, cLabels->labels[i].value});
        }
    }
}

static void assign(ib_KeyValueList** cLabels, const std::map<std::string, std::string>& cppLabels)
{
    size_t numLabels = cppLabels.size();
    size_t labelsSize = sizeof(ib_KeyValueList) + (numLabels * sizeof(ib_KeyValuePair));
    *cLabels = (ib_KeyValueList*)malloc(labelsSize);
    (*cLabels)->numLabels = numLabels;

    uint32_t i = 0;
    for (auto&& kv : cppLabels)
    {
        (*cLabels)->labels[i++] = {kv.first.c_str(), kv.second.c_str()};
    }
}
