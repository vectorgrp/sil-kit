// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Hash.hpp"

namespace ib {
namespace util {
namespace hash {

uint64_t HashCombine(uint64_t hash1, uint64_t hash2)
{
    return hash1 ^ (hash2 + 0x9e3779b97f4a7c15 + (hash1 << 6) + (hash1 >> 2));
}

uint64_t Hash(const std::string& s)
{
    uint64_t hash = 5381;
    for (auto c : s)
    {
        hash = (hash << 5) + hash + c; // hash * 33 + c
    }
    return hash;
}

uint64_t Hash(const std::map<std::string, std::string>& mapToHash)
{
    uint64_t hash = 8231;
    for (const auto& it : mapToHash)
    {
        hash = HashCombine(hash, Hash(it.first));
        hash = HashCombine(hash, Hash(it.second));
    }
    return hash;
}

} // namespace hash
} // namespace util
} // namespace ib