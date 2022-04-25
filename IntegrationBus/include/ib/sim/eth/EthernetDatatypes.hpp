// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "ib/IbMacros.hpp"
#include "ib/util/vector_view.hpp"
#include "ib/sim/datatypes.hpp"

// ================================================================================
//  Ethernet specific data types
// ================================================================================
namespace ib {
namespace sim {
//! The Ethernet namespace
namespace eth {

//! \brief Representing a MAC address, i.e. FF:FF:FF:FF:FF:FF
using EthernetMac = std::array<uint8_t, 6>;

/*! \brief VLAN Identifier
 * 
 * VLAN Identifier (12 Bit, range 1-4094 for a valid identifier or zero when no identifier is used).
 * The identifier 0x000 in a frame indicates that the frame has no VLAN identifier.
 * The identifier 0xFFF is reserved for special use inside switches and must not be used.
 */
using EthernetVid = uint16_t;

//! \brief Bitrate in kBit/sec
using EthernetBitrate = uint32_t;

//! \brief Tag Control Information (TCI), part of the VLAN tag.
struct EthernetTagControlInformation
{
    uint8_t pcp : 3; //!< Priority code point (0 lowest priority, 7 highest priority)
    uint8_t dei : 1; //!< Drop eligible indicator
    EthernetVid vid : 12; //!< VLAN identifier
};

//! \brief An Ethernet frame
class EthernetFrame
{
public:
    // MinGW does not allow exporting defaulted members as dllimport
    // so we user-default them inline after the EthernetFrame declaration.
    inline EthernetFrame();
    inline EthernetFrame(const EthernetFrame& other);
    inline EthernetFrame(EthernetFrame&& other);
    inline auto operator=(const EthernetFrame& other) -> EthernetFrame &;
    inline auto operator=(EthernetFrame&& other) -> EthernetFrame&;

    IntegrationBusAPI explicit EthernetFrame(const std::vector<uint8_t>& rawFrame);
    /*! \brief Constructor of an Ethernet frame
    *
    * \param rawFrame The raw Ethernet frame.
    */
    IntegrationBusAPI explicit EthernetFrame(std::vector<uint8_t>&& rawFrame);
    /*! \brief Constructor of an Ethernet frame
    *
    * \param rawFrame A pointer to the raw Ethernet frame data.
    * \param size The size of the frame data.
    */
    IntegrationBusAPI explicit EthernetFrame(const uint8_t* rawFrame, size_t size);
    
    //! \brief Get the destination MAC address from the Ethernet frame.
    IntegrationBusAPI auto GetDestinationMac() const -> EthernetMac;
    //! \brief Set the destination MAC address of the Ethernet frame.
    IntegrationBusAPI void SetDestinationMac(const EthernetMac& mac);
    //! \brief Get the source MAC address from the Ethernet frame.
    IntegrationBusAPI auto GetSourceMac() const -> EthernetMac;
    //! \brief Set the source MAC address of the Ethernet frame.
    IntegrationBusAPI void SetSourceMac(const EthernetMac& mac);

    //! \brief Get the VLAN tag from the Ethernet frame.
    IntegrationBusAPI auto GetVlanTag() const -> EthernetTagControlInformation;
    //! \brief Set the VLAN tag of the Ethernet frame.
    IntegrationBusAPI void SetVlanTag(const EthernetTagControlInformation& tci);

    //! \brief Get the ether type.
    IntegrationBusAPI auto GetEtherType() const -> uint16_t;
    //! \brief Set the ether type.
    IntegrationBusAPI void SetEtherType(uint16_t etherType);

    //! \brief Get the size of the Ethernet frame.
    IntegrationBusAPI auto GetFrameSize() const -> size_t;
    //! \brief Get the size of the Ethernet frame's header.
    IntegrationBusAPI auto GetHeaderSize() const -> size_t;
    //! \brief Get the size of the Ethernet frame's payload.
    IntegrationBusAPI auto GetPayloadSize() const -> size_t;

    //! \brief Get the payload of the Ethernet frame.
    IntegrationBusAPI auto GetPayload() -> util::vector_view<uint8_t>;
    IntegrationBusAPI auto GetPayload() const -> util::vector_view<const uint8_t>;
    //! \brief Set the payload of the Ethernet frame.
    IntegrationBusAPI void SetPayload(const std::vector<uint8_t>& payload);
    IntegrationBusAPI void SetPayload(const uint8_t* payload, size_t size);

