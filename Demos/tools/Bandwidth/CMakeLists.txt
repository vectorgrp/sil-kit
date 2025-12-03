# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_demo(SkBandwidth Bandwidth.cpp OFF)

set_target_properties(SkBandwidth PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

