// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once


#include "IVAsioPeer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Core {


struct MockVAsioPeer : IVAsioPeer
{
    // IVAsioPeer

    MOCK_METHOD(void, SendSilKitMsg, (SerializedMessage), (override));
    MOCK_METHOD(void, Subscribe, (VAsioMsgSubscriber), (override));
    MOCK_METHOD(const VAsioPeerInfo &, GetInfo, (), (const, override));
    MOCK_METHOD(void, SetInfo, (VAsioPeerInfo), (override));
    MOCK_METHOD(std::string, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(std::string, GetLocalAddress, (), (const, override));
    MOCK_METHOD(void, SetSimulationName, (const std::string &), (override));
    MOCK_METHOD(const std::string &, GetSimulationName, (), (const, override));
    MOCK_METHOD(void, StartAsyncRead, (), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
    MOCK_METHOD(void, EnableAggregation, (), (override));
    MOCK_METHOD(void, SetProtocolVersion, (ProtocolVersion), (override));
    MOCK_METHOD(ProtocolVersion, GetProtocolVersion, (), (const, override));

    // IServiceEndpoint (via IVAsioPeer)

    MOCK_METHOD(void, SetServiceDescriptor, (const ServiceDescriptor &), (override));
    MOCK_METHOD(const ServiceDescriptor &, GetServiceDescriptor, (), (const, override));
};


} // namespace Core
} // namespace SilKit
