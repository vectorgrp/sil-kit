// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"
#include "MessageBuffer.hpp"

//#include "TestDataTypes.hpp"
#include "VAsioProtocol.hpp"

// XXX hackish, should be hidden together wit IbMessage<T> specializations
#include "SerdesMwVAsio.hpp"

#include "SerdesSimData.hpp"

// Component specific Serialize/Deserialize functions
#include "CanSerdes.hpp"
#include "LinSerdes.hpp"
#include "EthernetSerdes.hpp"
#include "FlexraySerdes.hpp"
#include "RpcSerdes.hpp"
#include "InternalSerdes.hpp"
#include "SyncSerdes.hpp"
#include "ServiceSerdes.hpp"
#include "LoggingSerdes.hpp"

namespace ib {
namespace mw {

//helpers
template<typename MessageT>
inline constexpr auto messageKind() -> VAsioMsgKind { return VAsioMsgKind::IbMwMsg;}
template<> inline constexpr auto messageKind<ParticipantAnnouncement>() -> VAsioMsgKind { return VAsioMsgKind::IbRegistryMessage;}
template<> inline constexpr auto messageKind<ParticipantAnnouncementReply>() -> VAsioMsgKind { return VAsioMsgKind::IbRegistryMessage;}
template<> inline constexpr auto messageKind<KnownParticipants>() -> VAsioMsgKind { return VAsioMsgKind::IbRegistryMessage;}
template<> inline constexpr auto messageKind<SubscriptionAcknowledge>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAcknowledge;}
template<> inline constexpr auto messageKind<VAsioMsgSubscriber>() -> VAsioMsgKind { return VAsioMsgKind::SubscriptionAnnouncement;}

template<typename MessageT>
inline constexpr auto registryMessageKind() -> RegistryMessageKind { return RegistryMessageKind::Invalid;}
template<> inline constexpr auto registryMessageKind<ParticipantAnnouncement>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncement;}
template<> inline constexpr auto registryMessageKind<ParticipantAnnouncementReply>() -> RegistryMessageKind { return RegistryMessageKind::ParticipantAnnouncementReply;}
template<> inline constexpr auto registryMessageKind<KnownParticipants>() -> RegistryMessageKind { return RegistryMessageKind::KnownParticipants;}


template<typename... Args>
auto AdlDeserialize(Args&&... args) -> decltype(auto)
{
	return Deserialize(std::forward<Args>(args)...);
}
class SerializedMessage
{
public: //defaulted CTors
	SerializedMessage(SerializedMessage&&) = default;
	SerializedMessage& operator=(SerializedMessage&&) = default;
	SerializedMessage(const SerializedMessage&) = delete;
	SerializedMessage& operator=(const SerializedMessage&) = delete;
public: // Sending a SerializedMessage: from T to binary blob

	// Sim messages have additional parameters:
	template<typename MessageT>
	explicit SerializedMessage(const MessageT& message , EndpointAddress endpointAddress, EndpointId remoteIndex) 
	{
		 _remoteIndex = remoteIndex;
		 _endpointAddress = endpointAddress;
		 _messageKind = messageKind<MessageT>();
		 _registryKind = registryMessageKind<MessageT>();
		 WriteNetworkHeaders();
		 Serialize(_buffer, message);
	}

	template<typename MessageT>
	explicit SerializedMessage(const MessageT& message)
	{
		 _messageKind = messageKind<MessageT>();
		 _registryKind = registryMessageKind<MessageT>();
		 WriteNetworkHeaders();
		 _buffer << message;
	}
	template<typename MessageT>
	explicit SerializedMessage(ProtocolVersion version, const MessageT& message)
	{
		 _messageKind = messageKind<MessageT>();
		 _registryKind = registryMessageKind<MessageT>();
		_buffer.SetFormatVersion(version);
		 WriteNetworkHeaders();
		 _buffer << message;
	}

	auto ReleaseStorage() -> std::vector<uint8_t>
	{
        auto buffer = _buffer.ReleaseStorage();
        if (buffer.size() > std::numeric_limits<uint32_t>::max())
            throw std::runtime_error{"SerializedMessage::Serialize: message buffer is too large"};

		//emplace the buffer size as the first element in the byte stream
		uint32_t bufferSize = static_cast<uint32_t>(buffer.size());
		memcpy(buffer.data(), &bufferSize, sizeof(uint32_t));
		return buffer;
	}
public: // Receiving a SerializedMessage: from binary blob to IbMessage<T>
	explicit SerializedMessage(std::vector<uint8_t>&& blob)
		: _buffer{std::move(blob)}
	{
		ReadNetworkHeaders();
	}
	template<typename ApiMessageT>
	auto Deserialize() -> ApiMessageT
	{
		ApiMessageT value{};
		AdlDeserialize(_buffer, value);
		return value;
	}

	auto GetMessageKind() const -> VAsioMsgKind
	{
		return _messageKind;
	}
	auto GetRegistryKind() const -> RegistryMessageKind
	{
		return _registryKind;
	}
	auto GetRemoteIndex() const -> EndpointId
	{
		if(!IsMwOrSim(_messageKind))
		{
			throw std::runtime_error("SerializedMessage::GetEndpointAddress called on wrong message kind: " + std::to_string((int)_messageKind));
		}
		return _remoteIndex;
	}
	auto GetEndpointAddress() const -> EndpointAddress
	{
		if(!IsMwOrSim(_messageKind))
		{
			throw std::runtime_error("SerializedMessage::GetEndpointAddress called on wrong message kind: " + std::to_string((int)_messageKind));
		}
		return _endpointAddress;
	}
	auto SetProtocolVerison(ProtocolVersion version)
	{
		_buffer.SetFormatVersion(version);
	}
	auto PeekRegistryMessageHeader() const -> RegistryMsgHeader
	{
		return ib::mw::PeekRegistryMessageHeader(_buffer);
	}
private:
	bool IsMwOrSim(VAsioMsgKind kind) const
	{
		return kind == VAsioMsgKind::IbMwMsg
			|| kind == VAsioMsgKind::IbSimMsg
			;
	}
	void WriteNetworkHeaders()
	{
		_buffer << _messageSize; //place holder for finalization via ReleaseStorage()
		_buffer << _messageKind;
		if(_messageKind == VAsioMsgKind::IbRegistryMessage)
		{
			_buffer << _registryKind;
		}
		if(IsMwOrSim(_messageKind))
		{
			_buffer << _remoteIndex
				<< _endpointAddress;
		}
	}
	void ReadNetworkHeaders()
	{
		_messageSize = ExtractMessageSize(_buffer);
		_messageKind = ExtractMessageKind(_buffer);
		if(_messageKind == VAsioMsgKind::IbRegistryMessage)
		{
			//optional registry kind tag
			_registryKind = ExtractRegistryMessageKind(_buffer);
		}
		if(IsMwOrSim(_messageKind))
		{
			//optional remoteIndex and endpoint address
			_remoteIndex = ExtractEndpointId(_buffer);
			_endpointAddress = ExtractEndpointAddress(_buffer);
		}
	}
	// network headers, some members are optional depending on messageKind
	uint32_t _messageSize{0};
	VAsioMsgKind _messageKind{VAsioMsgKind::Invalid};
	RegistryMessageKind _registryKind{RegistryMessageKind::Invalid};
	// For simMsg
	EndpointAddress _endpointAddress{};
	EndpointId _remoteIndex{0};

	MessageBuffer _buffer;
};
} //mw
} //ib
