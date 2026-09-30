// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace SilKit {
namespace IntegrationTests {

// File sinks write "<LogName>_<ParticipantName>_<Timestamp>.txt" (or ".jsonl") into the working directory, so a log
// file can only be found by its prefix. The unique suffix keeps concurrent or leftover test runs apart.
inline auto MakeUniqueLogName(const std::string& base) -> std::string
{
    return base + "_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

// Tests run in parallel processes that share the working directory, so files may vanish while it is scanned. Only the
// non-throwing overloads are used: a throwing directory_iterator in a destructor would terminate the test process.
inline auto FindLogFiles(const std::string& prefix) -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> logFiles;
    std::error_code ec;
    for (std::filesystem::directory_iterator it{std::filesystem::current_path(), ec}, end; !ec && it != end;
         it.increment(ec))
    {
        std::error_code statusEc;
        if (it->is_regular_file(statusEc) && it->path().filename().string().rfind(prefix, 0) == 0)
        {
            logFiles.push_back(it->path());
        }
    }
    return logFiles;
}

inline auto ReadTextFile(const std::filesystem::path& path) -> std::string
{
    std::ifstream stream{path};
    std::stringstream contents;
    contents << stream.rdbuf();
    return contents.str();
}

// Removes all log files starting with the given prefix when going out of scope.
class ScopedLogFiles
{
public:
    explicit ScopedLogFiles(std::string prefix)
        : _prefix{std::move(prefix)}
    {
    }

    ~ScopedLogFiles()
    {
        for (const auto& logFile : FindLogFiles(_prefix))
        {
            std::error_code ec;
            std::filesystem::remove(logFile, ec);
        }
    }

    ScopedLogFiles(const ScopedLogFiles&) = delete;
    ScopedLogFiles& operator=(const ScopedLogFiles&) = delete;

    auto Prefix() const -> const std::string&
    {
        return _prefix;
    }

    // Concatenated contents of all matching files. Empty if no file exists.
    auto ReadAll() const -> std::string
    {
        std::string contents;
        for (const auto& logFile : FindLogFiles(_prefix))
        {
            contents += ReadTextFile(logFile);
        }
        return contents;
    }

private:
    std::string _prefix;
};

} // namespace IntegrationTests
} // namespace SilKit
