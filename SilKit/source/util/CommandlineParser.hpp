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

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <memory>

#include "Assert.hpp"

namespace SilKit {
namespace Util {

//! \brief Parse commandline arguments
class CommandlineParser
{
public:
    CommandlineParser() = default;

     //! \brief Declare a new argument
    template<class TArgument, typename... Args>
    auto& Add(Args&&... args)
    {
        _arguments.push_back(std::make_unique<TArgument>(std::forward<Args>(args)...));
        return *this;
    }

    /*! \brief Retrieve a commandline argument by its name
     * \throw SilKit::SilKitError when argument does not exist or is of a different kind
     */
    template<class TArgument>
    auto Get(std::string name) -> TArgument&
    {
        auto* argument = GetByName<TArgument>(name);
        if (!argument)
        {
            throw SilKitError("Unknown argument '" + name + "'");
        }
        return *argument;
    }

    /*! \brief Output usage info for previously declared parameters to the given stream
     */
    void PrintUsageInfo(std::ostream& out, const std::string& executableName)
    {
        out << "Usage: " << executableName;
        for (auto& argument : _arguments)
        {
            if (argument->IsHidden()) continue;
            out << " " << argument->Usage();
        }
        out << std::endl;
        out << "Arguments:" << std::endl;
        for (auto& argument : _arguments)
        {
            if (argument->IsHidden()) continue;
            out << argument->Description() << std::endl;
        }
    }

    /*! \brief Parse arguments based on argc/argv parameters from a main function
     * \throw SilKit::SilKitError when a parsing error occurs
     */
    void ParseArguments(int argc, char** argv)
    {
        auto positionalArgumentIt = std::find_if(_arguments.begin(), _arguments.end(), [](const auto& el)
            { return el->Kind() == ArgumentKind::Positional || el->Kind() == ArgumentKind::PositionalList; });
        for (auto i = 1; i < argc; ++i)
        {
            std::string argument{ argv[i] };

            auto arg{ argument };
            auto isShortForm{ false };
            if (arg.length() >= 3 && arg.substr(0, 2) == "--")
            {
                arg.erase(0, 2);
            }
            else if (arg.length() >= 2 && arg.substr(0, 1) == "-")
            {
                arg.erase(0, 1);
                isShortForm = true;
            }
            else if (positionalArgumentIt != _arguments.end())
            {
                if ((*positionalArgumentIt)->Kind() == ArgumentKind::Positional)
                {
                    auto* positionalArgument = static_cast<Positional*>(positionalArgumentIt->get());
                    positionalArgument->_value = std::move(arg);
                    positionalArgumentIt = std::find_if(++positionalArgumentIt, _arguments.end(), [](const auto& el)
                        { return el->Kind() == ArgumentKind::Positional || el->Kind() == ArgumentKind::PositionalList; });
                }
                else
                {
                    SILKIT_ASSERT((*positionalArgumentIt)->Kind() == ArgumentKind::PositionalList);
                    auto* positionalArgument = static_cast<PositionalList*>(positionalArgumentIt->get());
                    positionalArgument->_values.push_back(std::move(arg));
                }

                continue;
            }
            else
            {
                throw SilKitError("Bad argument '" + argument + "'");
            }

            auto splitPos = std::find(arg.begin(), arg.end(), '=');
            if (splitPos != arg.end())
            {
                std::string name = { arg.begin(), splitPos };
                std::string value = { splitPos + 1, arg.end() };

                auto* option = isShortForm ? GetByShortName<Option>(name) : GetByName<Option>(name);
                if (!option)
                {
                    throw SilKitError("Unknown argument '" + argument + "'");
                }
                option->_value = std::move(value);

                continue;
            }

            std::string name = arg;
            auto* option = isShortForm ? GetByShortName<Option>(name) : GetByName<Option>(name);
            if (option)
            {
                if (i + 1 >= argc)
                {
                    throw SilKitError("Argument '" + argument + "' without a value");
                }
                std::string value{ argv[++i] };
                option->_value = std::move(value);

                continue;
            }

            auto* flag = isShortForm ? GetByShortName<Flag>(name) : GetByName<Flag>(name);
            if (flag)
            {
                bool value = true;
                flag->_value = value;

                continue;
            }

            throw SilKitError("Unknown argument '" + argument + "'");
        }
    }

    enum { Hidden };

    enum class ArgumentKind { Positional, PositionalList, Option, Flag };

    struct IArgument
    {
        virtual ~IArgument() = default;

        virtual auto Kind() const -> ArgumentKind = 0;
        virtual auto Name() const -> std::string = 0;
        virtual auto Usage() const -> std::string = 0;
        virtual auto Description() const -> std::string = 0;
        virtual bool IsHidden() const = 0;
    };

    template<class Derived, class T>
    struct Argument 
        : public IArgument
    {
        Argument(std::string name, std::string usage, std::string description, bool hidden=false)
            : _name(std::move(name)), _usage(std::move(usage)), _description(std::move(description)), _hidden{hidden} {}

        auto Kind() const -> ArgumentKind override { return static_cast<const Derived*>(this)->kind; }
        auto Name() const -> std::string override { return _name; }
        auto Usage() const -> std::string override { return _usage; }
        auto Description() const -> std::string override { return _description; }
        bool IsHidden() const override { return _hidden; }

    private:
        std::string _name;
        std::string _usage;
        std::string _description;
        bool _hidden{false};
    };

