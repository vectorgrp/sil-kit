// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "GenerateVersion.hpp"

#include <cstdlib>
#include <ctime>
#include <regex>

namespace SilKit {
namespace VersionGen {

namespace {

// The literal contents of SilKitVersionMacros.h. Keep this ASCII only: the file
// is consumed by the MSVC resource compiler as well as by C and C++.
constexpr auto kHeaderTemplate = R"(// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// GENERATED FILE - DO NOT EDIT BY HAND.
// Written by the sil-kit-generate-version tool from the version numbers in
// SilKit/cmake/SilKitVersion.cmake. See docs/development/release.md.

#pragma once

#define SILKIT_VERSION_MAJOR @MAJOR@
#define SILKIT_VERSION_MINOR @MINOR@
#define SILKIT_VERSION_PATCH @PATCH@
#define SILKIT_VERSION_STRING "@VERSION_STRING@"
#define SILKIT_VERSION_SUFFIX "@SUFFIX@"

// The build number and the git hash describe a build, not the source tree, so
// CMake supplies them: -DSILKIT_BUILD_NUMBER=N and -DSILKIT_BUILD_GIT_HASH=<hash>.
// The values below are the fallbacks. The hash is the commit that was HEAD when
// this file was generated, i.e. the parent of the version bump.
#ifndef SILKIT_BUILD_NUMBER
#define SILKIT_BUILD_NUMBER 0
#endif

#ifndef SILKIT_GIT_HASH
#define SILKIT_GIT_HASH "@GIT_HASH@"
#endif
)";

void ReplaceAll(std::string& s, const std::string& from, const std::string& to)
{
    for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
    {
        s.replace(pos, from.size(), to);
    }
}

void SubstituteVar(std::string& s, const std::string& name, const std::string& value)
{
    ReplaceAll(s, "@" + name + "@", value);
}

// set(SILKIT_VERSION_MAJOR 5), tolerating any whitespace CMake would accept.
std::regex CMakeIntSetter(const std::string& variable)
{
    return std::regex{"(set[ \t]*\\([ \t]*" + variable + "[ \t]+)([0-9]+)"};
}

// set(SILKIT_VERSION_SUFFIX "rc1")
std::regex CMakeStringSetter(const std::string& variable)
{
    return std::regex{"(set[ \t]*\\([ \t]*" + variable + "[ \t]+\")([^\"]*)(\")"};
}

int MatchCMakeInt(const std::string& content, const std::regex& re)
{
    std::smatch match;
    if (!std::regex_search(content, match, re))
    {
        return -1;
    }
    return std::atoi(match[2].str().c_str());
}

// #define SILKIT_VERSION_MAJOR 5
int DefineInt(const std::string& content, const std::string& macroName)
{
    std::smatch match;
    const std::regex re{"#[ \t]*define[ \t]+" + macroName + "[ \t]+(-?[0-9]+)"};
    if (!std::regex_search(content, match, re))
    {
        return -1;
    }
    return std::atoi(match[1].str().c_str());
}

// #define SILKIT_VERSION_SUFFIX "rc1"
bool DefineString(const std::string& content, const std::string& macroName, std::string& out)
{
    std::smatch match;
    const std::regex re{"#[ \t]*define[ \t]+" + macroName + "[ \t]+\"([^\"]*)\""};
    if (!std::regex_search(content, match, re))
    {
        return false;
    }
    out = match[1].str();
    return true;
}

// Replaces the text captured by the given group of the first match. Done by
// position rather than with a regex_replace format string, because "$1" glued
// in front of a digit would read as a reference to group 1x.
bool ReplaceCapture(std::string& content, const std::regex& re, int group, const std::string& value)
{
    std::smatch match;
    if (!std::regex_search(content, match, re))
    {
        return false;
    }
    content.replace(static_cast<size_t>(match.position(group)), static_cast<size_t>(match.length(group)), value);
    return true;
}

} // namespace

std::string Version::ToShortString() const
{
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

std::string Version::ToString() const
{
    return ToShortString() + (suffix.empty() ? "" : "-" + suffix);
}

Version ParseVersionFromCMake(const std::string& content)
{
    Version version;
    version.major = MatchCMakeInt(content, CMakeIntSetter("SILKIT_VERSION_MAJOR"));
    version.minor = MatchCMakeInt(content, CMakeIntSetter("SILKIT_VERSION_MINOR"));
    version.patch = MatchCMakeInt(content, CMakeIntSetter("SILKIT_VERSION_PATCH"));

    std::smatch match;
    if (std::regex_search(content, match, CMakeStringSetter("SILKIT_VERSION_SUFFIX")))
    {
        version.suffix = match[2].str();
    }
    return version;
}

Version ParseVersionFromHeader(const std::string& content)
{
    Version version;
    version.major = DefineInt(content, "SILKIT_VERSION_MAJOR");
    version.minor = DefineInt(content, "SILKIT_VERSION_MINOR");
    version.patch = DefineInt(content, "SILKIT_VERSION_PATCH");
    DefineString(content, "SILKIT_VERSION_SUFFIX", version.suffix);
    return version;
}

std::string ParseGitHashFromHeader(const std::string& content)
{
    std::string gitHash;
    DefineString(content, "SILKIT_GIT_HASH", gitHash);
    return gitHash;
}

bool LooksLikeGeneratedHeader(const std::string& content)
{
    std::string gitHash;
    return DefineInt(content, "SILKIT_VERSION_MAJOR") >= 0 && DefineString(content, "SILKIT_GIT_HASH", gitHash);
}

bool PatchCMakeVersion(std::string& content, const Version& version, std::string& error)
{
    struct Setter
    {
        const char* variable;
        int value;
    };
    const Setter setters[] = {
        {"SILKIT_VERSION_MAJOR", version.major},
        {"SILKIT_VERSION_MINOR", version.minor},
        {"SILKIT_VERSION_PATCH", version.patch},
    };

    std::string result = content;
    for (const auto& setter : setters)
    {
        if (!ReplaceCapture(result, CMakeIntSetter(setter.variable), 2, std::to_string(setter.value)))
        {
            error = std::string{"no 'set("} + setter.variable + " <number>)' found";
            return false;
        }
    }

    if (!ReplaceCapture(result, CMakeStringSetter("SILKIT_VERSION_SUFFIX"), 2, version.suffix))
    {
        error = "no 'set(SILKIT_VERSION_SUFFIX \"...\")' found";
        return false;
    }

    content = result;
    return true;
}

std::string RenderHeader(const std::string& gitHash, const Version& version)
{
    std::string result{kHeaderTemplate};
    SubstituteVar(result, "GIT_HASH", gitHash);
    SubstituteVar(result, "MAJOR", std::to_string(version.major));
    SubstituteVar(result, "MINOR", std::to_string(version.minor));
    SubstituteVar(result, "PATCH", std::to_string(version.patch));
    SubstituteVar(result, "VERSION_STRING", version.ToString());
    SubstituteVar(result, "SUFFIX", version.suffix);
    return result;
}

std::string FinalizeChangelogHeading(const std::string& content, const Version& version, const std::string& date)
{
    // Only the first '# [x.y.z] - <anything>' heading is rewritten; a body that
    // happens to quote another one is left alone.
    const std::regex heading{"[ \t]*#[ \t]*\\[[^\\]\r\n]*\\][ \t]*-[ \t]*[^\r\n]*"};
    std::smatch match;
    if (!std::regex_search(content, match, heading))
    {
        return content;
    }

    std::string result = content;
    result.replace(static_cast<size_t>(match.position(0)), static_cast<size_t>(match.length(0)),
                   "# [" + version.ToString() + "] - " + date);
    return result;
}

std::string RenderChangelogStub(const Version& version)
{
    return "# [" + version.ToString() + "] - UNRELEASED\n\n> This changelog entry is still empty.\n";
}

bool InsertChangelogToctreeEntry(std::string& content, const Version& version, std::string& error)
{
    const std::string entry = "versions/" + version.ToShortString() + ".md";
    if (content.find(entry) != std::string::npos)
    {
        error = "'" + entry + "' is already listed";
        return false;
    }

    const std::string anchor = "versions/latest.md";
    const size_t anchorPos = content.find(anchor);
    if (anchorPos == std::string::npos)
    {
        error = "no '" + anchor + "' toctree entry to insert after";
        return false;
    }

    // Reuse the anchor line's indentation and line ending verbatim.
    const size_t lineStart = content.rfind('\n', anchorPos);
    const std::string indent = content.substr(lineStart + 1, anchorPos - (lineStart + 1));

    const size_t lineEnd = content.find('\n', anchorPos);
    if (lineEnd == std::string::npos)
    {
        content += "\n" + indent + entry;
        return true;
    }
    const bool crlf = lineEnd > 0 && content[lineEnd - 1] == '\r';
    const std::string eol = crlf ? "\r\n" : "\n";

    content.insert(lineEnd + 1, indent + entry + eol);
    return true;
}

std::string TodayIsoDate()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    // strftime rather than snprintf: GCC cannot prove that tm_mon and tm_mday
    // are two digits wide and warns about a possibly truncated %02d.
    char buffer[16] = {};
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local) == 0)
    {
        return "";
    }
    return buffer;
}

bool UsesCrlf(const std::string& content)
{
    return content.find("\r\n") != std::string::npos;
}

std::string WithLineEndings(const std::string& content, bool crlf)
{
    std::string result;
    result.reserve(content.size() + content.size() / 16);
    for (size_t i = 0; i < content.size(); ++i)
    {
        const char character = content[i];
        if (character == '\r' && i + 1 < content.size() && content[i + 1] == '\n')
        {
            continue; // the '\n' below re-adds the requested ending
        }
        if (character == '\n' && crlf)
        {
            result += '\r';
        }
        result += character;
    }
    return result;
}

} // namespace VersionGen
} // namespace SilKit
