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

add_library(SilKitUtil INTERFACE)

target_include_directories(SilKitUtil
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(tests)

# Filesystem utilities required for cross platform and language portability.
# Can be removed when we finaly upgrade to C++17

add_library(SilKitFilesystem STATIC
    Filesystem.hpp
    Filesystem.cpp
)
target_include_directories(SilKitFilesystem
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(SilKitSetThreadName STATIC
    SetThreadName.hpp
    SetThreadName.cpp
)
target_include_directories(SilKitSetThreadName
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(SilKitSetThreadName
    PRIVATE SilKitInterface)

add_library(SilKitUuid STATIC
    Uuid.hpp
    Uuid.cpp
)
target_include_directories(SilKitUuid
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_library(SilKitLabelMatching STATIC
    LabelMatching.hpp
    LabelMatching.cpp
)
target_include_directories(SilKitLabelMatching
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(SilKitLabelMatching
    PRIVATE SilKitInterface)

add_silkit_test(Test_LabelMatching
    SOURCES Test_LabelMatching.cpp 
    LIBS SilKitLabelMatching
)