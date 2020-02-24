// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>
#include <limits>
#include <iomanip>
#include <ostream>
#include <utility>


namespace ib {
namespace util {

template <typename IterableT>
class PrintableHexString
{
public:
    inline PrintableHexString(const IterableT& iterable)
        : _iterable{iterable}
        , _maxLength{iterable.end() - iterable.begin()}
    {
    }

    inline auto WithMaxLength(int64_t length) -> PrintableHexString<IterableT>&
    {
        _maxLength = std::min<int64_t>(length, static_cast<int64_t>(_iterable.end() - _iterable.begin()));
        return *this;
    }

    inline auto WithSeparator(std::string separator) -> PrintableHexString<IterableT>&
    {
        _separator = std::move(separator);
        return *this;
    }

    inline void to_ostream(std::ostream& out) const
    {
        assert(_maxLength <= _iterable.end() - _iterable.begin());

        if (_maxLength <= 0)
            return;

        const auto begin = _iterable.begin();
        const auto end = begin + _maxLength;

        std::ios oldState(nullptr);
        oldState.copyfmt(out);

        out << std::hex << std::setfill('0')
            << std::setw(2) << static_cast<uint16_t>(*begin);

        if (_maxLength > 1)
        {
            std::for_each(begin + 1,
                end,
                [&out, this](auto chr)
                {
                    out << _separator << std::setw(2) << static_cast<uint16_t>(chr);
                }
            );
        }
        if (_maxLength < _iterable.end() - _iterable.begin())
        {
            out << _separator << "...";
        }

        out.copyfmt(oldState);
    }

private:
    using difference_type = decltype(std::declval<IterableT>().end()
            - std::declval<IterableT>().begin());

    const IterableT& _iterable;
    difference_type _maxLength = (std::numeric_limits<difference_type>::max)();
    std::string _separator = "";
};

template <typename IterableT>
inline auto AsHexString(const IterableT& iterable) -> PrintableHexString<IterableT>
{
    return PrintableHexString<IterableT>(iterable);
}

template <typename IterableT>
inline std::ostream& operator<<(std::ostream& out, const PrintableHexString<IterableT>& hexString)
{
    hexString.to_ostream(out);
    return out;
}


} // namespace util
} // namespace ib
