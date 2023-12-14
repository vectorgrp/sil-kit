// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "AsioIoContext.hpp"

#include "AsioAcceptor.hpp"
#include "AsioConnector.hpp"
#include "AsioTimer.hpp"
#include "SetAsioSocketOptions.hpp"

#include "util/Exceptions.hpp"
#include "util/TracingMacros.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <regex> // IsIPv4 / IsIPv6
#include <unordered_set>

#include "asio.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioIoContext
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


namespace VSilKit {


namespace {

// only TCP/IP need platform tweaks
template <typename AcceptorT>
void SetPlatformOptions(AcceptorT&)
{
}

// local domain sockets on my WSL (Linux) require read/write permission for user
template <typename EndpointT>
void SetSocketPermissions(const EndpointT&)
{
}

template <typename AcceptorT>
void SetListenOptions(SilKit::Services::Logging::ILogger*, AcceptorT&)
{
}

template <typename SocketT>
void SetConnectOptions(SilKit::Services::Logging::ILogger*, SocketT&)
{
}

// platform specific definitions of utilities
#if defined(_WIN32)

#    include <mstcpip.h>

template <>
void SetPlatformOptions(asio::ip::tcp::acceptor& acceptor)
{
    using exclusive_addruse = asio::detail::socket_option::boolean<ASIO_OS_DEF(SOL_SOCKET), SO_EXCLUSIVEADDRUSE>;
    acceptor.set_option(exclusive_addruse{true});
}

#    if !defined(__MINGW32__)

template <>
void SetListenOptions(SilKit::Services::Logging::ILogger* logger, asio::ip::tcp::acceptor& acceptor)
{
    // This should improve loopback performance, and have no effect on remote TCP/IP
    int enabled = 1;
    DWORD numberOfBytes = 0;
    auto result = WSAIoctl(acceptor.native_handle(), SIO_LOOPBACK_FAST_PATH, &enabled, sizeof(enabled), nullptr, 0,
                           &numberOfBytes, 0, 0);

    if (result == SOCKET_ERROR)
    {
        auto lastError = ::GetLastError();
        SilKit::Services::Logging::Warn(
            logger, "SetListenOptions: Setting Loopback FastPath failed: WSA IOCtl last error: {}", lastError);
    }
}

template <>
void SetConnectOptions(SilKit::Services::Logging::ILogger* logger, asio::ip::tcp::socket& socket)
{
    // This should improve loopback performance, and have no effect on remote TCP/IP
    int enabled = 1;
    DWORD numberOfBytes = 0;
    auto result = WSAIoctl(socket.native_handle(), SIO_LOOPBACK_FAST_PATH, &enabled, sizeof(enabled), nullptr, 0,
                           &numberOfBytes, 0, 0);

    if (result == SOCKET_ERROR)
    {
        auto lastError = ::GetLastError();
        SilKit::Services::Logging::Warn(
            logger, "SetListenOptions: Setting Loopback FastPath failed: WSA IOCtl last error: {}", lastError);
    }
}

#    endif // !__MINGW32__

#else

template <>
void SetPlatformOptions(asio::ip::tcp::acceptor& acceptor)
{
    // We enable the SO_REUSEADDR flag on POSIX, this allows reusing a socket's address more quickly.
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address{true});
}

template <>
void SetSocketPermissions(const asio::local::stream_protocol::endpoint& endpoint)
{
    const auto path = endpoint.path();
    (void)chmod(path.c_str(), 0770);
}

template <>
void SetListenOptions(SilKit::Services::Logging::ILogger*, asio::ip::tcp::acceptor&)
{
    // no op
}

#endif

} // namespace


AsioIoContext::AsioIoContext(const AsioSocketOptions& socketOptions)
    : _socketOptions{socketOptions}
    , _asioIoContext{std::make_shared<asio::io_context>()}
{
}


AsioIoContext::~AsioIoContext()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
}


// IIoContext


void AsioIoContext::Run()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    if (_asioIoContext->stopped())
    {
        _asioIoContext->restart();
    }

    _asioIoContext->run();
}


void AsioIoContext::Post(std::function<void()> function)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");
    _asioIoContext->post(std::move(function));
}


void AsioIoContext::Dispatch(std::function<void()> function)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");
    _asioIoContext->dispatch(std::move(function));
}


