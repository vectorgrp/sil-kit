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

# Install ftxui
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  set(FTXUI_BUILD_EXAMPLES OFF)
  set(FTXUI_BUILD_DOCS OFF)
  set(FTXUI_BUILD_DOCS OFF)
  set(FTXUI_QUIET ON)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

make_silkit_demo(InteractiveSUT "InteractiveSUT.cpp")

target_include_directories(InteractiveSUT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(InteractiveSUT
    PRIVATE SilKit::SilKit
    PRIVATE ftxui::screen
    PRIVATE ftxui::component
)

add_executable(TestingSUT "TestingSUT.cpp")
target_include_directories(TestingSUT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(TestingSUT 
    PRIVATE SilKit::SilKit
)
