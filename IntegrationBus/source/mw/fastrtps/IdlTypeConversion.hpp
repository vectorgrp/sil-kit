#include "IdlTypeConversionMw.hpp"
#include "IdlTypeConversionSync.hpp"
#include "IdlTypeConversionLogging.hpp"
#include "IdlTypeConversionCan.hpp"
#include "IdlTypeConversionEth.hpp"
#include "IdlTypeConversionFlexray.hpp"
#include "IdlTypeConversionLin.hpp"
#include "IdlTypeConversionIo.hpp"
#include "IdlTypeConversionGenericMsg.hpp"


template <class IdlMessageType>
using to_ib_message_t = decltype(from_idl(std::move(std::declval<IdlMessageType>())));

template <class IbMessageType>
using to_idl_message_t = decltype(to_idl(std::declval<IbMessageType>()));
