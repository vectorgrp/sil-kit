# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_demo(SilKitDemoManagementClient ManagementClientDemo.cpp OFF)
target_include_directories(SilKitDemoManagementClient PRIVATE ${SILKIT_DEMO_DIR}/communication/include)
