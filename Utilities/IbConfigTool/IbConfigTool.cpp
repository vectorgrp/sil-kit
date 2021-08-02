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


//internal APIs
#include "YamlConfig.hpp" 

// legacy json parser
#include "JsonConfig.hpp"

namespace legacy {
//!< Legacy Json parsers
auto FromJsonString(const std::string& jsonString) -> ib::cfg::Config
{
    using namespace ib::cfg;

    std::string errorString;
    auto&& json = json11::Json::parse(jsonString, errorString);

    if (json.is_null())
    {
        std::cerr << "Error Parsing json: " << jsonString << "\n";
        throw Misconfiguration("IB config parsing error");
    }

    auto config = from_json<Config>(json);
    config.configFilePath.clear();

    PostProcess(config);

    return config;
}

auto ReadWholeFile(const std::string& filename) -> std::string
{
    std::ifstream fs(filename);

    if (!fs.is_open())
        throw ib::cfg::Misconfiguration("Invalid IB config filename '" + filename + "'");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return buffer.str();

}

auto FromJsonFile(const std::string& jsonFilename) -> ib::cfg::Config
{
    auto jsonString = ReadWholeFile(jsonFilename);
    auto&& config = legacy::FromJsonString(jsonString);
    config.configFilePath = jsonFilename;

    return config;
}

} // end legacy

namespace {

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
    ib::cfg::Config cfg;
    const bool inputJson = fileIsJson(inFile);
    std::cout << "Converting " 
        << "'" << inFile << "' (" << (inputJson ? "json" :"yaml") << ") to "
        << "'" << outFile  << "'"
        << " with output format " << (outputAsJson ? "json" : "yaml")
        << std::endl;
    if (inputJson)
    {
        // read the config using the old JSON parser.
        // The old implementation supports legacy adjustments.
        cfg = legacy::FromJsonFile(inFile);
    }
    else
    {
        //YAML files will be implicitly validated:
        cfg = ib::cfg::Config::FromYamlFile(inFile);
    }

    //create output file
    std::fstream out{ outFile, out.out };
    if (!out.good())
    {
        std::cout << "Convert: cannot create output file '" << outFile << "'" << std::endl;
        return;
    }
    if(outputAsJson)
    {
        out << cfg.ToJsonString();
    }
    else
    {
        out << cfg.ToYamlString();
    }
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
