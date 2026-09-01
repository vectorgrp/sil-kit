// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "util/GenerateVersion.hpp"

namespace {

using namespace SilKit::VersionGen;

// A faithful excerpt of SilKit/cmake/SilKitVersion.cmake, including the
// surrounding lines that must survive a patch untouched.
const char* const kCMakeVersion = R"(# SPDX-License-Identifier: MIT

macro(configure_silkit_version project_name)
    set(SILKIT_VERSION_MAJOR 5)
    set(SILKIT_VERSION_MINOR 0)
    set(SILKIT_VERSION_PATCH 8)
    set(SILKIT_BUILD_NUMBER 0 CACHE STRING "The build number")
    set(SILKIT_VERSION_SUFFIX "")

    set(VERSION_STRING "${SILKIT_VERSION_MAJOR}.${SILKIT_VERSION_MINOR}.${SILKIT_VERSION_PATCH}")
endmacro()
)";

// The header as committed before the generated-file banner and the #ifndef
// build-number fallback were introduced. Kept verbatim: LooksLikeGeneratedHeader
// must still accept an older generated header.
const char* const kLegacyHeader = R"(#pragma once

#define SILKIT_GIT_HASH "23932429ac68eecdb8ca7698f35783ee5d89a04b"
#define SILKIT_VERSION_MAJOR 5
#define SILKIT_VERSION_MINOR 0
#define SILKIT_VERSION_PATCH 6
#define SILKIT_BUILD_NUMBER 42
#define SILKIT_VERSION_STRING "5.0.6"
#define SILKIT_VERSION_SUFFIX ""
)";

// silkit/capi/Version.h, the hand-written public API header that must never be
// mistaken for a generated one.
const char* const kPublicApiHeader = R"(#pragma once
#include "silkit/capi/SilKitMacros.h"

SILKIT_BEGIN_DECLS
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Version_Major(uint32_t* outVersionMajor);
SILKIT_END_DECLS
)";

const char* const kOverviewRst = R"(Changelog
=========

.. toctree::
   :maxdepth: 1
   :glob:

   versions/latest.md
   versions/5.0.7.md
   versions/4.rst
)";

TEST(Test_Util_GenerateVersion, ParseVersionFromCmake)
{
    const auto version = ParseVersionFromCMake(kCMakeVersion);
    ASSERT_TRUE(version.IsValid());
    EXPECT_EQ(version.major, 5);
    EXPECT_EQ(version.minor, 0);
    EXPECT_EQ(version.patch, 8);
    EXPECT_EQ(version.suffix, "");
    EXPECT_EQ(version.ToString(), "5.0.8");
}

TEST(Test_Util_GenerateVersion, ParseVersionFromCmakeIgnoresBuildNumber)
{
    // SILKIT_BUILD_NUMBER carries a CACHE clause and must not be mistaken for a
    // version component.
    const auto version = ParseVersionFromCMake(kCMakeVersion);
    EXPECT_EQ(version.ToShortString(), "5.0.8");
}

TEST(Test_Util_GenerateVersion, ParseVersionFromCmakeWithoutVersionIsInvalid)
{
    EXPECT_FALSE(ParseVersionFromCMake("# nothing here\n").IsValid());
}

TEST(Test_Util_GenerateVersion, ParseVersionFromHeader)
{
    const auto version = ParseVersionFromHeader(kLegacyHeader);
    ASSERT_TRUE(version.IsValid());
    EXPECT_EQ(version.ToString(), "5.0.6");
    EXPECT_EQ(ParseGitHashFromHeader(kLegacyHeader), "23932429ac68eecdb8ca7698f35783ee5d89a04b");
}

TEST(Test_Util_GenerateVersion, GeneratedHeaderIsRecognized)
{
    EXPECT_TRUE(LooksLikeGeneratedHeader(kLegacyHeader));

    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 9;
    EXPECT_TRUE(LooksLikeGeneratedHeader(RenderHeader("abc", version)));
}

TEST(Test_Util_GenerateVersion, PublicApiHeaderIsNotMistakenForAGeneratedOne)
{
    // This is the guard that keeps a mistyped output path from clobbering
    // silkit/capi/Version.h.
    EXPECT_FALSE(LooksLikeGeneratedHeader(kPublicApiHeader));
    EXPECT_FALSE(LooksLikeGeneratedHeader(""));
}

