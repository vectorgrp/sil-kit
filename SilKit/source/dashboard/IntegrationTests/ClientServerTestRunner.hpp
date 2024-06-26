/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "OatppHeaders.hpp"

#include <atomic>
#include <list>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace SilKit {
namespace Dashboard {

/**
 * Helper class to run Client-Server tests
 */
class ClientServerTestRunner
{
public:
    typedef oatpp::web::server::HttpRouter HttpRouter;
    typedef oatpp::web::server::api::ApiController ApiController;

private:
    std::shared_ptr<oatpp::network::Server> m_server;
    std::list<std::shared_ptr<ApiController>> m_controllers;
    OATPP_COMPONENT(std::shared_ptr<HttpRouter>, m_router);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, m_connectionProvider);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, m_connectionHandler);

public:
    std::shared_ptr<HttpRouter> getRouter()
    {
        return m_router;
    }

    /**
   * Add controller's endpoints to router
   * @param controller
   */
    void addController(const std::shared_ptr<ApiController>& controller)
    {
        m_router->route(controller->getEndpoints());
        m_controllers.push_back(controller);
    }

    std::shared_ptr<oatpp::network::Server> getServer()
    {
        return m_server;
    }

    /**
   * Start server, execute code block passed as lambda, stop server.
   * @tparam Lambda
   * @param lambda
   * @param timeout
   */
    template <typename Lambda>
    void run(const Lambda& lambda, const std::chrono::duration<v_int64, std::micro>& timeout = std::chrono::hours(12))
    {
        auto startTime = std::chrono::system_clock::now();
        std::atomic<bool> running(true);
        std::mutex timeoutMutex;
        std::condition_variable timeoutCondition;
        // atomic
        std::atomic<bool> runConditionForLambda(true);
        // atomic end

        m_server = std::make_shared<oatpp::network::Server>(m_connectionProvider, m_connectionHandler);
        OATPP_LOGD("\033[1;34mClientServerTestRunner\033[0m",
                   "\033[1;34mRunning server on port %s. Timeout %lld(micro)\033[0m",
                   m_connectionProvider->getProperty("port").toString()->c_str(), timeout.count());

        std::function<bool()> condition = [&runConditionForLambda]() { return runConditionForLambda.load(); };

        std::thread serverThread([&condition, this] {
            m_server->run(condition);
            // stop moved here
            m_connectionHandler->stop();
            m_connectionProvider->stop();
            // stop moved here end
        });

        std::thread clientThread([&runConditionForLambda, &lambda] {
            lambda();
            //      m_server->stop();
            runConditionForLambda = false;
        });

        std::thread timerThread([&timeout, &startTime, &running, &timeoutMutex, &timeoutCondition] {
            std::unique_lock<std::mutex> lock(timeoutMutex);
            while (running)
            {
                timeoutCondition.wait_for(lock, std::chrono::seconds(1));
                auto elapsed = std::chrono::system_clock::now() - startTime;
                OATPP_ASSERT("ClientServerTestRunner: Error. Timeout." && elapsed < timeout);
            }
        });

        serverThread.join();
        clientThread.join();

        auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - startTime);
        OATPP_LOGD("\033[1;34mClientServerTestRunner\033[0m",
                   "\033[1;34mFinished with time %lld(micro). Stopping server...\033[0m", elapsed.count());

        running = false;
        timeoutCondition.notify_one();

        timerThread.join();
    }
};

} // namespace Dashboard
} // namespace SilKit
