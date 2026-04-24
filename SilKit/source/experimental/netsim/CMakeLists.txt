# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
)
