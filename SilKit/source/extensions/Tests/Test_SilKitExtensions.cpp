// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <tuple>
#include <iostream>
#include <filesystem>

#include "silkit/SilKitVersion.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ParticipantConfiguration.hpp"

#include "MockParticipant.hpp" //for DummyLogger
#include "SilKitExtensions.hpp"
#include "DummyExtension.hpp"


#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define setenv(x, y, z) SetEnvironmentVariable(x, y)
#endif

using namespace testing;

namespace {
std::string GetCurrentWorkingDir()
{
    return std::filesystem::current_path().string();
}

void SetCurrentWorkingDir(const std::string& cwd)
{
    std::filesystem::current_path(cwd);
}

class StdoutLogger : public SilKit::Core::Tests::MockLogger
{
public:
    void Info(const std::string& msg) override
    {
        std::cout << "SilKitExtensionTest: Info: " << msg << std::endl;
    }
    void Debug(const std::string& msg) override
    {
        std::cout << "SilKitExtensionTest: Debug: " << msg << std::endl;
    }
    void Warn(const std::string& msg) override
    {
        std::cout << "SilKitExtensionTest: Warn: " << msg << std::endl;
    }
    void Error(const std::string& msg) override
    {
        std::cout << "SilKitExtensionTest: Error: " << msg << std::endl;
    }
};
} // namespace

class Test_SilKitExtensions : public Test
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

std::string Test_SilKitExtensions::currentWorkingDir;

using triple = std::tuple<uint32_t, uint32_t, uint32_t>;

TEST_F(Test_SilKitExtensions, load_dummy_lib)
{
    {
        const auto testDir = std::filesystem::path{"silkit_library_test"};
        std::filesystem::current_path(testDir);
        auto dummyExtension = SilKit::LoadExtension(&logger, "DummyExtension");

        {
            auto otherInstance = SilKit::LoadExtension(&logger, "DummyExtension");
            std::cout << " created second instance of DummyExtension" << std::endl;
        }
        ASSERT_NE(dummyExtension, nullptr);
        std::string name{dummyExtension->GetExtensionName()};
        ASSERT_EQ(name, "DummyExtension");

        std::string vendor{dummyExtension->GetVendorName()};
        ASSERT_EQ(vendor, "Vector");


        triple version;
        triple reference{SilKit::Version::Major(), SilKit::Version::Minor(), SilKit::Version::Patch()};

        dummyExtension->GetVersion(std::get<0>(version), std::get<1>(version), std::get<2>(version));
        ASSERT_EQ(version, reference);
    }
    //must not crash when going out of scope
}


TEST_F(Test_SilKitExtensions, static_cast)
{
    const auto testDir = std::filesystem::path{"silkit_library_test"};
    std::filesystem::current_path(testDir);
    // test if dynamic cast of dynamic extension works
    auto extensionBase = SilKit::LoadExtension(&logger, "DummyExtension");
    auto* dummy = static_cast<DummyExtension*>(extensionBase.get());
    ASSERT_NE(dummy, nullptr);
    dummy->SetDummyValue(12345L);
    ASSERT_EQ(dummy->GetDummyValue(), 12345L);
}

TEST_F(Test_SilKitExtensions, wrong_version_number)
{
    try
    {
        auto extension = SilKit::LoadExtension(&logger, "WrongVersionExtension");
        triple version;
        triple reference{SilKit::Version::Major(), SilKit::Version::Minor(), SilKit::Version::Patch()};
        extension->GetVersion(std::get<0>(version), std::get<1>(version), std::get<2>(version));
        ASSERT_EQ(version, reference);
    }
    catch (const SilKit::ExtensionError& error)
    {
        const std::string msg{error.what()};
        std::cout << "OK: received expected version mismatch error" << std::endl;
        return;
    }
    FAIL() << "expected an ExtensionError when loading a shared library with\
        wrong version number";
}

TEST_F(Test_SilKitExtensions, wrong_build_system)
{
    auto extension = SilKit::LoadExtension(&logger, "WrongBuildSystem");
    //should print a harmless warning on stdout
}

TEST_F(Test_SilKitExtensions, multiple_extensions_loaded)
{
    const auto testDir = std::filesystem::path{"silkit_library_test"};
    std::filesystem::current_path(testDir);
    //check that multiple instances don't interfere
    auto base1 = SilKit::LoadExtension(&logger, "DummyExtension");
    auto base2 = SilKit::LoadExtension(&logger, "DummyExtension");

    auto* mod1 = static_cast<DummyExtension*>(base1.get());
    auto* mod2 = static_cast<DummyExtension*>(base2.get());

    mod1->SetDummyValue(1);
    EXPECT_NE(mod2->GetDummyValue(), 1);
    mod2->SetDummyValue(1337);
    EXPECT_NE(mod1->GetDummyValue(), 1337);
}

#if !defined(_WIN32)
TEST_F(Test_SilKitExtensions, load_from_envvar)
{
    const auto testDir = std::filesystem::path{"silkit_library_test"};
    setenv("TEST_VAR", testDir.c_str(), 1); // should be invariant
    SilKit::Config::Extensions config;
    config.searchPathHints.emplace_back("ENV:TEST_VAR");
    auto base1 = SilKit::LoadExtension(&logger, "DummyExtension", config);
    auto* mod1 = static_cast<DummyExtension*>(base1.get());
    mod1->SetDummyValue(1);
    EXPECT_EQ(mod1->GetDummyValue(), 1);
}
#endif
