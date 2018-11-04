#pragma once

#include "rational.hpp"

namespace stan {

struct duration : rational<std::uint32_t>
{
    using rational<std::uint32_t>::rational;

    duration operator+(duration const &d2) const
    {
        // https://www.geeksforgeeks.org/program-to-add-two-fractions
        assert(d2.den() > 0);
        integer gcd = compute_gcd(den(), d2.den());
        integer d = (den() * d2.den()) / gcd;
        integer n = (num() * d) / den() + (d2.num() * d) / d2.den();
        return { n, d };
    }

    static duration zero() { return duration(0, 1); }

    friend struct value;
    friend struct tuplet;

    friend duration operator*(int, duration const &);
};

} // namespace stan
