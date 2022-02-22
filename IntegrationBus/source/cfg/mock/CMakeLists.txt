# Copyright (c) Vector Informatik GmbH. All rights reserved.
add_library(IbCfgMockParticipantConfiguration STATIC
    MockParticipantConfiguration.cpp
    MockParticipantConfiguration.hpp
)

target_include_directories(IbCfgMockParticipantConfiguration
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(IbCfgMockParticipantConfiguration
    PUBLIC IbConfiguration
    PRIVATE IntegrationBus
)
