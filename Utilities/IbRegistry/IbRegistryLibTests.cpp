// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>
#include <iterator>
#if (0)
#if defined(_MSC_VER)
#include <io.h>
#define mktemp _mktemp
#endif


#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "GetTestPid.hpp"
#include "CreateComAdapter.hpp"
#include "Filesystem.hpp"

#include "ParticipantConfiguration.hpp"

#include "ib/extensions/CreateExtension.hpp"

using namespace testing;
using namespace std::chrono_literals;
using namespace ib::cfg;
using namespace ib::mw;

using namespace ib::extensions;

class IbRegistryLibFixture
    : public Test
{
public:
    IbRegistryLibFixture()
    {
        CreateConfig();
    }

    void TearDown() override
    {
        // Switch to original directory if the previous test fails
        ib::filesystem::current_path(currentWorkingDir);
    }

    const std::shared_ptr<ib::cfg::ParticipantConfiguration> GetConfig() const { return _config; }
    static void SetUpTestCase() { currentWorkingDir = ib::filesystem::current_path(); }

    static ib::filesystem::path currentWorkingDir;

private:

    void CreateConfig()
    {
        _config = std::dynamic_pointer_cast<ib::cfg::ParticipantConfiguration>(ib::cfg::CreateDummyConfiguration());
    }

    std::shared_ptr<ib::cfg::ParticipantConfiguration> _config;
};

ib::filesystem::path IbRegistryLibFixture::currentWorkingDir;


// Load registry from the search path specified in the extension configuration
TEST_F(IbRegistryLibFixture, load_registry_custom_search_path)
{
    try
    {
        auto config = GetConfig();
        const auto testDir = ib::filesystem::path{"vib_library_test"};
        bool ok = ib::filesystem::create_directory(testDir);
        std::cout << "Created temp directory \"" << testDir.string() <<"\": " << (ok ? "OK" : "FAIL") << std::endl;
        std::cout << "Current workdir (test fixture)=" << ib::filesystem::current_path().string() << std::endl;
        ib::filesystem::current_path(testDir);
        std::cout << "Current workdir (testDir)=" << ib::filesystem::current_path().string() << std::endl;

        //NB this should throw since we are now in 'testDir',
        //   which does not contain a valid shared library.
        EXPECT_THROW(CreateIbRegistry(config), std::runtime_error);

        config.extensionConfig.searchPathHints.emplace_back(currentWorkingDir.string());
        ASSERT_TRUE(CreateIbRegistry(config));
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << '\n';
        FAIL();
    }
}

TEST_F(IbRegistryLibFixture, load_registry)
{
    {
        auto registry = CreateIbRegistry(GetConfig());
        ASSERT_TRUE(registry);
    }
    auto registry = CreateIbRegistry(GetConfig());
    ASSERT_TRUE(registry);
    auto registry2 = CreateIbRegistry(GetConfig());
    ASSERT_TRUE(registry != registry2);
    // No ops as long as no config is loaded
    bool disco = false;
    bool con = false;
    registry->SetAllConnectedHandler([&con](){
        con = true;
    });
    registry->SetAllDisconnectedHandler([&disco](){
        disco = true;
    });
}

//This is taken from the integration tests, we need to make sure the 
// Registry loaded from the DLL is actually working
TEST_F(IbRegistryLibFixture, DISABLED_ensure_registry_works)
{
    //auto domainId = static_cast<uint32_t>(GetTestPid());

    //std::promise<void> allConnected;
    //std::promise<void> allDisconnected;
    //const auto numIterations = 5;

    //auto registry = CreateIbRegistry(GetConfig());

    //registry->SetAllConnectedHandler([&allConnected]() {
    //    std::cout << "connected\n";  allConnected.set_value(); 
    //});
    //registry->SetAllDisconnectedHandler([&allDisconnected]() {
    //    std::cout << "disconnected\n";  allDisconnected.set_value();
    //});
    //registry->ProvideDomain(domainId);

    //auto RunParticipant = [this, domainId](auto name) {
    //    auto comAdapter = CreateComAdapterImpl(GetConfig(), name);
    //    comAdapter->joinIbDomain(domainId);
    //    auto participantController = comAdapter->GetParticipantController();
    //    participantController->SetSimulationTask([](auto /*now*/, auto /*duration*/) {});
    //    participantController->RunAsync();
    //    return comAdapter;
    //};

    //for (auto i = 0; i < numIterations; i++)
    //{
    //    auto allConnectedFuture = allConnected.get_future();
    //    auto allDisconnectedFuture = allDisconnected.get_future();
    //    {
    //        auto p1 = RunParticipant("P1");
    //        auto p2 = RunParticipant("P2");
    //        auto status = allConnectedFuture.wait_for(60s);
    //        ASSERT_EQ(status, std::future_status::ready);
    //    }
    //    auto status = allDisconnectedFuture.wait_for(60s);
    //    ASSERT_EQ(status, std::future_status::ready);

    //    // reset promises
    //    allConnected = std::promise<void>{};
    //    allDisconnected = std::promise<void>{};
    //}
}

#endif // 0