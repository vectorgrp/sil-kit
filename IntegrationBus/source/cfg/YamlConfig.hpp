// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>

//fwd
namespace YAML {
    class Node;
} //end namespace YAML

namespace ib {
namespace cfg {

auto YamlToJson(const std::string& yamlString) -> std::string;
auto JsonToYaml(const std::string& jsonString) -> std::string;

void Validate(YAML::Node& yamlDoc);
//fwd
class CfgElem;

// Helper class to validate the YAML documents structure
// similar to a JSON schema.
class YamlValidator
{
public:
    YamlValidator();
    bool IsTopLevelElement(const std::string& elementName) const;
    bool IsChildOf(const std::string& elementName,  const std::string& parentCandidate) const;
    bool IsValidElement(const std::string& elementName) const;
private:
    std::shared_ptr<CfgElem> _rootElem;
};

} // namespace cfg
} // namespace ib
