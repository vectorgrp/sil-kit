// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "YamlConfig.hpp"
#include "yaml-cpp/yaml.h"

#include "ib/cfg/Config.hpp"

#include <iostream>
#include <map>
#include <iterator>
#include <set>
#include <algorithm>

#include "YamlSchema.hpp"
#include "YamlValidator.hpp"

namespace ib {
namespace cfg {


bool Validate(const std::string& yamlString, std::ostream& warningMessages)
{
    YamlValidator validator;
    return validator.Validate(yamlString, warningMessages);
}


//!< Helper to print the YAML document position
std::ostream& operator<<(std::ostream& out, const YAML::Mark& mark)
{
    if (!mark.is_null())
    {
        out << "line " << mark.line << ", column " << mark.column;
    }
    return out;
}

} // namespace cfg
} // namespace ib
