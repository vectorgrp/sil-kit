// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


namespace ib {
namespace cfg {

//! \brief super simple drop in replacement for std::optional
//!
//!  NB: replace with std::optional once we moved to C++17
template<class ConfigT>
class OptionalCfg
{
public:
    // ----------------------------------------
    // Public Data Types
    using value_type = ConfigT;

public:
    // ----------------------------------------
    // Constructors and Destructor
    OptionalCfg() = default;
    OptionalCfg(const OptionalCfg&) = default;
    OptionalCfg(OptionalCfg&&) = default;
    OptionalCfg(const ConfigT& cfg) : _config{cfg}, _has_value{true} {}
    OptionalCfg(ConfigT&& cfg) : _config{std::move(cfg)}, _has_value{true} {}
                
public:
    // ----------------------------------------
    // Operator Implementations
    OptionalCfg& operator=(const OptionalCfg& other) = default;
    OptionalCfg& operator=(OptionalCfg&& other) = default;

public:
    // ----------------------------------------
    // Public Methods
    //
    auto operator=(const ConfigT& cfg) { _config = cfg; _has_value = true; }
    auto operator=(ConfigT&& cfg) { _config = std::move(cfg); _has_value = true; }
    
    auto operator->() const -> const ConfigT* { return &_config; };
    auto operator->() -> ConfigT* { return &_config; };
    auto operator*() const -> const ConfigT& { return _config; };
    auto operator*() -> ConfigT& { return _config; };

    operator bool() const noexcept { return _has_value; }
    auto has_value() const noexcept -> bool { return _has_value; }

    auto value() const -> const ConfigT& { return _config; };
    auto value() -> ConfigT& { return _config; };

private:
    // ----------------------------------------
    // private members
    ConfigT _config;
    bool _has_value={false};
};

template<class ConfigT>
bool operator==(const OptionalCfg<ConfigT>& lhs, const OptionalCfg<ConfigT>& rhs)
{
    if (!lhs.has_value() || !rhs.has_value())
        return lhs.has_value() == rhs.has_value();

    return lhs.has_value() == rhs.has_value()
        && lhs.value() == rhs.value();
}

} // namespace cfg
} // namespace ib
