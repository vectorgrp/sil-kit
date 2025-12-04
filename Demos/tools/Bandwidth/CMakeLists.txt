# SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_demo(SilKitBandwidth Bandwidth.cpp OFF)

set_target_properties(SilKitBandwidth PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

