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

make_silkit_demo(SilKitDemoEthernet  EthernetDemo.cpp)

target_sources(SilKitDemoEthernet
    PRIVATE DemoEthernet.silkit.yaml
    PRIVATE NetworkSimulatorConfig.yaml
)

if (NOT _is_standalone)
    # SIL Kit Ethernet Demo using C++-API implemented using the C-API
    # The C++-API implementation using the C-API is currently part of the
    # integration test-suite of SIL Kit. Therefore it cannot be used when
    # the SIL Kit demos are built from the release package.

    make_silkit_demo(
        SilKitDemoEthernet_Hourglass
        EthernetDemo.cpp
    )

    target_link_libraries(
        SilKitDemoEthernet_Hourglass
        PRIVATE SilKit_Hourglass
    )

    target_sources(
        SilKitDemoEthernet_Hourglass
        PRIVATE DemoEthernet.silkit.yaml
        PRIVATE NetworkSimulatorConfig.yaml
    )
endif()