#include <iostream>

#include "ftxui/component/captured_mouse.hpp" // for ftxui
#include "ftxui/component/component.hpp" // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp" // for ComponentBase
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
#include "ftxui/dom/elements.hpp" // for separator, gauge, text, Element, operator|, vbox, border

using namespace ftxui;

class ConsoleUI
{
public:
    ConsoleUI() {}

private:
    ScreenInteractive _screen = ScreenInteractive::FitComponent();

    std::thread _renderThread;

    Components _buttons;
    Components _checkboxesTimers;
    Components _checkboxesReactive;

    std::string _logText;
    Elements _logElements;

    size_t _currentLogOffset = 0;
    size_t _numLogLines = 0;
    constexpr static size_t MAX_NUM_LOG_LINES = 256;
    constexpr static size_t MAX_NUM_LOG_LINES_ON_SCREEN = 26;
    std::array<std::string, MAX_NUM_LOG_LINES> _logLines;
    float _scrollY = 0.0f;

private:
    ButtonOption ButtonStyle()
    {
        auto option = ButtonOption::Animated();
        option.transform = [](const EntryState& s) {
            auto element = text(s.label);
            if (s.focused)
            {
                element |= bold;
            }
            return element | center | borderEmpty | flex;
        };
        return option;
    }

    Component Wrap(Component component)
    {
        return Renderer(component, [component] {
            return hbox({
                       component->Render() | flex,
                   })
                   | flex;
        });
    }


public:
    void SetupScreen() { 
        _screen.SetCursor({0, 0, Screen::Cursor::Shape::Hidden}); 
        for (int i = 0; i < MAX_NUM_LOG_LINES; i++)
        {
            _logLines[i] = "";
        }
    }

    void AddButton(const std::string& label, std::function<void()> onClick)
    {
        _buttons.emplace_back(Button(label, onClick, ButtonStyle()));
    }

    void AddTimerCheckbox(const std::string& label, bool& checked)
    {
        _checkboxesTimers.emplace_back(Checkbox(label, &checked));
    }

    void AddReactiveCheckbox(const std::string& label, bool& checked)
    {
        _checkboxesReactive.emplace_back(Checkbox(label, &checked));
    }

    void Log(std::string line)
    { 
        if (_numLogLines < MAX_NUM_LOG_LINES)
        {
            _logLines[(_currentLogOffset + _numLogLines) % MAX_NUM_LOG_LINES] = line;
            ++_numLogLines;

        }
        else
        {
            _logLines[_currentLogOffset] = line;
            _currentLogOffset = (_currentLogOffset + 1) % MAX_NUM_LOG_LINES;
        }

        if (_numLogLines > MAX_NUM_LOG_LINES_ON_SCREEN)
        {
            _scrollY = static_cast<float>(_numLogLines - MAX_NUM_LOG_LINES_ON_SCREEN / 2 - 1)
                       / static_cast<float>(MAX_NUM_LOG_LINES);
        }

        _screen.Post({}); // Trigger screen update
    }

    std::string GetLogLine(size_t i)
    { 
        size_t shiftedIndex = (_currentLogOffset + i) % MAX_NUM_LOG_LINES;
        return _logLines[shiftedIndex];
    }

    void StartRenderThread()
    {
        _renderThread = std::thread{[this]() {

            // Left menu 
            auto buttons = Wrap(Container::Vertical(_buttons));
            auto checkboxesTimers = Wrap(Container::Vertical(_checkboxesTimers));
            auto checkboxesReactive = Wrap(Container::Vertical(_checkboxesReactive));
            auto manualTriggersFrame = Renderer(buttons, [&] {
                return vbox({
                           text("Manual triggers") | center,
                           separator(),
                           buttons->Render(),
                       })
                       | flex | border;
            });

            auto cyclicFrame = Renderer(checkboxesTimers, [&] {
                return vbox({
                           text("Cyclic") | center,
                           separator(),
                           checkboxesTimers->Render(),
                       })
                       | flex | border;
            });

            auto reactiveMsg = Renderer(checkboxesReactive, [&] {
                return vbox({
                           text("Reactive") | center,
                           separator(),
                           checkboxesReactive->Render(),
                       })
                       | flex | border;
            });
            auto menuFrame = Container::Vertical({manualTriggersFrame, cyclicFrame, reactiveMsg});


            // Right log box
            _logElements.reserve(MAX_NUM_LOG_LINES);
            auto content = Renderer([&] {
                _logElements.clear();
                for (int i = 0; i < MAX_NUM_LOG_LINES; i++)
                {
                    _logElements.push_back(text(GetLogLine(i)));
                }
                return vbox(_logElements);
            });

            SliderOption<float> option_y;
            option_y.value = &_scrollY;
            option_y.min = 0.0f;
            option_y.max = 1.f;
            option_y.increment = static_cast<float>(MAX_NUM_LOG_LINES_ON_SCREEN) / static_cast<float>(MAX_NUM_LOG_LINES);
            option_y.direction = Direction::Down;
            option_y.color_active = Color::Yellow;
            option_y.color_inactive = Color::YellowLight;
            auto scrollbarY = Slider(option_y);
                                    
            auto scrollable_content = Renderer(content, [&, content] {
                return
                    vbox({
                           text("Log") | center | size(WIDTH, EQUAL, 90),
                    separator(), 
                    content->Render() | focusPositionRelative(0, _scrollY) | frame | flex, 
                }) | flex | border;
            });

            auto logFrame = Container::Horizontal({scrollable_content, scrollbarY | borderLight});

            auto layout = Container::Horizontal({menuFrame, logFrame});

            _screen.Loop(layout);
        }};
    }

    void StopRenderThread()
    {
        _screen.Exit();

        if (_renderThread.joinable())
        {
            _renderThread.join();
        }
    }
};
