// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <map>

namespace ib {
namespace util {
namespace hash {

/*! \brief Calculate the combination of two hash values 
*/
uint64_t HashCombine(uint64_t hash1, uint64_t hash2);

/*! \brief Calculate the hash value of a std::string
* This does not rely on size_t and std::hash and can be used to safely calculate 
* the same hash on different platforms.
* DJB2 from dj bernstein documented in http://www.cse.yorku.ca/~oz/hash.html.
*/
uint64_t Hash(const std::string& s);

/*! \brief Calculate a hash value of a std::map<std::string, std::string>
* The calculation is done by combining the hash values of all keys and values.
*/
uint64_t Hash(const std::map<std::string, std::string>& mapToHash);

} // namespace hash
} // namespace util
} // namespace ib