TEST(Test_Util_GenerateVersion, PatchCmakeVersionOnlyTouchesTheVersionLines)
{
    Version version;
    version.major = 6;
    version.minor = 1;
    version.patch = 2;
    version.suffix = "rc1";

    std::string content{kCMakeVersion};
    std::string error;
    ASSERT_TRUE(PatchCMakeVersion(content, version, error)) << error;

    EXPECT_NE(content.find("set(SILKIT_VERSION_MAJOR 6)"), std::string::npos);
    EXPECT_NE(content.find("set(SILKIT_VERSION_MINOR 1)"), std::string::npos);
    EXPECT_NE(content.find("set(SILKIT_VERSION_PATCH 2)"), std::string::npos);
    EXPECT_NE(content.find("set(SILKIT_VERSION_SUFFIX \"rc1\")"), std::string::npos);

    // Everything else survives verbatim.
    EXPECT_NE(content.find("set(SILKIT_BUILD_NUMBER 0 CACHE STRING \"The build number\")"), std::string::npos);
    EXPECT_NE(content.find("macro(configure_silkit_version project_name)"), std::string::npos);
    EXPECT_NE(content.find("${SILKIT_VERSION_MAJOR}.${SILKIT_VERSION_MINOR}"), std::string::npos);

    EXPECT_EQ(ParseVersionFromCMake(content).ToString(), "6.1.2-rc1");
}

TEST(Test_Util_GenerateVersion, PatchCmakeVersionReportsAMissingSetter)
{
    Version version;
    version.major = 1;
    version.minor = 2;
    version.patch = 3;

    std::string content = "set(SILKIT_VERSION_MAJOR 5)\n";
    std::string error;
    EXPECT_FALSE(PatchCMakeVersion(content, version, error));
    EXPECT_FALSE(error.empty());
}

TEST(Test_Util_GenerateVersion, RenderedHeaderIsAsciiLfAndRoundTrips)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 9;
    version.suffix = "rc2";

    const auto header = RenderHeader("deadbeef", version);

    for (const char character : header)
    {
        EXPECT_GE(static_cast<unsigned char>(character), 0x09u) << "non-ASCII byte in the generated header";
        EXPECT_LT(static_cast<unsigned char>(character), 0x80u) << "non-ASCII byte in the generated header";
    }
    EXPECT_EQ(header.find('\r'), std::string::npos) << "the generated header must use LF line endings";

    EXPECT_NE(header.find("#define SILKIT_VERSION_STRING \"5.0.9-rc2\""), std::string::npos);
    EXPECT_NE(header.find("DO NOT EDIT"), std::string::npos);

    EXPECT_EQ(ParseVersionFromHeader(header).ToString(), "5.0.9-rc2");
    EXPECT_EQ(ParseGitHashFromHeader(header), "deadbeef");
}

TEST(Test_Util_GenerateVersion, BuildNumberIsOnlyAFallbackInTheHeader)
{
    // The build number is a property of a build, supplied by CMake as
    // -DSILKIT_BUILD_NUMBER=N. The header must not pin it, or the definition
    // could never take effect.
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    const auto header = RenderHeader("abc", version);

    EXPECT_NE(header.find("#ifndef SILKIT_BUILD_NUMBER"), std::string::npos);
    EXPECT_NE(header.find("#define SILKIT_BUILD_NUMBER 0"), std::string::npos);
    EXPECT_NE(header.find("#endif"), std::string::npos);

    // The guarded define is the only one, and it comes after the #ifndef.
    EXPECT_LT(header.find("#ifndef SILKIT_BUILD_NUMBER"), header.find("#define SILKIT_BUILD_NUMBER"));
    EXPECT_EQ(header.find("#define SILKIT_BUILD_NUMBER", header.find("#define SILKIT_BUILD_NUMBER") + 1),
              std::string::npos);
}

TEST(Test_Util_GenerateVersion, RenderedHeaderIsStable)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;
    EXPECT_EQ(RenderHeader("abc", version), RenderHeader("abc", version));
}

TEST(Test_Util_GenerateVersion, FinalizeChangelogHeadingSetsVersionAndDate)
{
    const std::string entry = "# [5.0.8] - UNRELEASED\n\n## Added\n\n- Something\n";

    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    const auto finalized = FinalizeChangelogHeading(entry, version, "2026-09-01");
    EXPECT_EQ(finalized, "# [5.0.8] - 2026-09-01\n\n## Added\n\n- Something\n");
}

