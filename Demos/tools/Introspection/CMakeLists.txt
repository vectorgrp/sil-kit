# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_demo(SilKitDemoObserver ObserverDemo.cpp ON)

target_sources(SilKitDemoObserver
    PRIVATE Dashboard.hpp
    PRIVATE NetworkModel.hpp
    PRIVATE Terminal.hpp
)
target_include_directories(SilKitDemoObserver PRIVATE ${SILKIT_DEMO_DIR}/communication/include)

# The dashboard draws with UTF-8 box characters.
if(MSVC)
    target_compile_options(SilKitDemoObserver PRIVATE /utf-8)
endif()
