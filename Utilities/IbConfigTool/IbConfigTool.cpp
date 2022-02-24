// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <cctype>
#include <map>

#include "Config.hpp"
#include "YamlConfig.hpp"

namespace {

struct ConversionConfig 
{
    ib::cfg::Config config;
    std::string launchConfigurations; //!< unparsed launch configs
};

auto EmitYamlFormat(YAML::Node node, std::shared_ptr<YAML::Emitter> out = {}) -> std::string
{
    bool isTopLevel = false;
    if (!out)
    {
        isTopLevel = true;
        out = std::make_shared<YAML::Emitter>();
    }

    if (node.IsScalar())
    {
        *out << node;
    }
    else if (node.IsMap())
    {
        *out << YAML::BeginMap;
        for (auto kv : node)
        {
            *out << YAML::Key << kv.first.Scalar();
            if (kv.second.IsMap() || kv.second.IsSequence())
            {
                *out << YAML::Value;
                EmitYamlFormat(kv.second, out);
            }
            else
            {
                *out << YAML::Value << kv.second.Scalar();
            }
        }
        *out << YAML::EndMap;
    }
    else if (node.IsSequence())
    {
        *out << YAML::BeginSeq;
        for (auto el : node)
        {
            if (el.IsMap() || el.IsSequence())
            {
                *out << YAML::Value;
                EmitYamlFormat(el, out);
            }
            else
            {
                *out << YAML::Value << el.Scalar();
            }
        }
        *out << YAML::EndSeq;
    }
    // we only return non-empty string if we are the top-most invocation
    if (isTopLevel)
    {
        return out->c_str();
    }
    else
    {
        return {};
    }
}
auto ConvertFromFile(const std::string& fileName, bool outputAsJson) -> ConversionConfig
{
    ConversionConfig coco;
    //accepts json and yaml, legacy and current format
    coco.config = ib::cfg::Config::FromYamlFile(fileName);

    // at this point the config should be valid, however we ignored possible LaunchConfigurations so far

    auto readFile = [](const auto& filename)
    {
        std::ifstream fs(filename);

        if (!fs.is_open())
            throw ib::configuration_error("Invalid IB config filename '" + filename + "'");

        std::stringstream buffer;
        buffer << fs.rdbuf();

        return buffer.str();
    };
    auto node = YAML::Load(readFile(fileName));
    if (node.IsMap() && node["LaunchConfigurations"])
    {
        if (outputAsJson)
        {
            coco.launchConfigurations = ib::cfg::yaml_to_json(node["LaunchConfigurations"]);
        }
        else
        {
            coco.launchConfigurations = EmitYamlFormat(node["LaunchConfigurations"]);
        }
    }
    return coco;
}

auto ConvertTo(const ConversionConfig& coco, bool outputAsJson) -> std::string
{
    auto yamlDoc = ib::cfg::to_yaml(coco.config);
    if (coco.launchConfigurations.size() > 0)
    {
        try {
            yamlDoc["LaunchConfigurations"] = YAML::Load(coco.launchConfigurations);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: parsing of \"LaunchConfigurations\" failed: "
                << e.what()
                << "\n>>> The config was read as follows: \n"
                << coco.launchConfigurations
                << "\n>>> end." 
                << std::endl;
            ::exit(-1);
        }
    }
    if (outputAsJson)
    {
        return ib::cfg::yaml_to_json(yamlDoc);
    }
    else
    {
        return YAML::Dump(yamlDoc);
    }
}

void usage(const std::string& programName)
{
    const int optWidth = 20, argWidth = 20, docWidth = 45;
    using namespace std;
    cout 
        << "Usage: " <<  programName << "[options]" << endl
        << "Vector Integration Bus config tool converts between JSON and YAML format and "
        << " upgrades old config files"<< endl
        //<< "VIB version: " << ib::version::String() << endl
        << "where options is one of: " << endl
        << setw(optWidth) << left << "Option" 
        << setw(argWidth) << left << "Arguments "
        << setw(docWidth) << left << "Description"
        << endl
        // print usage
        << setw(optWidth) << left << "--help|-h" 
        << setw(argWidth) << left << " "
        << setw(docWidth) << left << "Print this message."
        << endl
        // --validate
        << setw(optWidth) << left << "--validate" 
        << setw(argWidth) << left << "file1 [file2...]"
        << setw(docWidth) << left << "Validate the specified files and print warnings and errors encountered"
        << endl
        // --convert
        << setw(optWidth) << left << "--convert" 
        << setw(argWidth) << left << "oldFile outputFile"
        << setw(docWidth) << left << "Convert the legacy `oldFile` to current config format and output into `outputFile`."
        << endl
        // --format
        << setw(optWidth) << left << "--format" 
        << setw(argWidth) << left << "yaml|json"
        << setw(docWidth) << left << "Specify the output format for the '--convert' operation here. default: json"
        << endl
        ;
}

void validate(const std::vector<std::string> files)
{
    for (const auto& fileName : files)
    {
        try
        {
            auto config = ib::cfg::Config::FromYamlFile(fileName);
            auto str = config.ToYamlString();
            auto config2 = ib::cfg::Config::FromYamlString(str);
            config2.configFilePath = fileName;
            if (!(config == config2))
            {
                throw std::runtime_error("serialize/deserialize returned a different config object!");
            }
            std::cout << "OK: " << fileName << std::endl;
        }
        catch (const std::exception& ex)
        {
            std::cout << "Failed to validate: " << fileName
                << ": " << ex.what()
                << std::endl;
        }
    }
}
void convert(const std::string& inFile, const std::string& outFile, bool outputAsJson)
{
    auto fileIsJson = [](const std::string& fileName) {
        const std::string suffix{ ".json" };
        auto idx = fileName.rfind(suffix);
        if (idx == fileName.npos) return false;
        return idx == (fileName.size() - suffix.size());
    };
    const bool inputJson = fileIsJson(inFile);
    std::cout << "Converting " 
        << "'" << inFile << "' (" << (inputJson ? "json" :"yaml") << ") to "
        << "'" << outFile  << "'"
        << " with output format " << (outputAsJson ? "json" : "yaml")
        << std::endl;

    auto cfg = ConvertFromFile(inFile, outputAsJson);

    //create output file
    std::fstream out{ outFile, out.out };
    if (!out.good())
    {
        std::cout << "Convert: cannot create output file '" << outFile << "'" << std::endl;
        return;
    }
    out << ConvertTo(cfg, outputAsJson);
    out.close();
}

bool parseOutputformat(const std::string& arg, bool& outputAsJson)
{
    bool result = true;
    std::string lowerArg;
    std::transform(arg.begin(), arg.end(), std::back_inserter(lowerArg),
            [](const auto ch) { return std::tolower(ch); });
    if (lowerArg == "yaml")
    {
        outputAsJson = false;
    }
    else if (lowerArg == "json")
    {
        outputAsJson = true;
    }
    else
    {
        std::cout << "ERROR: unknown argument to --format: \"" 
            << arg << "\". Expected on of 'yaml' or 'json'" << std::endl;
        result = false;
    }
    return result;
}
}// end anonymous ns


