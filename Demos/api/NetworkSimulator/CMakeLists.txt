# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_demo(SilKitDemoNetSim NetSimDemo.cpp)

target_sources(SilKitDemoNetSim
    PRIVATE src/MySimulatedNetwork.cpp
    PRIVATE src/MySimulatedNetwork.hpp
    PRIVATE src/Scheduler.cpp
    PRIVATE src/Scheduler.hpp
    PRIVATE src/Can/MySimulatedCanController.cpp
    PRIVATE src/Can/MySimulatedCanController.hpp
    PRIVATE DemoNetSim.silkit.yaml
)

target_include_directories(SilKitDemoNetSim
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src
)