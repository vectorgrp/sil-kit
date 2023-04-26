/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "SilKitExtensions.hpp"

#include <sstream>
#include <cstdlib> //getenv
#include <iostream>
#include <type_traits>

#include "SilKitVersionImpl.hpp"

#include "detail/LoadExtension.hpp"
#include "SilKitExtensionBase.hpp"
#include "SilKitExtensionMacros.hpp"
#include "SilKitExtensionUtils.hpp"

namespace SilKit {


namespace {

using BuildInfoType = decltype(SilKitExtensionDescriptor_t::build_infos);

std::string to_string(const BuildInfoType& bi)
{
    std::stringstream ss;
    ss << "C++=" << bi[static_cast<int>(BuildInfoField::Cxx)] << ", "
       << "Compiler=" << bi[static_cast<int>(BuildInfoField::Compiler)] << ", "
       << "Multithread=" << bi[static_cast<int>(BuildInfoField::Multithread)] << ", "
       << "Debug=" << bi[static_cast<int>(BuildInfoField::Debug)];
    return ss.str();
}

bool isBuildCompatible(const BuildInfoType& myInfos, const BuildInfoType& otherInfos)
{
    auto ok = true;
    for (auto i = 0u; i < sizeof(BuildInfoType) / sizeof(std::remove_all_extents_t<BuildInfoType>); i++)
    {
        ok &= (myInfos[i] == otherInfos[i]);
    }
    return ok;
}

void VerifyExtension(SilKit::Services::Logging::ILogger* logger, const SilKitExtensionDescriptor_t* descr)
{
    if (descr == nullptr)
    {
        throw ExtensionError("Extension returned invalid SilKitExtensionDescriptor");
    }
    //verify the extensions SILKIT version to ours
    if (descr->silkit_version_major != SilKit::Version::MajorImpl()
        || descr->silkit_version_minor != SilKit::Version::MinorImpl()
        || descr->silkit_version_patch != SilKit::Version::PatchImpl())
    {
        std::stringstream ss;
        ss << "Version mismatch: host SILKIT version is: " << SilKit::Version::MajorImpl() << "."
           << SilKit::Version::MinorImpl() << "." << SilKit::Version::PatchImpl()
           << " module has version: " << descr->silkit_version_major << "." << descr->silkit_version_minor << "."
           << descr->silkit_version_patch << ".";

        throw ExtensionError(ss.str());
    }

    //verfiy build infos
    const BuildInfoType ourBuild = SILKIT_MAKE_BUILDINFOS();
    const auto& extensionBuild = descr->build_infos;

    if (!isBuildCompatible(ourBuild, extensionBuild))
    {
        std::stringstream ss;
        ss << "Build information mismatch: host build info is: " << to_string(ourBuild)
           << ", module build info is: " << to_string(extensionBuild) << ".";
        if (logger)
        {
            logger->Warn(ss.str());
        }
        throw ExtensionError(ss.str());
    }

    const std::string my_sys{BuildinfoSystem()};
    const std::string mod_sys{descr->system_name};
    if (my_sys == "UNKNOWN")
    {
        if (logger)
        {
            logger->Warn("SILKIT extension verification: build system is misconfigured, the host system is UNKNOWN");
        }
    }

    if (my_sys != mod_sys)
    {
        std::stringstream msg;
        msg << "SILKIT extension verification: Host system '" << my_sys << "' differs from module system '" << mod_sys
            << "'";
        if (logger)
        {
            logger->Warn(msg.str());
        }
    }
}

//! \brief The shared library look up only uses some OS specific bits, like path
//         separators, library prefix and file extensions from the detail impl.
//The search paths can be changed using the hints argument.

std::vector<std::string> FindLibrary(const std::string& name, const SilKit::ExtensionPathHints& hints)
{
    const std::string debug_suffix = "d";
    const std::vector<std::string> candidates{name,
                                              //Debug libs
                                              detail::lib_prefix + name + debug_suffix + detail::lib_file_extension,
                                              name + debug_suffix + detail::lib_file_extension,
                                              //Release libs
                                              detail::lib_prefix + name + detail::lib_file_extension,
                                              name + detail::lib_file_extension};
    std::vector<std::string> rv;

    for (const auto& hint : hints)
    {
        std::string dir = hint;
        if (hint.substr(0, 4) == "ENV:")
        {
            // dereference environment variable
            char* envhint = ::getenv(hint.substr(4).c_str());
            if (envhint == nullptr)
                continue;

            dir.assign(envhint);
        }
        //for each path hint, check all possible module names for the load
        //config
        for (const auto& path : candidates)
        {
            //eg, result looks like "./libDummyModuled.so"
            const auto current = dir + detail::path_sep + path;
            if (detail::FileExists(current))
            {
                rv.push_back(current);
            }
        }
    }
    return rv;
}

template <typename SymType>
SymType* GetSymbol(detail::LibraryHandle hnd, const std::string& sym_name)
{
    //find the module's entry symbols, return properly typed function pointer
    void* sym = detail::FindSymbol(hnd, sym_name);
    return reinterpret_cast<SymType*>(sym); //NB this is allowed in C++11/14.
}

} //end anonymous namespace

///////////////////////////////////////////////////////////////////////////

auto LoadExtension(Services::Logging::ILogger* logger, const std::string& name) -> std::shared_ptr<ISilKitExtension>
{
    return LoadExtension(logger, name, Config::Extensions{});
}

auto LoadExtension(Services::Logging::ILogger* logger, const std::string& name, const Config::Extensions& config)
    -> std::shared_ptr<ISilKitExtension>
{
    using namespace detail;

    ExtensionPathHints searchPathHints = config.searchPathHints;
    searchPathHints.emplace_back("ENV:SILKIT_EXTENSION_PATH");
    searchPathHints.emplace_back(".");
    searchPathHints.emplace_back(GetProcessPath());

    auto paths = FindLibrary(name, searchPathHints);
    detail::LibraryHandle lib_handle = nullptr;

    auto check_lib = [logger](const std::string& path) -> detail::LibraryHandle {
        try
        {
            auto lib = OpenLibrary(path);
            //find required C symbols for extension entry
            auto extension_descr = GetSymbol<decltype(silkit_extension_descriptor)>(lib, "silkit_extension_descriptor");
            VerifyExtension(logger, extension_descr);
            return lib;
        }
        catch (const ExtensionError& ex)
        {
            std::stringstream msg;
            msg << "Failed to verify SILKIT extension located at path'" << path << "': " << ex.what();
            if (logger)
            {
                logger->Debug(msg.str());
            }
            return nullptr;
        }
    };

    //verify that at least one library is found in the candidate paths, it is
    //loadable and the build infos are compatible.
    for (const auto& path : paths)
    {
        lib_handle = check_lib(path);
        if (lib_handle != nullptr)
        {
            std::stringstream msg;
            msg << "Loaded SILKIT extension '" << name << "' from path '" << path << "'";
            if (logger)
            {
                logger->Info(msg.str());
            }
            break;
        }
    }

    if (lib_handle == nullptr)
    {
        std::stringstream ss;
        for (const auto& path : searchPathHints)
        {
            ss << path << ", ";
        }
        throw ExtensionError("No loadable library found in the following paths: " + ss.str());
    }

    // now create an instance and attach the library cleanup routine as
    // a deleter to the unique pointer.
    auto* create_ext = GetSymbol<decltype(CreateExtension)>(lib_handle, "CreateExtension");
    auto* release_ext = GetSymbol<decltype(ReleaseExtension)>(lib_handle, "ReleaseExtension");

    auto* extension = static_cast<ISilKitExtension*>(create_ext());
    return {extension, [lib_handle, release_ext](ISilKitExtension* instance) {
                //call the cleanup code inside the module
                release_ext(instance);
                //unload the actual shared library
                CloseLibrary(lib_handle);
            }};
}


} //end namespace SilKit
