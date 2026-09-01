// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Standalone maintainer tool that performs a SIL Kit version bump:
//
//   * rewrites the version numbers in SilKit/cmake/SilKitVersion.cmake
//   * regenerates SilKit/include/silkit/capi/SilKitVersionMacros.h
//   * archives docs/changelog/versions/latest.md as <old-version>.md and lists
//     it in docs/changelog/overview.rst
//
// Either all of that succeeds or nothing is written. Run without version
// arguments to only refresh the generated header (e.g. after a rebase).
//
// See docs/development/release.md for the full procedure.
//
// Requires C++17 (std::filesystem). No external dependencies.

#include "GenerateVersion.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using SilKit::VersionGen::Version;

namespace {

// ---------------------------------------------------------------------------
// File helpers
// ---------------------------------------------------------------------------

// Paths are printed through Show(): streaming an fs::path quotes it and escapes
// the backslashes, which is unreadable on Windows. Normalizing also keeps the
// separators consistent in paths assembled from "a/b/c" fragments.
std::string Show(const fs::path& path)
{
    return path.lexically_normal().make_preferred().string();
}

std::string ReadFileFull(const fs::path& path)
{
    std::ifstream file{path, std::ios::binary};
    if (!file.is_open())
    {
        return "";
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Written in binary mode: the line endings are chosen explicitly by the caller
// to match the working tree rather than left to the platform's text mode.
bool WriteFileFull(const fs::path& path, const std::string& content)
{
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file.is_open())
    {
        std::cerr << "error: cannot write " << Show(path) << "\n";
        return false;
    }
    file << content;
    if (!file.good())
    {
        std::cerr << "error: failed while writing " << Show(path) << "\n";
        return false;
    }
    return true;
}

std::string TrimStr(const std::string& str)
{
    const size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    const size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// ---------------------------------------------------------------------------
// Source tree layout
// ---------------------------------------------------------------------------

const char* const kCMakeVersionFile = "SilKit/cmake/SilKitVersion.cmake";
const char* const kVersionMacrosHeader = "SilKit/include/silkit/capi/SilKitVersionMacros.h";
const char* const kChangelogVersionsDir = "docs/changelog/versions";
const char* const kChangelogOverview = "docs/changelog/overview.rst";

struct Layout
{
    fs::path sourceDir;
    fs::path cmakeVersionFile;
    fs::path versionsDir;
    fs::path latestMd;
    fs::path overviewRst;
};

Layout MakeLayout(const fs::path& sourceDir)
{
    Layout layout;
    layout.sourceDir = sourceDir;
    layout.cmakeVersionFile = sourceDir / kCMakeVersionFile;
    layout.versionsDir = sourceDir / kChangelogVersionsDir;
    layout.latestMd = layout.versionsDir / "latest.md";
    layout.overviewRst = sourceDir / kChangelogOverview;
    return layout;
}

// A directory is the source tree root if it holds SilKitVersion.cmake.
fs::path FindSourceDir()
{
    fs::path dir = fs::current_path();
    while (true)
    {
        if (fs::exists(dir / kCMakeVersionFile))
        {
            return dir;
        }
        const fs::path parent = dir.parent_path();
        if (parent == dir)
        {
            break;
        }
        dir = parent;
    }
    return {};
}

// ---------------------------------------------------------------------------
// Git hash resolution (no git binary required)
// ---------------------------------------------------------------------------

// In a linked worktree '.git' is a file containing 'gitdir: <path>'.
fs::path ResolveGitDir(const fs::path& candidate)
{
    if (fs::is_directory(candidate))
    {
        return candidate;
    }
    if (!fs::is_regular_file(candidate))
    {
        return {};
    }

    const std::string content = TrimStr(ReadFileFull(candidate));
    const std::string prefix = "gitdir: ";
    if (content.compare(0, prefix.size(), prefix) != 0)
    {
        return {};
    }
    fs::path gitDir = TrimStr(content.substr(prefix.size()));
    if (gitDir.is_relative())
    {
        gitDir = candidate.parent_path() / gitDir;
    }
    return fs::is_directory(gitDir) ? gitDir : fs::path{};
}

std::string FindPackedRef(const fs::path& packedRefs, const std::string& ref)
{
    const std::string content = ReadFileFull(packedRefs);
    if (content.empty())
    {
        return "";
    }
    std::istringstream stream{content};
    std::string line;
    while (std::getline(stream, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == '^')
        {
            continue;
        }
        const size_t space = line.find(' ');
        if (space != std::string::npos && TrimStr(line.substr(space + 1)) == ref)
        {
            return line.substr(0, space);
        }
    }
    return "";
}

std::string ResolveGitHash(const fs::path& gitDir)
{
    const std::string head = TrimStr(ReadFileFull(gitDir / "HEAD"));
    if (head.empty())
    {
        return "UNKNOWN";
    }

    const std::string refPrefix = "ref: ";
    if (head.compare(0, refPrefix.size(), refPrefix) != 0)
    {
        // Detached HEAD: the content is the hash itself.
        return head.size() >= 40 ? head.substr(0, 40) : head;
    }

    const std::string ref = TrimStr(head.substr(refPrefix.size()));

    // A linked worktree keeps HEAD locally but shares refs via 'commondir'.
    std::vector<fs::path> searchDirs{gitDir};
    const std::string commonDir = TrimStr(ReadFileFull(gitDir / "commondir"));
    if (!commonDir.empty())
    {
        fs::path common = commonDir;
        if (common.is_relative())
        {
            common = gitDir / common;
        }
        searchDirs.push_back(fs::weakly_canonical(common));
    }

    for (const auto& dir : searchDirs)
    {
        if (fs::exists(dir / ref))
        {
            return TrimStr(ReadFileFull(dir / ref));
        }
    }
    for (const auto& dir : searchDirs)
    {
        const std::string packed = FindPackedRef(dir / "packed-refs", ref);
        if (!packed.empty())
        {
            return packed;
        }
    }
    return "UNKNOWN";
}

// ---------------------------------------------------------------------------
// Command line
// ---------------------------------------------------------------------------

void PrintUsage(const char* prog)
{
    std::cout << "Usage: " << prog << " [options] [output-header|-]\n"
              << "\n"
              << "Performs a SIL Kit version bump: patches SilKitVersion.cmake, regenerates\n"
              << "SilKitVersionMacros.h, and rotates the changelog. Without --major/--minor/\n"
              << "--patch it only regenerates the header from the current version.\n"
              << "\n"
              << "  --major N          New version major (requires --minor and --patch)\n"
              << "  --minor N          New version minor\n"
              << "  --patch N          New version patch\n"
              << "  --suffix S         Version suffix, e.g. rc1 (default: keep current)\n"
              << "  --date YYYY-MM-DD  Release date for the archived changelog entry\n"
              << "                     (default: today)\n"
              << "  --source-dir PATH  Source tree root (default: search upward from CWD)\n"
              << "  --git-dir PATH     Path to .git (default: <source-dir>/.git)\n"
              << "  --git-hash HASH    Override the git hash; skips reading .git\n"
              << "  --no-changelog     Do not rotate the changelog\n"
              << "  --force            Overwrite an existing archived changelog entry\n"
              << "  --check            Verify the header matches SilKitVersion.cmake and exit;\n"
              << "                     writes nothing, non-zero exit on mismatch\n"
              << "  --dry-run, -n      Print what would happen, write nothing\n"
              << "  --help, -h         Show this help\n"
              << "\n"
              << "The output header defaults to <source-dir>/" << kVersionMacrosHeader << ".\n"
              << "Pass '-' to write the header to stdout instead. An existing file that is not\n"
              << "a generated version-macros header is never overwritten.\n"
              << "\n"
              << "The build number is not set here: it is a property of a build, not of the\n"
              << "source tree. Pass -DSILKIT_BUILD_NUMBER=N to CMake; the generated header only\n"
              << "provides the fallback of 0.\n"
              << "\n"
              << "See docs/development/release.md for the release procedure.\n";
}

struct Options
{
    int major{-1};
    int minor{-1};
    int patch{-1};
    bool haveSuffix{false};
    std::string suffix;
    std::string date;
    fs::path sourceDir;
    fs::path gitDir;
    bool haveGitDir{false};
    std::string gitHashOverride;
    std::string outputPath;
    bool haveOutputPath{false};
    bool noChangelog{false};
    bool force{false};
    bool check{false};
    bool dryRun{false};
};

bool ParseOptions(int argc, char* argv[], Options& options)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        auto requireNext = [&]() -> std::string {
            if (i + 1 >= argc)
            {
                std::cerr << "error: " << arg << " requires an argument\n";
                std::exit(1);
            }
            return argv[++i];
        };

        if (arg == "--help" || arg == "-h")
        {
            PrintUsage(argv[0]);
            std::exit(0);
        }
        else if (arg == "--major")
        {
            options.major = std::atoi(requireNext().c_str());
        }
        else if (arg == "--minor")
        {
            options.minor = std::atoi(requireNext().c_str());
        }
        else if (arg == "--patch")
        {
            options.patch = std::atoi(requireNext().c_str());
        }
        else if (arg == "--suffix")
        {
            options.suffix = requireNext();
            options.haveSuffix = true;
        }
        else if (arg == "--date")
        {
            options.date = requireNext();
        }
        else if (arg == "--source-dir")
        {
            options.sourceDir = requireNext();
        }
        else if (arg == "--git-dir")
        {
            options.gitDir = requireNext();
            options.haveGitDir = true;
        }
        else if (arg == "--git-hash")
        {
            options.gitHashOverride = requireNext();
        }
        else if (arg == "--no-changelog")
        {
            options.noChangelog = true;
        }
        else if (arg == "--force")
        {
            options.force = true;
        }
        else if (arg == "--check")
        {
            options.check = true;
        }
        else if (arg == "--dry-run" || arg == "-n")
        {
            options.dryRun = true;
        }
        else if (arg == "-" || (!arg.empty() && arg[0] != '-'))
        {
            if (options.haveOutputPath)
            {
                std::cerr << "error: more than one output path given ('" << options.outputPath << "' and '" << arg
                          << "')\n";
                return false;
            }
            options.outputPath = arg;
            options.haveOutputPath = true;
        }
        else
        {
            std::cerr << "error: unknown option '" << arg << "'\n";
            PrintUsage(argv[0]);
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Planned file writes: everything is validated before anything is written
// ---------------------------------------------------------------------------

struct PlannedWrite
{
    fs::path path;
    std::string content;
    std::string what;
};

bool CommitWrites(const std::vector<PlannedWrite>& writes, bool useCrlf, bool dryRun)
{
    for (const auto& write : writes)
    {
        if (dryRun)
        {
            std::cout << "[dry-run] " << write.what << ": " << Show(write.path) << "\n";
            continue;
        }
        if (!WriteFileFull(write.path, SilKit::VersionGen::WithLineEndings(write.content, useCrlf)))
        {
            return false;
        }
        std::cout << write.what << ": " << Show(write.path) << "\n";
    }
    return true;
}

int RunCheck(const Layout& layout, const fs::path& headerPath, const Version& cmakeVersion)
{
    if (!fs::exists(headerPath))
    {
        std::cerr << "error: " << Show(headerPath) << " does not exist\n";
        return 1;
    }

    const std::string headerContent = ReadFileFull(headerPath);
    const Version headerVersion = SilKit::VersionGen::ParseVersionFromHeader(headerContent);
    if (!headerVersion.IsValid())
    {
        std::cerr << "error: " << Show(headerPath) << " has no usable version macros\n";
        return 1;
    }
    if (!headerVersion.SameNumbers(cmakeVersion) || headerVersion.suffix != cmakeVersion.suffix)
    {
        std::cerr << "error: version drift\n"
                  << "  " << Show(layout.cmakeVersionFile) << ": " << cmakeVersion.ToString() << "\n"
                  << "  " << Show(headerPath) << ": " << headerVersion.ToString() << "\n"
                  << "  run sil-kit-generate-version (without version arguments) to regenerate the header\n";
        return 1;
    }

    std::cout << "ok: " << Show(headerPath) << " matches " << Show(layout.cmakeVersionFile) << " at "
              << cmakeVersion.ToString() << "\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[])
{
    Options options;
    if (!ParseOptions(argc, argv, options))
    {
        return 1;
    }

    // --- locate the source tree -------------------------------------------

    fs::path sourceDir = options.sourceDir;
    if (sourceDir.empty())
    {
        sourceDir = FindSourceDir();
    }
    if (sourceDir.empty() || !fs::exists(sourceDir / kCMakeVersionFile))
    {
        std::cerr << "error: no SIL Kit source tree found (expected " << kCMakeVersionFile << ")\n"
                  << "  run from inside the source tree or pass --source-dir PATH\n";
        return 1;
    }
    const Layout layout = MakeLayout(sourceDir);

    // --- the version currently recorded in the source tree ------------------

    std::string cmakeContent = ReadFileFull(layout.cmakeVersionFile);
    // Generated files follow the line endings the working tree already uses,
    // which depends on git's core.autocrlf.
    const bool useCrlf = SilKit::VersionGen::UsesCrlf(cmakeContent);
    const Version currentVersion = SilKit::VersionGen::ParseVersionFromCMake(cmakeContent);
    if (!currentVersion.IsValid())
    {
        std::cerr << "error: cannot read the version from " << Show(layout.cmakeVersionFile) << "\n";
        return 1;
    }

    // --- output header ------------------------------------------------------

    const bool toStdout = options.outputPath == "-";
    const fs::path headerPath =
        (options.haveOutputPath && !toStdout) ? fs::path{options.outputPath} : sourceDir / kVersionMacrosHeader;

    if (options.check)
    {
        return RunCheck(layout, headerPath, currentVersion);
    }

    // --- the version being written ------------------------------------------

    const int given = (options.major >= 0) + (options.minor >= 0) + (options.patch >= 0);
    if (given != 0 && given != 3)
    {
        std::cerr << "error: --major, --minor and --patch must be given together\n";
        return 1;
    }

    Version newVersion = currentVersion;
    if (given == 3)
    {
        newVersion.major = options.major;
        newVersion.minor = options.minor;
        newVersion.patch = options.patch;
    }
    if (options.haveSuffix)
    {
        newVersion.suffix = options.suffix;
    }

    const bool bumping = !newVersion.SameNumbers(currentVersion);
    const bool cmakeChanged = bumping || newVersion.suffix != currentVersion.suffix;

    // --- guard the output header --------------------------------------------

    if (!toStdout && fs::exists(headerPath))
    {
        const std::string existing = ReadFileFull(headerPath);
        if (!SilKit::VersionGen::LooksLikeGeneratedHeader(existing))
        {
            std::cerr << "error: refusing to overwrite " << Show(headerPath) << "\n"
                      << "  this is not a generated version-macros header (no SILKIT_VERSION_MAJOR /\n"
                      << "  SILKIT_GIT_HASH defines). Did you mean " << kVersionMacrosHeader << "?\n";
            return 1;
        }

        // Report drift instead of silently taking the header's word for it.
        const Version headerVersion = SilKit::VersionGen::ParseVersionFromHeader(existing);
        if (headerVersion.IsValid() && !headerVersion.SameNumbers(currentVersion))
        {
            std::cout << "note: header was at " << headerVersion.ToString() << ", " << Show(layout.cmakeVersionFile)
                      << " says " << currentVersion.ToString() << "; using the latter\n";
        }
    }

    // --- git hash -----------------------------------------------------------

    std::string gitHash = options.gitHashOverride;
    if (gitHash.empty())
    {
        const fs::path gitDir = ResolveGitDir(options.haveGitDir ? options.gitDir : sourceDir / ".git");
        gitHash = gitDir.empty() ? "UNKNOWN" : ResolveGitHash(gitDir);
        if (gitHash == "UNKNOWN")
        {
            std::cerr << "warning: could not determine the git hash; writing \"UNKNOWN\"\n";
        }
    }

    // --- stdout mode: no side effects ---------------------------------------

    if (toStdout)
    {
        std::cout << SilKit::VersionGen::RenderHeader(gitHash, newVersion);
        return 0;
    }

    // --- plan every write, validating as we go ------------------------------

    std::vector<PlannedWrite> writes;
    const bool rotating = bumping && !options.noChangelog;

    if (cmakeChanged)
    {
        std::string error;
        if (!SilKit::VersionGen::PatchCMakeVersion(cmakeContent, newVersion, error))
        {
            std::cerr << "error: cannot update " << Show(layout.cmakeVersionFile) << ": " << error << "\n";
            return 1;
        }
        writes.push_back({layout.cmakeVersionFile, cmakeContent,
                          "version " + currentVersion.ToString() + " -> " + newVersion.ToString()});
    }

    if (rotating)
    {
        const fs::path archiveMd = layout.versionsDir / (currentVersion.ToShortString() + ".md");

        if (!fs::exists(layout.latestMd))
        {
            std::cerr << "error: " << Show(layout.latestMd) << " not found; cannot rotate the changelog\n"
                      << "  pass --no-changelog to bump the version anyway\n";
            return 1;
        }
        if (fs::exists(archiveMd) && !options.force)
        {
            std::cerr << "error: " << Show(archiveMd) << " already exists\n"
                      << "  " << Show(layout.cmakeVersionFile) << " says the current version is "
                      << currentVersion.ToString() << ", but that entry is already archived.\n"
                      << "  Check the version numbers, or pass --force to overwrite, or --no-changelog\n"
                      << "  to leave the changelog alone.\n";
            return 1;
        }
        if (!fs::exists(layout.overviewRst))
        {
            std::cerr << "error: " << Show(layout.overviewRst) << " not found\n";
            return 1;
        }

        const std::string date = options.date.empty() ? SilKit::VersionGen::TodayIsoDate() : options.date;
        if (date.empty())
        {
            std::cerr << "error: cannot determine today's date; pass --date YYYY-MM-DD\n";
            return 1;
        }
        const std::string latestContent = ReadFileFull(layout.latestMd);

        writes.push_back({archiveMd, SilKit::VersionGen::FinalizeChangelogHeading(latestContent, currentVersion, date),
                          "changelog: archive " + currentVersion.ToString() + " (" + date + ")"});
        writes.push_back({layout.latestMd, SilKit::VersionGen::RenderChangelogStub(newVersion),
                          "changelog: reset latest.md for " + newVersion.ToString()});

        std::string overviewContent = ReadFileFull(layout.overviewRst);
        std::string error;
        if (SilKit::VersionGen::InsertChangelogToctreeEntry(overviewContent, currentVersion, error))
        {
            writes.push_back({layout.overviewRst, overviewContent,
                              "changelog: list " + currentVersion.ToShortString() + ".md in the toctree"});
        }
        else if (!options.force)
        {
            std::cerr << "error: cannot update " << Show(layout.overviewRst) << ": " << error << "\n";
            return 1;
        }
        else
        {
            std::cout << "note: " << Show(layout.overviewRst) << " left unchanged: " << error << "\n";
        }
    }

    writes.push_back({headerPath, SilKit::VersionGen::RenderHeader(gitHash, newVersion),
                      "header " + newVersion.ToString() + " (git hash: " + gitHash + ")"});

    if (!CommitWrites(writes, useCrlf, options.dryRun))
    {
        return 1;
    }

    if (!options.dryRun && bumping)
    {
        std::cout << "\nNext: fill in " << Show(layout.latestMd) << " and review 'git diff'.\n"
                  << "See docs/development/release.md.\n";
    }
    return 0;
}
