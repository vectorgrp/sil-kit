function(ib_enable_asan isOn)
    if(NOT isOn)
        return()
    endif()
    message(STATUS "VIB -- Enabling Address Sanitizer")
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        #clang, gcc
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer )
        #ensures that -lasan is linked in as first library
        add_link_options(-fsanitize=address)
    endif()
endfunction()

function(ib_enable_ubsan isOn)
    if(NOT isOn)
        return()
    endif()
    if(MSVC)
        message(STATUS "VIB -- UBSAN not supported on MSVC")
    else()
        message(STATUS "VIB -- Enabling Undefined Behavior Sanitizer")
        #clang, gcc
        add_compile_options(-fsanitize=undefined) 
        #ensures that -lasan is linked in as first library
        add_link_options(-fsanitize=undefined)
    endif()
endfunction()

function(ib_enable_warnings target)
    if(MSVC)
        # TODO
        #Note this only works on CMake 3.15, otherwise it will produce command
        # line warnings because /W3 is overriden with /W4
        #set(_flags /W4)
    else()
        set(_flags
            -pedantic
            -Wall
            -Wextra
            -Wcast-align
            -Wformat=2
            -Wshadow 
            -Wsign-promo 
            -Wstrict-overflow=5
            -Wno-unused
            -Wpacked
            )
        target_compile_options(${target} PRIVATE ${_flags})
    endif()
endfunction()

macro(ib_check_reproducible)
    if(NOT DEFINED ENV{SOURCE_DATE_EPOCH})
        message(STATUS "VIB - Reproducible build: SOURCE_DATE_EPOCH is not set!"
           " Set it to 'git log -1 --format=%ct -r origin/master'")
    else()
        message(STATUS "VIB - Reproducible build: SOURCE_DATE_EPOCH=$ENV{SOURCE_DATE_EPOCH}")
    endif()
    if(NOT "$ENV{LC_ALL}" STREQUAL  "C")
        message(STATUS "VIB - Reproducible build: LC_ALL is not set to 'C': '$ENV{LC_ALL}'")
    else()
        message(STATUS "VIB - Reproducible build: LC_ALL=C")
    endif()
    if(NOT "$ENV{TZ}" STREQUAL  "UTC")
        message(STATUS "VIB - Reproducible build: TZ is not set to 'UTC': '$ENV{TZ}'")
    else()
        message(STATUS "VIB - Reproducible build: TZ=UTC")
    endif()

endmacro()
