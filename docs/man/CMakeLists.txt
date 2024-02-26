#
# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT
#
# Install man pages for Linux systems
if (UNIX)
    include(GNUInstallDirs)
    install(
        FILES
            sil-kit-registry.1
            sil-kit-system-controller.1
            sil-kit-monitor.1
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/
        CONFIGURATIONS Release Debug RelWithDebInfo
        COMPONENT utils
    )
endif()