static auto IsIpV4(const std::string& string) -> bool
{
    static std::regex regex{R"(^[0-9]+[.][0-9]+[.][0-9]+[.][0-9]+$)", std::regex::optimize};
    return std::regex_match(string, regex);
}


static auto IsIpV6(const std::string& string) -> bool
{
    static std::regex regex{R"(^\[[0-9:]+\]$)", std::regex::optimize};
    return std::regex_match(string, regex);
}

static auto CleanIpAddress(const std::string& string) -> std::string
{
    static std::regex regex{R"(^\[([0-9:]+)\]$)", std::regex::optimize};

    std::smatch matches;
    if (std::regex_match(string, matches, regex))
    {
        return matches[1].str();
    }

    return string;
}

template <typename AsioAcceptorType, typename AsioEndpointType>
void OpenAcceptor(AsioAcceptorType& acceptor, AsioEndpointType endpoint, SilKit::Services::Logging::ILogger& logger)
{
    acceptor.open(endpoint.protocol());
    SetPlatformOptions(acceptor);
    acceptor.bind(endpoint);
    SetSocketPermissions(endpoint);
    acceptor.listen();
    SetListenOptions(&logger, acceptor);
}


auto AsioIoContext::MakeTcpAcceptor(const std::string& ipAddress, uint16_t port) -> std::unique_ptr<IAcceptor>
{
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", ipAddress, port);

    auto address = CleanIpAddress(ipAddress);
    asio::ip::tcp::endpoint endpoint{asio::ip::make_address(address), port};
    asio::ip::tcp::acceptor acceptor{_asioIoContext->get_executor()};

    OpenAcceptor(acceptor, endpoint, *_logger);

    return std::make_unique<AsioAcceptor<decltype(acceptor)>>(_socketOptions, _asioIoContext, std::move(acceptor),
                                                              *_logger);
}


auto AsioIoContext::MakeLocalAcceptor(const std::string& path) -> std::unique_ptr<IAcceptor>
{
    SILKIT_TRACE_METHOD_(_logger, "({})", path);

    asio::local::stream_protocol::endpoint endpoint{path};
    asio::local::stream_protocol::acceptor acceptor{_asioIoContext->get_executor()};

    OpenAcceptor(acceptor, endpoint, *_logger);

    return std::make_unique<AsioAcceptor<decltype(acceptor)>>(_socketOptions, _asioIoContext, std::move(acceptor),
                                                              *_logger);
}


auto AsioIoContext::MakeTcpConnector(const std::string& ipAddress, uint16_t port) -> std::unique_ptr<IConnector>
{
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", ipAddress, port);

    using AsioProtocolType = asio::ip::tcp;
    using ConnectorType = AsioConnector<AsioProtocolType>;

    auto address = CleanIpAddress(ipAddress);
    AsioProtocolType::endpoint endpoint{asio::ip::make_address(address), port};

    return std::make_unique<ConnectorType>(_asioIoContext, _socketOptions, endpoint, *_logger);
}


auto AsioIoContext::MakeLocalConnector(const std::string& path) -> std::unique_ptr<IConnector>
{
    SILKIT_TRACE_METHOD_(_logger, "({})", path);

    using AsioProtocolType = asio::local::stream_protocol;
    using ConnectorType = AsioConnector<AsioProtocolType>;

    AsioProtocolType::endpoint endpoint{path};

    return std::make_unique<ConnectorType>(_asioIoContext, _socketOptions, endpoint, *_logger);
}


auto AsioIoContext::MakeTimer() -> std::unique_ptr<ITimer>
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    return std::make_unique<AsioTimer>(_asioIoContext);
}


auto AsioIoContext::Resolve(const std::string& name) -> std::vector<std::string>
{
    SILKIT_TRACE_METHOD_(_logger, "({})", name);

    std::vector<std::string> addresses;

    if (IsIpV4(name) || IsIpV6(name))
    {
        addresses.emplace_back(name);
        return addresses;
    }

    asio::ip::tcp::resolver resolver{_asioIoContext->get_executor()};
    for (const auto& endpoint : resolver.resolve(name, ""))
    {
        addresses.emplace_back(endpoint.endpoint().address().to_string());
    }

    return addresses;
}


void AsioIoContext::SetLogger(SilKit::Services::Logging::ILogger& logger)
{
    SILKIT_TRACE_METHOD_(&logger, "({})", static_cast<const void*>(&logger));
    _logger = &logger;
}


} // namespace VSilKit


#undef SILKIT_TRACE_METHOD_
