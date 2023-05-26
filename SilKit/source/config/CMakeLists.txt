# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

add_library(I_SilKit_Config INTERFACE)

target_include_directories(I_SilKit_Config
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(I_SilKit_Config
    INTERFACE
        YamlInterface
        I_SilKit_Util
)

add_library(O_SilKit_Config OBJECT
    Configuration.hpp
    Configuration.cpp
    ParticipantConfiguration.hpp
    ParticipantConfiguration.cpp
    ParticipantConfigurationFromXImpl.hpp
    ParticipantConfigurationFromXImpl.cpp
    Validation.hpp
    Validation.cpp
    YamlParser.hpp
    YamlParser.cpp
    YamlConversion.hpp
    YamlConversion.cpp
    YamlSchema.hpp
    YamlSchema.cpp
    YamlValidator.hpp
    YamlValidator.cpp
)

target_link_libraries(O_SilKit_Config
    PUBLIC I_SilKit_Config

    PRIVATE SilKitInterface
    PUBLIC I_SilKit_Util
    PUBLIC yaml-cpp
    PRIVATE I_SilKit_Util_FileHelpers
)

target_include_directories(O_SilKit_Config
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

add_silkit_test(Test_MwCfgValidation
    SOURCES Test_Validation.cpp
    LIBS O_SilKit_Config
)

add_silkit_test(Test_ParticipantConfiguration
    SOURCES Test_ParticipantConfiguration.cpp
    LIBS S_SilKitImpl O_SilKit_Core_Mock_NullConnection
    CONFIGS
        ParticipantConfiguration_Minimal.json
        ParticipantConfiguration_Full.json
        ParticipantConfiguration_Full.yaml
        ParticipantConfiguration_Logging_Without_File.json
)

add_silkit_test(Test_MwCfgYamlParser
    SOURCES Test_YamlParser.cpp
    LIBS O_SilKit_Config yaml-cpp
)

add_silkit_test(Test_MwCfgYamlValidator
    SOURCES Test_YamlValidator.cpp
    LIBS O_SilKit_Config yaml-cpp
)
if(SILKIT_BUILD_TESTS)
    add_library(I_SilKit_Config_TestUtils INTERFACE)

    target_include_directories(I_SilKit_Config_TestUtils
        INTERFACE ${CMAKE_CURRENT_LIST_DIR}
    )
    target_link_libraries(I_SilKit_Config_TestUtils
        INTERFACE I_SilKit_Config
    )

endif()