int main(int argc, char** argv)
{
    //parse command options
    enum Action
    {
        Invalid,
        Convert,
        Validate,
    };

    Action action{ Invalid };
    int result = EXIT_SUCCESS;
    int fileListStart = 0;
    std::string fromFile;
    std::string toFile;
    bool asJson{ true };

    std::vector<std::string> args;
    std::copy(argv + 1, argv + argc, std::back_inserter(args));
    for (auto i = 0; i < args.size(); i++)
    {
        const auto& arg = args.at(i);
        if (arg == "-h" || arg == "--help")
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (arg == "--validate")
        {
            action = Validate;
            fileListStart = i + 1;
            //consume all other arguments as file names
            break;
        }
        else if (arg == "--convert")
        {
            if (i + 2 >= args.size()) 
            {
                std::cout << "ERROR: --convert requires two arguments: 'oldFile' and 'outputFile'" << std::endl;
                return EXIT_FAILURE;
            }
            action = Convert;
            fromFile = args.at(i + 1);
            toFile = args.at(i + 2);
            //shift to next arg
            i += 2;
            continue;
        }
        else if (arg == "--format")
        {
            if (i + 1 >= args.size())
            {
                std::cout << "ERROR: --format requires a single argument of 'json' or 'yaml'" << std::endl;
                return EXIT_FAILURE;
            }
            if (!parseOutputformat(args.at(i + 1), asJson))
            {
                return EXIT_FAILURE;
            }
            i+=1;
            continue;
        }
        else
        {
            usage(argv[0]);
            std::cout << "ERROR: unknown argument: " << arg << std::endl;
            return EXIT_FAILURE;
        }
    }
    switch (action)
    {
    case  Validate:
        validate({ args.begin() + fileListStart, args.end() });
        break;
    case Convert:
        try {
            convert(fromFile, toFile, asJson);
        }
        catch (const std::exception& ex)
        {
            std::cout << "ERROR: caught exception: " << ex.what() << std::endl;
            std::cout << "Aborting." << std::endl;
            return EXIT_FAILURE;
        }
        break;
    case Invalid:
        //[[fallthrough]]
    default:
        usage(argv[0]);
        std::cout << "ERROR: No action given: specify one of '--convert' or '--validate'" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#endif // 0
