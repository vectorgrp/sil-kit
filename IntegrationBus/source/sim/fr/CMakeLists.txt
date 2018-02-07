add_library(IbSimFlexRay STATIC
    FrController.cpp
    FrController.hpp
    FrControllerProxy.cpp
    FrControllerProxy.hpp
    FrDatatypeUtils.cpp
    FrDatatypeUtils.hpp
)

target_include_directories(IbSimFlexRay
    PUBLIC .
)

target_link_libraries(IbSimFlexRay
    PRIVATE IbInterface
)

add_mvsc_ib_include_filter()