    //! \brief Get the raw Ethernet frame.
    IntegrationBusAPI auto RawFrame() const -> const std::vector<uint8_t>&;
    //! \brief Set the raw Ethernet frame.
    IntegrationBusAPI void SetRawFrame(const std::vector<uint8_t>&);

private:
    std::vector<uint8_t> _rawFrame;
};
////////////////////////////////////////////////////////////////////////////////
// EthernetFrame Inline definitions
////////////////////////////////////////////////////////////////////////////////

// Explicitly defaulted, must be split from declaration because of MinGW
EthernetFrame::EthernetFrame() = default;
EthernetFrame::EthernetFrame(const EthernetFrame& other) = default;
EthernetFrame::EthernetFrame(EthernetFrame&& other) = default;
auto EthernetFrame::operator=(const EthernetFrame& other) -> EthernetFrame & = default;
auto EthernetFrame::operator=(EthernetFrame&& other) -> EthernetFrame& = default;

//! \brief An Ethernet transmit id
using EthernetTxId = uint32_t;

//! \brief An Ethernet frame including the raw frame, Transmit ID and timestamp
struct EthernetFrameEvent
{
    EthernetTxId transmitId; //!< Set by the EthController, used for acknowledgments
    std::chrono::nanoseconds timestamp; //!< Reception time
    EthernetFrame ethFrame; //!< The Ethernet frame
};

//! \brief Acknowledgment status for an EthernetTransmitRequest
enum class EthernetTransmitStatus : uint8_t
{
    /*! The message was successfully transmitted on the Ethernet link.
    */
    Transmitted = 0,

    /*! The transmit request was rejected, because the Ethernet controller is not active.
    */
    ControllerInactive = 1,

    /*! The transmit request was rejected, because the Ethernet link is down.
    */
    LinkDown = 2,

    /*! The transmit request was dropped, because the transmit queue is full.
    */
    Dropped = 3,

    /*! (currently not in use)
     *
     * The transmit request was rejected, because there is already another request with the same transmitId.
    */
    DuplicatedTransmitId = 4,

    /*! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, wrong order of VLAN tags).
    */
    InvalidFrameFormat = 5
};

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetFrameTransmitEvent
{
    EthernetTxId transmitId;   //!< Identifies the EthernetTransmitRequest, to which this EthernetFrameTransmitEvent refers to.
    EthernetMac sourceMac; //!< The source MAC address encoded as integral data type
    std::chrono::nanoseconds timestamp; //!< Timestamp of the Ethernet acknowledge.
    EthernetTransmitStatus status; //!< Status of the EthernetTransmitRequest.
};

//! \brief State of the Ethernet controller
enum class EthernetState : uint8_t
{
    Inactive = 0, //!< The Ethernet controller is switched off (default after reset).
    LinkDown = 1, //!< The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
    LinkUp = 2, //!< The Ethernet controller is active and the link to another Ethernet controller is established.
};

//! \brief A state change event of the Ethernet controller
struct EthernetStateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    EthernetState state; //!< State of the Ethernet controller.
};

//! \brief A bitrate change event of the Ethernet controller
struct EthernetBitrateChangeEvent
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the state change.
    EthernetBitrate bitrate; //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
};

//! \brief Publishes status of the simulated Ethernet controller
struct EthernetStatus
{
    std::chrono::nanoseconds timestamp; //!< Timestamp of the status change.
    EthernetState state; //!< State of the Ethernet controller.
    EthernetBitrate bitrate; //!< Bit rate in kBit/sec of the link when in state LinkUp, otherwise zero.
};

//! \brief Mode for switching an Ethernet Controller on or off
enum class EthernetMode : uint8_t
{
    Inactive = 0, //!< The controller is inactive (default after reset).
    Active = 1, //!< The controller is active.
};

//! \brief Set the Mode of the Ethernet Controller.
struct EthernetSetMode
{
    EthernetMode mode; //!< EthernetMode that the Ethernet controller should reach.
};

} // namespace eth
} // namespace sim
} // namespace ib
