#pragma once


#include "IIoContext.hpp"

#include "MakeAsioIoContext.hpp"

#include "ILogger.hpp"

#include "asio.hpp"

#include <atomic>
#include <mutex>
#include <unordered_set>

#include <cstdint>


namespace VSilKit {


class AsioIoContext final : public IIoContext
{
    AsioSocketOptions _socketOptions;
    asio::io_context _ioContext;
    SilKit::Services::Logging::ILogger* _logger{nullptr};

public:
    explicit AsioIoContext(const AsioSocketOptions& socketOptions);
    ~AsioIoContext() override;

public: // IIoContext
    void Run() override;
    void Post(std::function<void()> function) override;
    void Dispatch(std::function<void()> function) override;
    auto ConnectTcp(const std::string& address, uint16_t port, std::error_code& errorCode)
        -> std::unique_ptr<IRawByteStream> override;
    auto ConnectLocal(const std::string& path, std::error_code& errorCode) -> std::unique_ptr<IRawByteStream> override;
    auto MakeTcpAcceptor(const std::string& address, uint16_t port) -> std::unique_ptr<IAcceptor> override;
    auto MakeLocalAcceptor(const std::string& path) -> std::unique_ptr<IAcceptor> override;
    auto MakeTimer() -> std::unique_ptr<ITimer> override;
    auto Resolve(const std::string& name) -> std::vector<std::string> override;
    void SetLogger(SilKit::Services::Logging::ILogger& logger) override;
};


} // namespace VSilKit
