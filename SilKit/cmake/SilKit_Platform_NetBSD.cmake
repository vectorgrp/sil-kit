message(STATUS "SIL Kit Platform: configuring for ${SILKIT_HOST_PLATFORM}")

# fixes asio compilation
add_definitions(-DASIO_DISABLE_STD_EXPERIMENTAL_STRING_VIEW)

#XXX current version of spdlog does not build cleanly, because a __NetBSD__ check is missing in os.h +227
# Remove this once we update to latest spdlog
add_definitions(-U__x86_64__)
