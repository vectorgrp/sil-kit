# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

include(CheckCXXSourceCompiles)
macro(silkit_check_for_asio)
    check_cxx_source_compiles("
#include <asio.hpp>

int main()
{
  asio::io_context ctx;
  ctx.run();
  return 0;
}
" HAVE_ASIO_LIB)


if(NOT HAVE_ASIO_LIB)
    message(FATAL_ERROR "No libasio headers found")
else()
    message(STATUS "Libasio headers found!")
endif()

endmacro()
