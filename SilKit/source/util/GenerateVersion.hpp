// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Pure text transformations behind the sil-kit-generate-version tool.
//
// Everything here operates on in-memory strings so it can be unit tested
// without touching the source tree. All rendered output uses LF line endings
// and stays within ASCII.

#pragma once

#include <string>

namespace SilKit {
namespace VersionGen {

struct Version
{
    int major{-1};
    int minor{-1};
    int patch{-1};

    bool IsValid() const
    {
        return major >= 0 && minor >= 0 && patch >= 0;
    }

    // "5.0.8". There is no suffix here: a pre-release suffix describes a build
    // and is supplied by CMake, not stored in the source tree.
    std::string ToString() const;

    bool operator==(const Version& other) const
    {
        return major == other.major && minor == other.minor && patch == other.patch;
    }

    bool operator!=(const Version& other) const
    {
        return !(*this == other);
    }
};

// Parses SILKIT_VERSION_{MAJOR,MINOR,PATCH} out of the contents of
// SilKit/cmake/SilKitVersion.cmake. Returns an invalid Version if a number is
// missing.
Version ParseVersionFromCMake(const std::string& content);

// Parses the same values out of a generated SilKitVersionMacros.h.
Version ParseVersionFromHeader(const std::string& content);

// Extracts SILKIT_GIT_HASH from a generated header, or "" if absent.
std::string ParseGitHashFromHeader(const std::string& content);

// True if the content is recognizably a generated version-macros header. Used
// to refuse overwriting hand-written headers such as silkit/capi/Version.h.
bool LooksLikeGeneratedHeader(const std::string& content);

// Rewrites the set(SILKIT_VERSION_*) lines in SilKitVersion.cmake, leaving
// every other byte untouched. Returns false and fills 'error' if one of the
// expected lines is missing.
bool PatchCMakeVersion(std::string& content, const Version& version, std::string& error);

// Renders the full SilKitVersionMacros.h. 'gitHash' is written as the fallback
// value. The build number and pre-release suffix are not parameters at all:
// CMake supplies those at build time.
std::string RenderHeader(const std::string& gitHash, const Version& version);

// Rewrites the leading '# [x.y.z] - UNRELEASED' heading of a changelog entry to
// the given version and release date. The body is left untouched. If no such
// heading is found the content is returned unchanged.
std::string FinalizeChangelogHeading(const std::string& content, const Version& version, const std::string& date);

// The placeholder written to latest.md right after a bump.
std::string RenderChangelogStub(const Version& version);

// Inserts a 'versions/<version>.md' line into the toctree of
// docs/changelog/overview.rst, directly after 'versions/latest.md'. Returns
// false and fills 'error' if the anchor is missing or the entry already exists.
bool InsertChangelogToctreeEntry(std::string& content, const Version& version, std::string& error);

// Today's date as YYYY-MM-DD in local time, or "" if the clock cannot be
// formatted.
std::string TodayIsoDate();

// True if the content uses CRLF line endings. The working tree may be either,
// depending on core.autocrlf, so generated files follow what is already there.
bool UsesCrlf(const std::string& content);

// Rewrites CRLF/LF line endings to the requested kind. A lone CR is left alone.
std::string WithLineEndings(const std::string& content, bool crlf);

} // namespace VersionGen
} // namespace SilKit
