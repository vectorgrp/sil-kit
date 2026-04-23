# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# The Participant's connection interfaces are collected in this folder
# for easy consumption from external projects

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_MessageBuffer.cpp LIBS I_SilKit)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_InternalSerdes.cpp LIBS I_SilKit)
