// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <tuple>
#include <iostream>

#include "ib/version.hpp"
#include "ParticipantConfiguration.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockParticipant.hpp" //for DummyLogger
#include "IbExtensions.hpp"
#include "DummyExtension.hpp"
#include "Filesystem.hpp"


#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define setenv(x, y, z) SetEnvironmentVariable(x, y)
#endif

using namespace testing;

namespace
{
std::string GetCurrentWorkingDir()
{
    return ib::filesystem::current_path().string();
}

void SetCurrentWorkingDir(const std::string& cwd)
{
    ib::filesystem::current_path(cwd);
}

class StdoutLogger: public ib::mw::test::DummyLogger 
{
public:
    void Info(const std::string& msg) override
    {
        std::cout << "IbExtensionTest: Info: " << msg << std::endl;
    }
    void Debug(const std::string& msg) override
    {
        std::cout << "IbExtensionTest: Debug: " << msg << std::endl;
    }
    void Warn(const std::string& msg) override
    {
        std::cout << "IbExtensionTest: Warn: " << msg << std::endl;
    }
    void Error(const std::string& msg) override
    {
        std::cout << "IbExtensionTest: Error: " << msg << std::endl;
    }
};
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
    StdoutLogger logger;
};

std::string IbExtensionsTest::currentWorkingDir;

using triple = std::tuple<uint32_t, uint32_t, uint32_t>;

TEST_F(IbExtensionsTest, load_dummy_lib)
{
    {
        const auto testDir = ib::filesystem::path{"vib_library_test"};
        ib::filesystem::current_path(testDir);
        auto dummyExtension = ib::extensions::LoadExtension(&logger, "DummyExtension");

        {
            auto otherInstance = ib::extensions::LoadExtension(&logger, "DummyExtension");
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


TEST_F(IbExtensionsTest, dynamic_cast)
{
    const auto testDir = ib::filesystem::path{"vib_library_test"};
    ib::filesystem::current_path(testDir);
    // test if dynamic cast of dynamic extension works
    auto extensionBase = ib::extensions::LoadExtension(&logger, "DummyExtension");
    auto* dummy = dynamic_cast<DummyExtension*>(extensionBase.get());
    ASSERT_NE(dummy, nullptr);
    dummy->SetDummyValue(12345L);
    ASSERT_EQ(dummy->GetDummyValue(), 12345L);
}

TEST_F(IbExtensionsTest, wrong_version_number)
{
    try
    {
        auto extension = ib::extensions::LoadExtension(&logger, "WrongVersionExtension");
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
    auto extension = ib::extensions::LoadExtension(&logger, "WrongBuildSystem");
    //should print a harmless warning on stdout
}

TEST_F(IbExtensionsTest, multiple_extensions_loaded)
{
    const auto testDir = ib::filesystem::path{"vib_library_test"};
    ib::filesystem::current_path(testDir);
    //check that multiple instances don't interfere
    auto base1 = ib::extensions::LoadExtension(&logger, "DummyExtension");
    auto base2 = ib::extensions::LoadExtension(&logger, "DummyExtension");

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
    const auto testDir = ib::filesystem::path{"vib_library_test"};
    setenv("TEST_VAR", testDir.c_str(), 1); // should be invariant
    ib::cfg::Extensions config;
    config.searchPathHints.emplace_back("ENV:TEST_VAR");
    auto base1 = ib::extensions::LoadExtension(&logger, "DummyExtension", config);
    auto* mod1 = dynamic_cast<DummyExtension*>(base1.get());
    mod1->SetDummyValue(1);
    EXPECT_EQ(mod1->GetDummyValue(), 1);

}
#endif