    /*! \brief A positional argument, i.e. one without any prefix, processed in the added order
     *
     * Usage ("<value>") and description ("<value>: Explanation") are used by PrintVersionInfo.
     */
    struct Positional
        : public Argument<Positional, std::string>
    {
        friend class CommandlineParser;
        static constexpr auto kind = ArgumentKind::Positional;

        Positional(std::string name, std::string usage, std::string description)
            : Argument(std::move(name), std::move(usage), std::move(description)), _value() {}

        auto Value() const -> std::string { return _value; }
        auto HasValue() const -> bool { return !_value.empty(); }

    private:
        std::string _value;
    };

    /*! \brief A positional argument to collect multiple string values
     * 
     * It must be the last positional argument that is added, as it collects any remaining positional arguments.
     * Usage ("<value1> [value2 ...]") and description ("<value>, <value2>, ...: Explanation") are used by PrintVersionInfo.
     */
    struct PositionalList
        : public Argument<PositionalList, std::vector<std::string>>
    {
        friend class CommandlineParser;
        static constexpr auto kind = ArgumentKind::PositionalList;

        PositionalList(std::string name, std::string usage, std::string description)
            : Argument(std::move(name), std::move(usage), std::move(description)), _values() {}

        auto Values() const -> std::vector<std::string> { return _values; }
        auto HasValues() const -> bool { return !_values.empty(); }

    private:
        std::vector<std::string> _values;
    };

     /*! \brief A named argument with a string value
     * 
     * It supports a long prefix ("--" followed by name), and a short prefix ("-" followed by shortName), if not empty.
     * The actual value can be suffixed either after a "=" or a " ", i.e. a separate commandline argument.
     * Usage ("[--Name <value>]") and description ("-ShortName, --Name <value>: Explanation") are used by PrintVersionInfo.
     */
    struct Option
        : public Argument<Option, std::string>
    {
        friend class CommandlineParser;
        static constexpr auto kind = ArgumentKind::Option;

        Option(std::string name, std::string shortName, std::string defaultValue, std::string usage, std::string description)
            : Argument(std::move(name), std::move(usage), std::move(description)), _shortName(std::move(shortName)), _defaultValue(std::move(defaultValue)), _value() {}

        Option(std::string name, std::string shortName, std::string defaultValue, std::string usage, std::string description, decltype(Hidden))
            : Argument(std::move(name), std::move(usage), std::move(description), true), _shortName(std::move(shortName)), _defaultValue(std::move(defaultValue)), _value() {}

        auto ShortName() const -> std::string { return _shortName; }
        auto DefaultValue() const -> std::string { return _defaultValue; }
        auto Value() const -> std::string { return _value.empty() ? _defaultValue : _value; }
        auto HasValue() const -> bool { return !_value.empty(); }

    private:
        std::string _shortName;
        std::string _defaultValue;
        std::string _value;
    };

     /*! \brief A named argument representing a boolean value
     * 
     * It supports a long prefix ("--" followed by name), and a short prefix ("-" followed by shortName), if not empty.
     * Its value is false until the argument is used.
     * Usage ("[--Name]") and description ("-ShortName, --Name: Explanation") are used by PrintVersionInfo.
     */
    struct Flag
        : public Argument<Flag, bool>
    {
        friend class CommandlineParser;
        static constexpr auto kind = ArgumentKind::Flag;

        Flag(std::string name, std::string shortName, std::string usage, std::string description)
            : Argument(std::move(name), std::move(usage), std::move(description)), _shortName(std::move(shortName)), _value(false) {}

        Flag(std::string name, std::string shortName, std::string usage, std::string description, decltype(Hidden))
            : Argument(std::move(name), std::move(usage), std::move(description), true), _shortName(std::move(shortName)), _value(false) {}

        auto ShortName() const -> std::string { return _shortName; }
        auto DefaultValue() const -> bool { return false; }
        auto Value() const -> bool { return _value; }

    private:
        std::string _shortName;
        bool _value;
    };

private:
    template<class TArgument>
    auto GetByName(std::string name) -> TArgument*
    {
        auto it = std::find_if(_arguments.begin(), _arguments.end(), [&name](const auto& el) {
            return (el->Kind() == TArgument::kind && el->Name() == name);
        });
        return (it != _arguments.end() ? static_cast<TArgument*>(it->get()) : nullptr);
    }

    template<class TArgument>
    auto GetByShortName(std::string name) -> TArgument*
    {
        auto it = std::find_if(_arguments.begin(), _arguments.end(), [&name](const auto& el) { 
            return (el->Kind() == TArgument::kind && !static_cast<TArgument&>(*el).ShortName().empty() && static_cast<TArgument&>(*el).ShortName() == name);
        });
        return (it != _arguments.end() ? static_cast<TArgument*>(it->get()) : nullptr);
    }
private:
    std::vector<std::unique_ptr<IArgument>> _arguments;
};

} // namespace Util
} // namespace SilKit
