#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "ib/cfg/Config.hpp"

int main(int argc, char** argv)
{
	for(auto i = 1; i < argc; i++)
	{
		const std::string fileName = argv[i];
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
		catch(const std::exception& ex)
		{
			std::cout <<"Failed to validate: " << fileName
				<<": " <<ex.what()
				<<std::endl;
		}
	}
	return EXIT_SUCCESS;
}
