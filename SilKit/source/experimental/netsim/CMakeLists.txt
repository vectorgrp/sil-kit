# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Experimental_NetworkSimulatorInternals INTERFACE)

target_include_directories(I_SilKit_Experimental_NetworkSimulatorInternals 
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE SilKitInterface
)

target_link_libraries(I_SilKit_Experimental_NetworkSimulatorInternals
    INTERFACE I_SilKit_Core_Internal
)

add_library(O_SilKit_Experimental_NetworkSimulatorInternals OBJECT
    INetworkSimulatorInternal.hpp
    NetworkSimulatorInternal.cpp
    NetworkSimulatorInternal.hpp
    SimulatedNetworkInternal.cpp
    SimulatedNetworkInternal.hpp
    SimulatedNetworkRouter.cpp
    SimulatedNetworkRouter.hpp
    NetworkSimulatorDatatypesInternal.hpp
    eventproducers/CanEventProducer.cpp
    eventproducers/CanEventProducer.hpp
    eventproducers/FlexRayEventProducer.cpp
    eventproducers/FlexRayEventProducer.hpp
    eventproducers/LinEventProducer.cpp
    eventproducers/LinEventProducer.hpp
    eventproducers/EthernetEventProducer.cpp
    eventproducers/EthernetEventProducer.hpp
)

target_link_libraries(O_SilKit_Experimental_NetworkSimulatorInternals
    PUBLIC I_SilKit_Experimental_NetworkSimulatorInternals
    PRIVATE I_SilKit_Core_Internal
)
