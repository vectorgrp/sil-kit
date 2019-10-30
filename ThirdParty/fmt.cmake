function(include_fmt)
    # function() adds an additional scope
    # XXX fmt is in our public ILogger.hpp, and also in the interface link list of IbInterface
    # therefore it *must* be install-exported, otherwise the install-export of Integrationbus fails
    set(FMT_VERSION "5.3.0")
    set(FMT_INSTALL ON CACHE BOOL "Make sure the fmt-header-only interface target is exported" FORCE)
    message(STATUS "Add subdirectory ${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fmt-${FMT_VERSION}
        ${CMAKE_BINARY_DIR}/ThirdParty/fmt-${FMT_VERSION}
        EXCLUDE_FROM_ALL
    )
endfunction()

include_fmt()
