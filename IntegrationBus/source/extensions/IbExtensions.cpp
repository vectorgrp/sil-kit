// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#ifdef WIN32
#    define _CRT_SECURE_NO_WARNINGS
#endif

#include "IbExtensions.hpp"

#include <sstream>
#include <cstdlib> //getenv
#include <iostream>
#include <type_traits>

#include "ib/version.hpp"

#include "detail/LoadExtension.hpp"
#include "IbExtensionBase.hpp"
#include "IbExtensionMacros.hpp"
#include "IbExtensionUtils.hpp"


namespace ib { namespace extensions {

namespace {

using BuildInfoType =decltype(IbExtensionDescriptor_t::build_infos);

std::string to_string(const BuildInfoType&  bi)
{
    std::stringstream ss;
    ss 
        << "C++=" << bi[static_cast<int>(BuildInfoField::Cxx)] << ", "
        << "Compiler=" << bi[static_cast<int>(BuildInfoField::Compiler)] << ", "
        << "Multithread=" << bi[static_cast<int>(BuildInfoField::Multithread)] << ", "
        << "Debug=" << bi[static_cast<int>(BuildInfoField::Debug)] 
        ;
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

void VerifyExtension(ib::mw::logging::ILogger* logger, const IbExtensionDescriptor_t* descr)
{
    if(descr == nullptr)
    {
        throw ExtensionError("Extension returned invalid IbExtensionDescriptor");
    }
    //verify the extensions VIB version to ours
    if(descr->vib_version_major != ib::version::Major()
        || descr->vib_version_minor != ib::version::Minor()
        || descr->vib_version_patch != ib::version::Patch()
      )
    {
        std::stringstream ss;
        ss <<"Version mismatch: host VIB version is: "
            << ib::version::Major() <<"."
            << ib::version::Minor() <<"."
            << ib::version::Patch()
            << " module has version: "
            << descr->vib_version_major <<"."
            << descr->vib_version_minor <<"."
            << descr->vib_version_patch <<"."
            ;
            
        throw ExtensionError(ss.str());
    }

    //verfiy build infos
    const BuildInfoType ourBuild= IB_MAKE_BUILDINFOS();
    const auto& extensionBuild = descr->build_infos;

    if(!isBuildCompatible(ourBuild, extensionBuild))
    {
        std::stringstream ss;
        ss << "Build information mismatch: host build info is: "
            << to_string(ourBuild)
            << ", module build info is: "
            << to_string(extensionBuild)
            << "."
            ;
        if (logger)
        {
            logger->Warn(ss.str());
        }
        throw ExtensionError(ss.str());
    }

    const std::string my_sys{BuildinfoSystem()};
    const std::string mod_sys{descr->system_name};
    if(my_sys == "UNKNOWN")
    {
        if(logger)
        {
            logger->Warn("VIB extension verification: build system is misconfigured, the host system is UNKNOWN");
        }
    }

    if(my_sys != mod_sys)
    {
        std::stringstream msg;
        msg << "VIB extension verification: Host system '" << my_sys 
            <<"' differs from module system '" << mod_sys
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

std::vector<std::string> FindLibrary(const std::string& name,
                const ib::extensions::ExtensionPathHints& hints)
{
    const std::string debug_suffix = "d";
    const std::vector<std::string> candidates{
        name,
        //Debug libs
        detail::lib_prefix + name + debug_suffix + detail::lib_file_extension,
        name + debug_suffix + detail::lib_file_extension,
        //Release libs
        detail::lib_prefix + name + detail::lib_file_extension,
        name + detail::lib_file_extension
    };
    std::vector<std::string> rv;

    for(const auto& hint: hints)
    {
        std::string dir = hint;
        if(hint.substr(0,4) == "ENV:") {
            // dereference environment variable
            char* envhint = ::getenv(hint.substr(4).c_str());
            if(envhint == nullptr) continue;

            dir.assign(envhint);
        }
        //for each path hint, check all possible module names for the load
        //config
        for(const auto& path: candidates)
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

template<typename SymType>
SymType* GetSymbol(detail::LibraryHandle hnd, const std::string& sym_name)
{
    //find the module's entry symbols, return properly typed function pointer
    void* sym = detail::FindSymbol(hnd,sym_name);
    return reinterpret_cast<SymType*>(sym); //NB this is allowed in C++11/14. 
}

} //end anonymous namespace

///////////////////////////////////////////////////////////////////////////

auto LoadExtension(mw::logging::ILogger* logger,
    const std::string& name) -> std::shared_ptr<IIbExtension>
{
    return LoadExtension(logger, name, cfg::Extensions{});
}

auto LoadExtension(mw::logging::ILogger* logger,
    const std::string& name, const cfg::Extensions& config) -> std::shared_ptr<IIbExtension>
{
    using namespace detail;

    ExtensionPathHints searchPathHints = config.searchPathHints;
    searchPathHints.emplace_back("ENV:IB_EXTENSION_PATH");
    searchPathHints.emplace_back(".");
    searchPathHints.emplace_back(GetProcessPath());

    auto paths = FindLibrary(name, searchPathHints);
    detail::LibraryHandle lib_handle = nullptr;

    auto check_lib = [logger](const std::string &path) 
        -> detail::LibraryHandle {
        try
        {
            auto lib = OpenLibrary(path);
            //find required C symbols for extension entry
            auto extension_descr = GetSymbol<decltype(vib_extension_descriptor)>
                            (lib, "vib_extension_descriptor");
            VerifyExtension(logger, extension_descr);
            return lib;
        }
        catch(const ExtensionError& ex)
        {
            std::stringstream msg;
            msg << "Failed to verify VIB extension located at path'" << path <<"': "
                << ex.what();
            if (logger)
            {
                logger->Debug(msg.str());
            }
            return nullptr;
        }
    };

    //verify that at least one library is found in the candidate paths, it is
    //loadable and the build infos are compatible.
    for(const auto& path: paths)
    {
        lib_handle = check_lib(path);
        if(lib_handle != nullptr)
        {
            std::stringstream msg;
            msg << "Loaded VIB extension '" << name << "' from path '" << path << "'";
            if (logger)
            {
                logger->Info(msg.str());
            }
            break;
        }
    }

    if(lib_handle == nullptr)
    {
        std::stringstream ss;
        for(const auto& path: searchPathHints)
        {
            ss << path << ", ";
        }
        throw ExtensionError(
                "No loadable library found in the following paths: " + ss.str()
        );
    }

    // now create an instance and attach the library cleanup routine as
    // a deleter to the unique pointer.
    auto* create_ext = GetSymbol<decltype(CreateExtension)>
                         (lib_handle, "CreateExtension");
    auto* release_ext = GetSymbol<decltype(ReleaseExtension)>
                         (lib_handle, "ReleaseExtension");

    auto* extension = static_cast<IIbExtension*>(create_ext());
    return {
        extension,
        [lib_handle, release_ext](IIbExtension* instance)
        {
            //call the cleanup code inside the module
            release_ext(instance);
            //unload the actual shared library
            CloseLibrary(lib_handle);
        }
    };
}

}//end namespace extensions
}//end namespace ib
