// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "traits/SilKitMsgTraits.hpp"
#include "TestDataTypes.hpp"

// Datatypes for testing versioning and renaming of SIL Kit messages.
namespace SilKit {
namespace Core {

//////////////////////////////////////////////////////////////////////
// Traits for internal testing data types for wire protocol versioning and evolution
//////////////////////////////////////////////////////////////////////
DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version1, TestMessage);
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version1::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version1::TestMessage, 1);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version2, TestMessage);
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version2::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version2::TestMessage, 2);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests, TestFrameEvent);
//The renamed TestMessage must have the same SerdesName as the previous struct
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::TestFrameEvent, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::TestFrameEvent, 3);

} // namespace Core
} // namespace SilKit
