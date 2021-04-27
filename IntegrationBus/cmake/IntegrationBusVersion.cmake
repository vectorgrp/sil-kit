# IntegrationBus Versioning:
# * Major and minor release number is configured here. The patch number should not be changed here; it will be set by 
#   the Jenkins workflow to the master branch's build number for packaging (cmake -DIB_BUILD_NUMBER).
# * Major and minor release number, as well as the sprint number are encoded into Version.hpp and compiled into the library,
#   they will be accessible from public headers.
macro(configure_ib_version project_name)
    set(IB_VERSION_MAJOR 3)
    set(IB_VERSION_MINOR 3)
    set(IB_VERSION_PATCH 10)
    set(IB_BUILD_NUMBER 0 CACHE STRING "The build number")
    set(IB_SPRINT_NUMBER 56)
    set(IB_SPRINT_NAME "Sprint-${IB_SPRINT_NUMBER}")

    set(${project_name}_VERSION_MAJOR ${IB_VERSION_MAJOR})
    set(${project_name}_VERSION_MINOR ${IB_VERSION_MINOR})
    set(${project_name}_VERSION_PATCH ${IB_VERSION_PATCH})
    set(${project_name}_VERSION_TWEAK ${IB_BUILD_NUMBER})
    set(${project_name}_VERSION "${IB_VERSION_MAJOR}.${IB_VERSION_MINOR}.${IB_VERSION_PATCH}-${IB_SPRINT_NAME}")

    set(PROJECT_VERSION_MAJOR ${IB_VERSION_MAJOR})
    set(PROJECT_VERSION_MINOR ${IB_VERSION_MINOR})
    set(PROJECT_VERSION_PATCH ${IB_VERSION_PATCH})
    set(PROJECT_VERSION "${IB_VERSION_MAJOR}.${IB_VERSION_MINOR}.${IB_VERSION_PATCH}")
endmacro()
