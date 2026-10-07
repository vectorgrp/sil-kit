// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <io.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace Introspection {

// ================================================================================
//  Styled text
// ================================================================================

// SGR parameter strings (the part between "\x1b[" and "m").
namespace Style {
const char* const None = "";
const char* const Bold = "1";
const char* const Dim = "2";
const char* const Italic = "3";
const char* const Strike = "2;9;38;5;203";
const char* const Red = "38;5;203";
const char* const Green = "38;5;114";
const char* const NewItem = "1;38;5;156";
const char* const Yellow = "38;5;221";
const char* const Cyan = "38;5;117";
const char* const Grey = "38;5;245";
const char* const DarkGrey = "38;5;239";
const char* const White = "1;38;5;255";
const char* const Header = "1;38;5;255;48;5;24";
const char* const HeaderDim = "38;5;153;48;5;24";
const char* const Section = "1;38;5;75";
} // namespace Style

//! Number of terminal columns a UTF-8 string occupies (all glyphs used here are single-width).
inline auto VisibleWidth(const std::string& s) -> size_t
{
    size_t width = 0;
    for (unsigned char c : s)
    {
        if ((c & 0xC0) != 0x80)
        {
            ++width;
        }
    }
    return width;
}

//! Returns the prefix of a UTF-8 string that occupies at most maxWidth columns.
inline auto CutToWidth(const std::string& s, size_t maxWidth) -> std::string
{
    size_t width = 0;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const auto c = static_cast<unsigned char>(s[i]);
        if ((c & 0xC0) != 0x80)
        {
            if (width == maxWidth)
            {
                return s.substr(0, i);
            }
            ++width;
        }
    }
    return s;
}

inline auto Repeat(const std::string& s, size_t count) -> std::string
{
    std::string result;
    result.reserve(s.size() * count);
    for (size_t i = 0; i < count; ++i)
    {
        result += s;
    }
    return result;
}

//! A line of text made of differently styled segments; knows its visible width.
class Text
{
public:
    Text() = default;
    Text(std::string text, std::string style = Style::None)
    {
        Add(std::move(text), std::move(style));
    }

    auto Add(std::string text, std::string style = Style::None) -> Text&
    {
        if (!text.empty())
        {
            _width += VisibleWidth(text);
            _segments.emplace_back(std::move(style), std::move(text));
        }
        return *this;
    }

    auto Add(const Text& other) -> Text&
    {
        for (const auto& segment : other._segments)
        {
            Add(segment.second, segment.first);
        }
        return *this;
    }

    auto Spaces(size_t count) -> Text&
    {
        return Add(std::string(count, ' '));
    }

    //! Pad with spaces (or truncate with an ellipsis) to exactly the given width.
    auto Fit(size_t width) const -> Text
    {
        if (_width <= width)
        {
            Text result{*this};
            result.Spaces(width - _width);
            return result;
        }
        return Truncated(width);
    }

    auto Truncated(size_t maxWidth) const -> Text
    {
        if (_width <= maxWidth)
        {
            return *this;
        }
        Text result;
        if (maxWidth == 0)
        {
            return result;
        }
        size_t remaining = maxWidth - 1;
        for (const auto& segment : _segments)
        {
            if (remaining == 0)
            {
                break;
            }
            const auto part = CutToWidth(segment.second, remaining);
            remaining -= VisibleWidth(part);
            result.Add(part, segment.first);
        }
        result.Add("…", Style::Grey);
        return result;
    }

    auto Width() const -> size_t
    {
        return _width;
    }

    auto Render(bool color) const -> std::string
    {
        std::string out;
        for (const auto& segment : _segments)
        {
            if (color && !segment.first.empty())
            {
                out += "\x1b[" + segment.first + "m" + segment.second + "\x1b[0m";
            }
            else
            {
                out += segment.second;
            }
        }
        return out;
    }

private:
    std::vector<std::pair<std::string, std::string>> _segments; // (style, text)
    size_t _width{0};
};

// ================================================================================
//  Terminal control
// ================================================================================

//! Puts the console into a full-screen, VT-capable mode for the lifetime of the object.
class Terminal
{
public:
    Terminal() = default;
    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;

    ~Terminal()
    {
        LeaveFullscreen();
#if defined(_WIN32)
        if (_modeChanged)
        {
            SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), _originalMode);
        }
#endif
    }

    //! Enables UTF-8 output and ANSI escape sequence processing. Returns false if stdout is
    //! not an interactive terminal that understands escape sequences.
    auto EnableVirtualTerminal() -> bool
    {
#if defined(_WIN32)
        SetConsoleOutputCP(CP_UTF8);
        if (!_isatty(_fileno(stdout)))
        {
            return false;
        }
        const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!GetConsoleMode(handle, &_originalMode))
        {
            return false;
        }
        if (!SetConsoleMode(handle, _originalMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        {
            return false;
        }
        _modeChanged = true;
        return true;
#else
        return isatty(fileno(stdout)) != 0;
#endif
    }

    void EnterFullscreen()
    {
        // Alternate screen buffer, hide cursor, disable line wrap.
        Write("\x1b[?1049h\x1b[?25l\x1b[?7l\x1b[2J");
        _fullscreen = true;
    }

    void LeaveFullscreen()
    {
        if (_fullscreen)
        {
            Write("\x1b[0m\x1b[?7h\x1b[?25h\x1b[?1049l");
            _fullscreen = false;
        }
    }

    //! Returns the console size as (columns, rows).
    static auto Size() -> std::pair<size_t, size_t>
    {
#if defined(_WIN32)
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        {
            return {static_cast<size_t>(info.srWindow.Right - info.srWindow.Left + 1),
                    static_cast<size_t>(info.srWindow.Bottom - info.srWindow.Top + 1)};
        }
#else
        winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0)
        {
            return {static_cast<size_t>(ws.ws_col), static_cast<size_t>(ws.ws_row)};
        }
#endif
        return {120, 40};
    }

    //! Draws a full frame in place: every line is overwritten and cleared to its end.
    void DrawFrame(const std::vector<Text>& lines, size_t columns)
    {
        std::string frame = "\x1b[H";
        for (size_t i = 0; i < lines.size(); ++i)
        {
            frame += lines[i].Truncated(columns > 0 ? columns - 1 : 0).Render(true);
            frame += "\x1b[0m\x1b[K";
            if (i + 1 < lines.size())
            {
                frame += "\r\n";
            }
        }
        frame += "\x1b[J";
        if (frame != _lastFrame)
        {
            Write(frame);
            _lastFrame = std::move(frame);
        }
    }

    void Invalidate()
    {
        _lastFrame.clear();
        Write("\x1b[2J");
    }

    static void Write(const std::string& s)
    {
        std::fwrite(s.data(), 1, s.size(), stdout);
        std::fflush(stdout);
    }

private:
    bool _fullscreen{false};
    std::string _lastFrame;
#if defined(_WIN32)
    DWORD _originalMode{0};
    bool _modeChanged{false};
#endif
};

} // namespace Introspection
