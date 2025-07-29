// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <map>

namespace SilKit {
namespace Util {
namespace Hash {

/*! \brief Calculate the combination of two hash values 
*/
inline uint64_t HashCombine(uint64_t hash1, uint64_t hash2)
{
    return hash1 ^ (hash2 + 0x9e3779b97f4a7c15 + (hash1 << 6) + (hash1 >> 2));
}

/*! \brief Calculate the hash value of a std::string
* This does not rely on size_t and std::hash and can be used to safely calculate 
* the same hash on different platforms.
* DJB2 from dj bernstein documented in http://www.cse.yorku.ca/~oz/hash.html.
*/
inline uint64_t Hash(const std::string& s)
{
    uint64_t hash = 5381;
    for (auto c : s)
    {
        hash = (hash << 5) + hash + static_cast<uint64_t>(c); // hash * 33 + c
    }
    return hash;
}

/*! \brief Calculate a hash value of a std::map<std::string, std::string>
* The calculation is done by combining the hash values of all keys and values.
*/
inline uint64_t Hash(const std::map<std::string, std::string>& mapToHash)
{
    uint64_t hash = 8231;
    for (const auto& it : mapToHash)
    {
        hash = HashCombine(hash, Hash(it.first));
        hash = HashCombine(hash, Hash(it.second));
    }
    return hash;
}

} // namespace Hash
} // namespace Util
} // namespace SilKit
