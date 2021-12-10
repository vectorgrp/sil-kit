// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib {
namespace util {
namespace serdes {

inline namespace V1 {

/*! \brief The current version of the serializer and deserializer.
 *  \returns the current version of the serializer and deserializer. */
constexpr uint16_t SerDesVersion() { return 1; }

/*! \brief Class representing the header information for data member update. */
class DataMemberUpdateHeader
{
public:
  /*! \brief Instantiates a new header for a data member update. */
  DataMemberUpdateHeader() = default;

  /*! \brief Returns the used serialization version of the data member update.
   *  \returns the version of the data member update. */
  uint16_t GetVersion() const { return mVersion; }

private:
  uint16_t mVersion{SerDesVersion()}; ///< The used serialization version for the data member update
};

/*! \brief Class representing the header information for event member update. */
class EventMemberUpdateHeader
{
public:
  /*! \brief Instantiates a new header for a event member update. */
  EventMemberUpdateHeader() = default;

  /*! \brief Returns the used serialization version of the event member update.
   *  \returns the version of the event member update. */
  uint16_t GetVersion() const { return mVersion; }

private:
  uint16_t mVersion{ SerDesVersion() }; ///< The used serialization version for the event member update
};

/*! \brief Enumeration describing the message type of a method call. */
enum class MethodCallMessageType : uint8_t
{
  Request,         ///< A method request which expects a response.
  RequestNoReturn, ///< A method request which expects no response (fire-and-forget).
  Response         ///< A method response to a method request.
};

/*! \brief Enumeration describing the return code for a method call. */
enum class MethodCallReturnCode : uint8_t
{
  NoError,              ///< No error occurred and the method call was successful.
  NoCallbackRegistered, ///< No callback was registered on the callee side.
};

/*! \brief Class representing the method call specific header information. */
class MethodCallHeader
{
public:
  MethodCallHeader() = default;

  /*! \brief Instantiates a new header for a method call.
   *  \param requestId The request ID of the method call.
   *  \param messageType The message type of the method call.
   *  \param returnCode The return code of the method call. */
  MethodCallHeader(int64_t requestId, MethodCallMessageType messageType, MethodCallReturnCode returnCode) :
    mRequestId(requestId),
    mMessageType(messageType),
    mReturnCode(returnCode)
  {
  }

  /*! \brief Returns the used serialization version of the method call.
   *  \returns the version of the method call. */
  uint16_t GetVersion() const { return mVersion; }

  /*! \brief Returns the request ID of the method call.
   *  \returns the request ID of the method call. */
  int64_t GetRequestId() const { return mRequestId; }

  /*! \brief Returns the message type of the method call.
   *  \returns the message type of the method call. */
  MethodCallMessageType GetMessageType() const { return mMessageType; }

  /*! \brief Returns the return code of the method call.
   *  \returns the return code of the method call. */
  MethodCallReturnCode GetReturnCode() const { return mReturnCode; }

private:
  // When the header structure is changed create a new version of the header in a new Vx namespace.

  uint16_t mVersion{SerDesVersion()}; ///< The used serialization version for the method call
  int64_t mRequestId{0};              ///< The request ID for the method call
  MethodCallMessageType mMessageType{MethodCallMessageType::Request}; ///< The message type of the method call
  MethodCallReturnCode mReturnCode{MethodCallReturnCode::NoError};    ///< The return code of the method call
};

} // namespace V1
} // namespace serdes
} // namespace util
} // namespace ib
