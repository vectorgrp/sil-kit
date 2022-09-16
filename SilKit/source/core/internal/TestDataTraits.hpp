/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#pragma once

#include "traits/SilKitMsgTraits.hpp"
#include "TestDataTypes.hpp"

// Datatypes for testing versioning and renaming of SIL Kit messages.
namespace SilKit {
namespace Core {

//////////////////////////////////////////////////////////////////////
// Traits for internal testing data types for wire protocol versioning and evolution
//////////////////////////////////////////////////////////////////////
DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version1, TestMessage)
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version1::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version1::TestMessage, 1);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests::Version2, TestMessage)
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::Version2::TestMessage, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::Version2::TestMessage, 2);

DefineSilKitMsgTrait_TypeName(SilKit::Core::Tests, TestFrameEvent)
//The renamed TestMessage must have the same SerdesName as the previous struct
DefineSilKitMsgTrait_SerdesName(SilKit::Core::Tests::TestFrameEvent, "TESTMESSAGE");
DefineSilKitMsgTrait_Version(SilKit::Core::Tests::TestFrameEvent, 3);

} // namespace Core
} // namespace SilKit