TEST(Test_Util_GenerateVersion, FinalizeChangelogHeadingCorrectsAStaleVersion)
{
    // After a rebase latest.md may carry the wrong number; the archived file
    // must name the version actually being released.
    const std::string entry = "# [5.0.6] - UNRELEASED\n\n## Fixed\n";

    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    EXPECT_EQ(FinalizeChangelogHeading(entry, version, "2026-09-01"), "# [5.0.8] - 2026-09-01\n\n## Fixed\n");
}

TEST(Test_Util_GenerateVersion, FinalizeChangelogHeadingLeavesAHeadinglessEntryAlone)
{
    const std::string entry = "no heading here\n";
    Version version;
    version.major = 1;
    version.minor = 0;
    version.patch = 0;
    EXPECT_EQ(FinalizeChangelogHeading(entry, version, "2026-09-01"), entry);
}

TEST(Test_Util_GenerateVersion, ChangelogStubMatchesTheEstablishedFormat)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 9;
    EXPECT_EQ(RenderChangelogStub(version), "# [5.0.9] - UNRELEASED\n\n> This changelog entry is still empty.\n");
}

TEST(Test_Util_GenerateVersion, InsertToctreeEntryAfterLatest)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    std::string content{kOverviewRst};
    std::string error;
    ASSERT_TRUE(InsertChangelogToctreeEntry(content, version, error)) << error;

    const size_t latest = content.find("versions/latest.md");
    const size_t inserted = content.find("versions/5.0.8.md");
    const size_t next = content.find("versions/5.0.7.md");
    ASSERT_NE(inserted, std::string::npos);
    EXPECT_LT(latest, inserted);
    EXPECT_LT(inserted, next);

    // The toctree indentation is preserved.
    EXPECT_NE(content.find("\n   versions/5.0.8.md\n"), std::string::npos);
}

TEST(Test_Util_GenerateVersion, InsertToctreeEntryRejectsADuplicate)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 7;

    std::string content{kOverviewRst};
    std::string error;
    EXPECT_FALSE(InsertChangelogToctreeEntry(content, version, error));
    EXPECT_FALSE(error.empty());
    EXPECT_EQ(content, kOverviewRst);
}

TEST(Test_Util_GenerateVersion, InsertToctreeEntryReportsAMissingAnchor)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    std::string content = "Changelog\n=========\n";
    std::string error;
    EXPECT_FALSE(InsertChangelogToctreeEntry(content, version, error));
    EXPECT_FALSE(error.empty());
}

TEST(Test_Util_GenerateVersion, InsertToctreeEntryKeepsCrlfLineEndings)
{
    Version version;
    version.major = 5;
    version.minor = 0;
    version.patch = 8;

    std::string content = ".. toctree::\r\n\r\n   versions/latest.md\r\n   versions/4.rst\r\n";
    std::string error;
    ASSERT_TRUE(InsertChangelogToctreeEntry(content, version, error)) << error;
    EXPECT_NE(content.find("\r\n   versions/5.0.8.md\r\n"), std::string::npos);
}

TEST(Test_Util_GenerateVersion, DetectLineEndings)
{
    EXPECT_TRUE(UsesCrlf("a\r\nb\r\n"));
    EXPECT_FALSE(UsesCrlf("a\nb\n"));
    EXPECT_FALSE(UsesCrlf(""));
}

TEST(Test_Util_GenerateVersion, ApplyLineEndings)
{
    EXPECT_EQ(WithLineEndings("a\nb\n", true), "a\r\nb\r\n");
    EXPECT_EQ(WithLineEndings("a\r\nb\r\n", false), "a\nb\n");

    // Idempotent, so applying the working tree's endings to content spliced out
    // of an existing file cannot double them up.
    EXPECT_EQ(WithLineEndings("a\r\nb\r\n", true), "a\r\nb\r\n");
    EXPECT_EQ(WithLineEndings("a\nb\n", false), "a\nb\n");

    // A lone CR is not a line ending and is left alone.
    EXPECT_EQ(WithLineEndings("a\rb\n", true), "a\rb\r\n");
}

TEST(Test_Util_GenerateVersion, TodayIsAnIsoDate)
{
    const auto today = TodayIsoDate();
    ASSERT_EQ(today.size(), 10u);
    EXPECT_EQ(today[4], '-');
    EXPECT_EQ(today[7], '-');
}

} // anonymous namespace
