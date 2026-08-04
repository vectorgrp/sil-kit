// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Standalone tool: generates version_macros.hpp from CLI arguments and the
// current git HEAD hash. Optionally rotates the changelog when the version
// number changes. Requires C++17 (std::filesystem). No external dependencies.
//
// Usage: sil-kit-version-gen [options] <output-header>
//   --major N          Version major (required)
//   --minor N          Version minor (required)
//   --patch N          Version patch (required)
//   --build N          Build number (default: 0)
//   --suffix S         Version suffix (default: "")
//   --git-dir PATH     Path to .git directory (default: search upward from CWD)
//   --git-hash HASH    Override git hash; skips .git reading
//   --source-dir PATH  Source tree root for changelog files (default: CWD)
//   --no-changelog     Skip changelog rotation even if version changed
//   --dry-run          Print actions without writing any files
//   Pass '-' as output-header to write to stdout.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// File helpers
// ---------------------------------------------------------------------------

static std::string readFileFull(const fs::path& path)
{
    std::ifstream f(path);
    if (!f.is_open())
        return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool writeFileFull(const fs::path& path, const std::string& content)
{
    std::ofstream f(path);
    if (!f.is_open())
    {
        std::cerr << "error: cannot write " << path << "\n";
        return false;
    }
    f << content;
    return f.good();
}

static std::string trimStr(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ---------------------------------------------------------------------------
// Git hash resolution (mirrors the old MakeVersionMacros.cmake.in logic)
// ---------------------------------------------------------------------------

static fs::path findGitDir()
{
    fs::path dir = fs::current_path();
    while (true)
    {
        fs::path candidate = dir / ".git";
        if (fs::exists(candidate / "HEAD"))
            return candidate;
        fs::path parent = dir.parent_path();
        if (parent == dir)
            break;
        dir = parent;
    }
    return {};
}

static std::string resolveGitHash(const fs::path& gitDir)
{
    std::string headContent = trimStr(readFileFull(gitDir / "HEAD"));
    if (headContent.empty())
        return "UNKNOWN";

    const std::string refPrefix = "ref: ";
    if (headContent.compare(0, refPrefix.size(), refPrefix) == 0)
    {
        std::string ref = headContent.substr(refPrefix.size());
        fs::path refFile = gitDir / ref;

        if (fs::exists(refFile))
            return trimStr(readFileFull(refFile));

        // Fall back to packed-refs
        std::string packedRefs = readFileFull(gitDir / "packed-refs");
        if (!packedRefs.empty())
        {
            std::istringstream stream(packedRefs);
            std::string line;
            while (std::getline(stream, line))
            {
                if (line.empty() || line[0] == '#' || line[0] == '^')
                    continue;
                size_t space = line.find(' ');
                if (space != std::string::npos && trimStr(line.substr(space + 1)) == ref)
                    return line.substr(0, space);
            }
        }
        return "UNKNOWN";
    }

    // Detached HEAD — content is the hash directly
    if (headContent.size() >= 40)
        return headContent.substr(0, 40);
    return headContent;
}

// ---------------------------------------------------------------------------
// Parse current version from an existing version_macros.hpp
// ---------------------------------------------------------------------------

struct Version
{
    int major{-1}, minor{-1}, patch{-1};
    bool valid() const { return major >= 0 && minor >= 0 && patch >= 0; }
};

static int extractDefineInt(const std::string& content, const std::string& macroName)
{
    std::string search = "#define " + macroName + " ";
    size_t pos = content.find(search);
    if (pos == std::string::npos)
        return -1;
    pos += search.size();
    size_t end = content.find_first_of("\r\n", pos);
    return std::atoi(trimStr(content.substr(pos, end - pos)).c_str());
}

static Version parseVersionFromHeader(const fs::path& path)
{
    Version v;
    std::string content = readFileFull(path);
    if (content.empty())
        return v;
    v.major = extractDefineInt(content, "SILKIT_VERSION_MAJOR");
    v.minor = extractDefineInt(content, "SILKIT_VERSION_MINOR");
    v.patch = extractDefineInt(content, "SILKIT_VERSION_PATCH");
    return v;
}

// ---------------------------------------------------------------------------
// Changelog rotation
// ---------------------------------------------------------------------------

static bool rotateChangelog(const fs::path& sourceDir, const Version& oldVer, bool dryRun)
{
    fs::path versionsDir = sourceDir / "docs" / "changelog" / "versions";
    fs::path latestMd = versionsDir / "latest.md";
    fs::path templateMd = versionsDir / "template.md";

    fs::path archiveMd = versionsDir / (std::to_string(oldVer.major) + "." + std::to_string(oldVer.minor) + "."
                                        + std::to_string(oldVer.patch) + ".md");

    if (!fs::exists(latestMd))
    {
        std::cerr << "warning: changelog rotation skipped — " << latestMd << " not found\n";
        return true;
    }
    if (!fs::exists(templateMd))
    {
        std::cerr << "warning: changelog rotation skipped — " << templateMd << " not found\n";
        return true;
    }
    if (fs::exists(archiveMd))
    {
        std::cerr << "warning: changelog rotation skipped — " << archiveMd << " already exists\n";
        return true;
    }

    if (dryRun)
    {
        std::cout << "[dry-run] copy   " << latestMd << "\n"
                  << "          ->     " << archiveMd << "\n";
        std::cout << "[dry-run] copy   " << templateMd << "\n"
                  << "          ->     " << latestMd << "\n";
        return true;
    }

    std::error_code ec;
    std::cout << "changelog: " << latestMd << " -> " << archiveMd << "\n";
    fs::copy_file(latestMd, archiveMd, ec);
    if (ec)
    {
        std::cerr << "error: " << ec.message() << "\n";
        return false;
    }
    std::cout << "changelog: " << latestMd << " <- " << templateMd << "\n";
    fs::copy_file(templateMd, latestMd, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        std::cerr << "error: " << ec.message() << "\n";
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Header generation
// ---------------------------------------------------------------------------

static void replaceAll(std::string& s, const std::string& from, const std::string& to)
{
    for (size_t pos = 0; (pos = s.find(from, pos)) != std::string::npos; pos += to.size())
        s.replace(pos, from.size(), to);
}

static void substituteVar(std::string& s, const std::string& name, const std::string& value)
{
    replaceAll(s, "@" + name + "@", value);
}

// clang-format off
static constexpr auto kHeaderTemplate = R"(// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#define SILKIT_GIT_HASH "@GIT_HASH@"
#define SILKIT_VERSION_MAJOR @MAJOR@
#define SILKIT_VERSION_MINOR @MINOR@
#define SILKIT_VERSION_PATCH @PATCH@
#define SILKIT_BUILD_NUMBER @BUILD@
#define SILKIT_VERSION_STRING "@VERSION_STRING@"
#define SILKIT_VERSION_SUFFIX "@SUFFIX@"
)";
// clang-format on

static std::string buildHeader(const std::string& gitHash, int major, int minor, int patch, int build,
                               const std::string& suffix)
{
    const std::string versionStr = std::to_string(major) + "." + std::to_string(minor) + "."
                                   + std::to_string(patch) + (suffix.empty() ? "" : "-" + suffix);

    std::string result{kHeaderTemplate};
    substituteVar(result, "GIT_HASH", gitHash);
    substituteVar(result, "MAJOR", std::to_string(major));
    substituteVar(result, "MINOR", std::to_string(minor));
    substituteVar(result, "PATCH", std::to_string(patch));
    substituteVar(result, "BUILD", std::to_string(build));
    substituteVar(result, "VERSION_STRING", versionStr);
    substituteVar(result, "SUFFIX", suffix);
    return result;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

static void usage(const char* prog)
{
    std::cerr << "Usage: " << prog << " [options] <output-header|->\n"
              << "\n"
              << "  --major N          Version major (required)\n"
              << "  --minor N          Version minor (required)\n"
              << "  --patch N          Version patch (required)\n"
              << "  --build N          Build number (default: 0)\n"
              << "  --suffix S         Version suffix (default: \"\")\n"
              << "  --git-dir PATH     Path to .git directory\n"
              << "  --git-hash HASH    Override git hash (skips .git reading)\n"
              << "  --source-dir PATH  Source tree root for changelog files\n"
              << "  --no-changelog     Skip changelog rotation\n"
              << "  --dry-run          Print actions without writing any files\n";
}

int main(int argc, char* argv[])
{
    int major = -1, minor = -1, patch = -1, build = 0;
    std::string suffix;
    fs::path gitDir;
    std::string gitHashOverride;
    fs::path sourceDir;
    std::string outputPath;
    bool noChangelog = false;
    bool dryRun = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        auto requireNext = [&]() -> std::string {
            if (i + 1 >= argc)
            {
                std::cerr << "error: " << arg << " requires an argument\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if (arg == "--major")
            major = std::atoi(requireNext().c_str());
        else if (arg == "--minor")
            minor = std::atoi(requireNext().c_str());
        else if (arg == "--patch")
            patch = std::atoi(requireNext().c_str());
        else if (arg == "--build")
            build = std::atoi(requireNext().c_str());
        else if (arg == "--suffix")
            suffix = requireNext();
        else if (arg == "--git-dir")
            gitDir = requireNext();
        else if (arg == "--git-hash")
            gitHashOverride = requireNext();
        else if (arg == "--source-dir")
            sourceDir = requireNext();
        else if (arg == "--no-changelog")
            noChangelog = true;
        else if (arg == "--dry-run" || arg == "-n")
            dryRun = true;
        else if (!arg.empty() && arg[0] != '-')
            outputPath = arg;
        else
        {
            std::cerr << "error: unknown option '" << arg << "'\n";
            usage(argv[0]);
            return 1;
        }
    }

    if (major < 0 || minor < 0 || patch < 0)
    {
        std::cerr << "error: --major, --minor, --patch are required\n";
        usage(argv[0]);
        return 1;
    }
    if (outputPath.empty())
    {
        std::cerr << "error: output path is required\n";
        usage(argv[0]);
        return 1;
    }

    // Resolve git hash
    std::string gitHash;
    if (!gitHashOverride.empty())
    {
        gitHash = gitHashOverride;
    }
    else
    {
        if (gitDir.empty())
            gitDir = findGitDir();
        gitHash = gitDir.empty() ? "UNKNOWN" : resolveGitHash(gitDir);
    }

    // Changelog rotation — read existing header before overwriting
    if (!noChangelog && outputPath != "-")
    {
        Version oldVer = parseVersionFromHeader(outputPath);
        bool versionChanged =
            oldVer.valid() && (oldVer.major != major || oldVer.minor != minor || oldVer.patch != patch);
        if (versionChanged)
        {
            fs::path srcDir = sourceDir.empty() ? fs::current_path() : sourceDir;
            if (!rotateChangelog(srcDir, oldVer, dryRun))
                return 1;
        }
    }

    // Generate and write the header
    std::string content = buildHeader(gitHash, major, minor, patch, build, suffix);

    if (dryRun)
    {
        std::cout << "[dry-run] write  " << outputPath << "\n"
                  << "---\n"
                  << content;
        return 0;
    }

    if (outputPath == "-")
    {
        std::cout << content;
    }
    else
    {
        if (!writeFileFull(outputPath, content))
            return 1;
        std::cout << "wrote " << outputPath << "  (git hash: " << gitHash << ")\n";
    }

    return 0;
}
