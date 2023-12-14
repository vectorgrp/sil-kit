// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MakeAsioIoContext.hpp"

#include "impl/AsioIoContext.hpp"


namespace VSilKit {


auto MakeAsioIoContext(const AsioSocketOptions& socketOptions) -> std::unique_ptr<IIoContext>
{
    return std::make_unique<AsioIoContext>(socketOptions);
}


} // namespace VSilKit
