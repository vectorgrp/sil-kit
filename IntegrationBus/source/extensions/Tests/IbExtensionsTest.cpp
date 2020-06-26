// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <tuple>

#include "ib/version.hpp"
#include "ib/cfg/ExtensionConfigBuilder.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "IbExtensions.hpp"
#include "DummyExtension.hpp"


#if defined(WIN32)
// without underscore is deprecated on windows
#define getcwd _getcwd
#define chdir _chdir
#endif

using namespace testing;

namespace
{
    std::string GetCurrentWorkingDir()
    {
        constexpr size_t maxPath = 4096;
        char buffer[maxPath];
        if (getcwd(buffer, maxPath) == nullptr)
        {
            throw std::runtime_error("Couldn't get current working directory.");
        }
        return std::string(buffer);
    }
    void SetCurrentWorkingDir(const std::string& cwd)
    {
        if (chdir(cwd.c_str()) != 0)
        {
            throw std::runtime_error("Couldn't set the current working directory.");
        }
    }
}

class IbExtensionsTest : public Test
{
protected:
    void TearDown() override
    {
        // Switch to original directory if the previous test fails
        SetCurrentWorkingDir(currentWorkingDir);
    }

    static void SetUpTestCase()
    {
        currentWorkingDir = GetCurrentWorkingDir();
    }

    static std::string currentWorkingDir;
};

std::string IbExtensionsTest::currentWorkingDir;

using triple = std::tuple<uint32_t, uint32_t, uint32_t>;

TEST_F(IbExtensionsTest, load_dummy_lib)
{
    {
        auto dummyExtension = ib::extensions::LoadExtension("DummyExtension");

        {
            auto otherInstance = ib::extensions::LoadExtension("DummyExtension");
            std::cout <<" created second instance of DummyExtension" << std::endl;
        }
        ASSERT_NE(dummyExtension, nullptr);
        std::string name{dummyExtension->GetExtensionName()};
        ASSERT_EQ(name,  "DummyExtension");

        std::string vendor{dummyExtension->GetVendorName()};
        ASSERT_EQ(vendor,  "Vector");


        triple version;
        triple reference{ib::version::Major(),
                 ib::version::Minor(), ib::version::Patch()};

        dummyExtension->GetVersion(std::get<0>(version), std::get<1>(version),
                         std::get<2>(version));
        ASSERT_EQ(version, reference);

    }
    //must not crash when going out of scope
}

// Use the search path hints specified in the extension configuration
TEST_F(IbExtensionsTest, load_dummy_lib_from_custom_search_path)
{
    try
    {
        SetCurrentWorkingDir("..");

        ib::cfg::ExtensionConfig config;
        EXPECT_THROW(ib::extensions::LoadExtension("DummyExtension", config), ib::extensions::ExtensionError);

        config.searchPathHints.emplace_back(currentWorkingDir);
        ASSERT_NE(ib::extensions::LoadExtension("DummyExtension", config), nullptr);
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
        FAIL();
    }
}

TEST_F(IbExtensionsTest, dynamic_cast)
{
    //test wether dynamic cast of dynamic extension works
    auto extensionBase = ib::extensions::LoadExtension("DummyExtension");
    auto* dummy = dynamic_cast<DummyExtension*>(extensionBase.get());
    ASSERT_NE(dummy, nullptr);
    dummy->SetDummyValue(12345L);
    ASSERT_EQ(dummy->GetDummyValue(), 12345L);
}

TEST_F(IbExtensionsTest, wrong_version_number)
{
    try
    {
        auto extension = ib::extensions::LoadExtension("WrongVersionExtension");
        triple version;
        triple reference{
            ib::version::Major(),
            ib::version::Minor(),
            ib::version::Patch()
        };
        extension->GetVersion(std::get<0>(version), std::get<1>(version),
                std::get<2>(version));
        ASSERT_EQ(version, reference);
    }
    catch (const ib::extensions::ExtensionError& error)
    {
        const std::string msg{error.what()};
        std::cout << "OK: received expected version mismatch error"
            << std::endl;
        return;
    }
    FAIL() << "expected an ExtensionError when loading a shared library with\
        wrong version number";

}

TEST_F(IbExtensionsTest, wrong_build_system)
{
    auto extension = ib::extensions::LoadExtension("WrongBuildSystem");
    //should print a harmless warning on stdout
}

TEST_F(IbExtensionsTest, multiple_extensions_loaded)
{
    //check that multiple instances don't interfere
    auto base1 = ib::extensions::LoadExtension("DummyExtension");
    auto base2 = ib::extensions::LoadExtension("DummyExtension");

    auto* mod1 = dynamic_cast<DummyExtension*>(base1.get());
    auto* mod2 = dynamic_cast<DummyExtension*>(base2.get());

    mod1->SetDummyValue(1);
    EXPECT_NE(mod2->GetDummyValue(), 1);
    mod2->SetDummyValue(1337);
    EXPECT_NE(mod1->GetDummyValue(), 1337);
}

#if !defined(_WIN32)
TEST_F(IbExtensionsTest, load_from_envvar)
{
    SetCurrentWorkingDir("..");
    setenv("TEST_VAR", "Tests", 1); //should be invariant
    ib::cfg::ExtensionConfig config;
    config.searchPathHints.emplace_back("ENV:TEST_VAR");
    auto base1 = ib::extensions::LoadExtension("DummyExtension", config);
    auto* mod1 = dynamic_cast<DummyExtension*>(base1.get());
    mod1->SetDummyValue(1);
    EXPECT_EQ(mod1->GetDummyValue(), 1);

}
#endif
