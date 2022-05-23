// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"
#include "MessageBuffer.hpp"

#include "TestDataTypes.hpp"
#include "VAsioProtocol.hpp"

// XXX hackish, should be hidden together wit IbMessage<T> specializations
#include "SerdesMw.hpp"
#include "SerdesMwVAsio.hpp"

#include "SerdesMwLogging.hpp"
#include "SerdesMwSync.hpp"
#include "SerdesSimData.hpp"
#include "SerdesSimRpc.hpp"
#include "SerdesSimCan.hpp"
#include "SerdesSimEthernet.hpp"
#include "SerdesSimLin.hpp"
#include "SerdesSimFlexray.hpp"
#include "SerdesMwService.hpp"

namespace ib {
namespace mw {

template<typename ApiMessageT>
struct IbMessage
	: public ApiMessageT
{
	using ApiMessageT::ApiMessageT;

	void Serialize(MessageBuffer& buffer)
	{
		buffer << static_cast<ApiMessageT&>(*this);
	}
	void Deserialize(MessageBuffer& buffer)
	{
		buffer >> static_cast<ApiMessageT&>(*this);
	}
};

class SerializedMessage
{
public: //defaulted CTors
	SerializedMessage(SerializedMessage&&) = default;
	SerializedMessage& operator=(SerializedMessage&&) = default;
	SerializedMessage(const SerializedMessage&) = delete;
	SerializedMessage& operator=(const SerializedMessage&) = delete;
public:
	SerializedMessage(std::vector<uint8_t>&& blob)
		: _buffer{std::move(blob)}
	{
		ReadNetworkHeaders();
	}
	template<typename ApiMessageT>
	auto Deserialize() -> IbMessage<ApiMessageT>
	{
		IbMessage<ApiMessageT> value{};
		value.Deserialize(_buffer);
		return value;
	}
	/* TODO besseres design fuer Serialize<Type>
	auto Serialize() -> std::vector<uint8_t>
	{
        auto buffer = _buffer.ReleaseStorage();
        if (buffer.size() > std::numeric_limits<uint32_t>::max())
            throw std::runtime_error{"SerializedMessage::Serialize: message buffer is too large"};

		//emplace the buffer size as the first element in the byte stream
		uint32_t bufferSize = static_cast<uint32_t>(buffer.size());
		memcpy(buffer.data(), &bufferSize, sizeof(uint32_t));
	}
	*/
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
	uint32_t _messageSize;
	VAsioMsgKind _messageKind;
	RegistryMessageKind _registryKind;
	// For simMsg
	EndpointAddress _endpointAddress;
	EndpointId _remoteIndex;

	MessageBuffer _buffer;
};
} //mw
} //ib
