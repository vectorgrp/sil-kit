// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <limits>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <utility>
#include <algorithm>


namespace SilKit {
namespace Util {

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
        _maxLength =
            std::min<difference_type>(static_cast<difference_type>(length), _iterable.end() - _iterable.begin());
        return *this;
    }

    inline auto WithSeparator(std::string separator) -> PrintableHexString<IterableT>&
    {
        _separator = std::move(separator);
        return *this;
    }

    inline void to_ostream(std::ostream& out) const
    {
        if (_maxLength <= 0)
            return;

        const auto begin = _iterable.begin();
        const auto end = begin + _maxLength;

        //NB, we use a temporary stream because we don't want to modify the internal format state of out
        std::ostringstream oss;

        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint16_t>(*begin);

        if (_maxLength > 1)
        {
            std::for_each(begin + 1, end,
                          [&oss, this](auto chr) { oss << _separator << std::setw(2) << static_cast<uint16_t>(chr); });
        }
        if (_maxLength < _iterable.end() - _iterable.begin())
        {
            oss << _separator << "...";
        }
        out << oss.str();
    }

private:
    using difference_type = decltype(std::declval<IterableT>().end() - std::declval<IterableT>().begin());

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


} // namespace Util
} // namespace SilKit
