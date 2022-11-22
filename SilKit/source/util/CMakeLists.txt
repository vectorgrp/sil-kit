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

add_library(I_SilKit_Util INTERFACE)

target_include_directories(I_SilKit_Util
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

add_subdirectory(tests)


# Filesystem utilities required for cross platform and language portability.
# Can be removed when we finaly upgrade to C++17

add_library(S_SilKit_Util_Filesystem STATIC
    Filesystem.hpp
    Filesystem.cpp
)
target_include_directories(S_SilKit_Util_Filesystem
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(S_SilKit_Util_Filesystem
    PRIVATE SilKitInterface
)


add_library(S_SilKit_Util_SetThreadName STATIC
    SetThreadName.hpp
    SetThreadName.cpp
)
target_include_directories(S_SilKit_Util_SetThreadName
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(S_SilKit_Util_SetThreadName
    PRIVATE SilKitInterface)


add_library(S_SilKit_Util_Uuid STATIC
    Uuid.hpp
    Uuid.cpp
)
target_include_directories(S_SilKit_Util_Uuid
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)


add_library(S_SilKit_Util_LabelMatching STATIC
    LabelMatching.hpp
    LabelMatching.cpp
)
target_include_directories(S_SilKit_Util_LabelMatching
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(S_SilKit_Util_LabelMatching
    PRIVATE SilKitInterface)

add_silkit_test(Test_LabelMatching
    SOURCES Test_LabelMatching.cpp 
    LIBS S_SilKit_Util_LabelMatching
)
