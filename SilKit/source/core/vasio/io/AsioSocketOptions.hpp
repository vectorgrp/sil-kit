// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


namespace VSilKit {


struct AsioSocketOptions
{
    struct
    {
        bool quickAck{false};
        bool noDelay{false};
        int receiveBufferSize{-1};
        int sendBufferSize{-1};
    } tcp;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::AsioSocketOptions;
} // namespace Core
} // namespace SilKit
