#include "AsioIoContext.hpp"

#include "AsioAcceptor.hpp"
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
{
}


AsioIoContext::~AsioIoContext()
{
    SILKIT_TRACE_METHOD(_logger, "(...)");
}


void AsioIoContext::Run()
{
    SILKIT_TRACE_METHOD(_logger, "()");

    _ioContext.run();
}


void AsioIoContext::Post(std::function<void()> function)
{
    _ioContext.post(std::move(function));
}


void AsioIoContext::Dispatch(std::function<void()> function)
{
    _ioContext.dispatch(std::move(function));
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


auto AsioIoContext::ConnectTcp(const std::string& address, uint16_t port, std::error_code& errorCode)
    -> std::unique_ptr<IRawByteStream>
{
    SILKIT_TRACE_METHOD(_logger, "({}, {})", address, port);

    asio::ip::tcp::endpoint endpoint{asio::ip::address::from_string(address), port};
    asio::ip::tcp::socket socket{_ioContext};

    socket.open(endpoint.protocol(), errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD(_logger, "failed to open socket: {}", errorCode.message());
        return nullptr;
    }

    SetConnectOptions(_logger, socket);

    socket.connect(endpoint, errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD(_logger, "failed to connect socket: {}", errorCode.message());
        return nullptr;
    }

    SetAsioSocketOptions(_logger, socket, _socketOptions, errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD(_logger, "failed to set socket options: {}", errorCode.message());
        return nullptr;
    }

    AsioGenericRawByteStreamOptions options{};
    options.tcp.quickAck = _socketOptions.tcp.quickAck;

    return std::make_unique<AsioGenericRawByteStream>(*this, options, std::move(socket), *_logger);
}


auto AsioIoContext::ConnectLocal(const std::string& path, std::error_code& errorCode) -> std::unique_ptr<IRawByteStream>
{
    SILKIT_TRACE_METHOD(_logger, "({})", path);

    asio::local::stream_protocol::endpoint endpoint{path};
    asio::local::stream_protocol::socket socket{_ioContext};

    socket.connect(endpoint, errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD(_logger, "failed to connect socket");
        return nullptr;
    }

    AsioGenericRawByteStreamOptions options{};

    return std::make_unique<AsioGenericRawByteStream>(*this, options, std::move(socket), *_logger);
}


auto AsioIoContext::MakeTcpAcceptor(const std::string& ipAddress, uint16_t port) -> std::unique_ptr<IAcceptor>
{
    SILKIT_TRACE_METHOD(_logger, "({}, {})", ipAddress, port);

    auto address = CleanIpAddress(ipAddress);
    asio::ip::tcp::endpoint endpoint{asio::ip::make_address(address), port};
    asio::ip::tcp::acceptor acceptor{_ioContext.get_executor()};

    OpenAcceptor(acceptor, endpoint, *_logger);

    return std::make_unique<AsioAcceptor<decltype(acceptor)>>(*this, _socketOptions, std::move(acceptor), *_logger);
}


auto AsioIoContext::MakeLocalAcceptor(const std::string& path) -> std::unique_ptr<IAcceptor>
{
    SILKIT_TRACE_METHOD(_logger, "({})", path);

    asio::local::stream_protocol::endpoint endpoint{path};
    asio::local::stream_protocol::acceptor acceptor{_ioContext.get_executor()};

    OpenAcceptor(acceptor, endpoint, *_logger);

    return std::make_unique<AsioAcceptor<decltype(acceptor)>>(*this, _socketOptions, std::move(acceptor), *_logger);
}


auto AsioIoContext::MakeTimer() -> std::unique_ptr<ITimer>
{
    SILKIT_TRACE_METHOD(_logger, "()");

    asio::steady_timer timer{_ioContext.get_executor()};
    return std::make_unique<AsioTimer>(std::move(timer));
}


auto AsioIoContext::Resolve(const std::string& name) -> std::vector<std::string>
{
    SILKIT_TRACE_METHOD(_logger, "({})", name);

    std::vector<std::string> addresses;

    if (IsIpV4(name) || IsIpV6(name))
    {
        addresses.emplace_back(name);
        return addresses;
    }

    asio::ip::tcp::resolver resolver{_ioContext.get_executor()};
    for (const auto& endpoint : resolver.resolve(name, ""))
    {
        addresses.emplace_back(endpoint.endpoint().address().to_string());
    }

    return addresses;
}


void AsioIoContext::SetLogger(SilKit::Services::Logging::ILogger& logger)
{
    SILKIT_TRACE_METHOD(&logger, "({})", static_cast<const void*>(&logger));
    _logger = &logger;
}


} // namespace VSilKit
