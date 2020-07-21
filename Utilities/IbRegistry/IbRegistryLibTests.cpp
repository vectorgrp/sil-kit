#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdlib>
#include <fstream>
#include <vector>
#include <iterator>

#if defined(_MSC_VER)
#include <io.h>
#define mktemp _mktemp
#endif

#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "GetTestPid.hpp"
#include "CreateComAdapter.hpp"


#include "ib/extensions/CreateExtension.hpp"

#if defined(WIN32)
// without underscore is deprecated on windows
#define getcwd _getcwd
#define chdir _chdir
#endif

using namespace testing;
using namespace std::chrono_literals;
using namespace ib::cfg;
using namespace ib::mw;

using namespace ib::extensions;

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
        SetCurrentWorkingDir(currentWorkingDir);
    }

    const Config GetConfig() const { return _config; }
    static void SetUpTestCase() { currentWorkingDir = GetCurrentWorkingDir(); }

    static std::string currentWorkingDir;

private:

    void CreateConfig()
    {
        ConfigBuilder builder{ "IbRegistryLib Test" };
        builder
            .SimulationSetup()
            .AddParticipant("P1")
            .AddParticipantController()
            .WithSyncType(SyncType::DistributedTimeQuantum);

        builder
            .SimulationSetup()
            .AddParticipant("P2")
            .AddParticipantController()
            .WithSyncType(SyncType::DistributedTimeQuantum);

        builder.SimulationSetup().ConfigureTimeSync().WithTickPeriod(1ms);
        builder.WithActiveMiddleware(Middleware::VAsio);
        _config = builder.Build();
    }

    Config _config;
};

std::string IbRegistryLibFixture::currentWorkingDir;


// Load registry from the search path specified in the extension configuration
TEST_F(IbRegistryLibFixture, load_registry_custom_search_path)
{
    try
    {
        SetCurrentWorkingDir("..");

        auto config = GetConfig();
        //NB this only throws because it is the first invocation of CreateIbRegistry, once the extension is found and cached this won't throw anymore
        EXPECT_THROW(CreateIbRegistry(config), std::runtime_error);

        config.extensionConfig.searchPathHints.emplace_back(currentWorkingDir);
        ASSERT_TRUE(CreateIbRegistry(config));

        SetCurrentWorkingDir(currentWorkingDir);
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
TEST_F(IbRegistryLibFixture, ensure_registry_works)
{
    auto domainId = static_cast<uint32_t>(GetTestPid());

    std::promise<void> allConnected;
    std::promise<void> allDisconnected;
    const auto numIterations = 5;

    auto registry = CreateIbRegistry(GetConfig());

    registry->SetAllConnectedHandler([&allConnected]() {
        std::cout << "connected\n";  allConnected.set_value(); 
    });
    registry->SetAllDisconnectedHandler([&allDisconnected]() {
        std::cout << "disconnected\n";  allDisconnected.set_value();
    });
    registry->ProvideDomain(domainId);

    auto RunParticipant = [this, domainId](auto name) {
        auto comAdapter = CreateComAdapterImpl(GetConfig(), name);
        comAdapter->joinIbDomain(domainId);
        auto participantController = comAdapter->GetParticipantController();
        participantController->SetSimulationTask([](auto /*now*/, auto /*duration*/) {});
        participantController->RunAsync();
        return comAdapter;
    };

    for (auto i = 0; i < numIterations; i++)
    {
        auto allConnectedFuture = allConnected.get_future();
        auto allDisconnectedFuture = allDisconnected.get_future();
        {
            auto p1 = RunParticipant("P1");
            auto p2 = RunParticipant("P2");
            auto status = allConnectedFuture.wait_for(60s);
            ASSERT_EQ(status, std::future_status::ready);
        }
        auto status = allDisconnectedFuture.wait_for(60s);
        ASSERT_EQ(status, std::future_status::ready);

        // reset promises
        allConnected = std::promise<void>{};
        allDisconnected = std::promise<void>{};
    }

}
