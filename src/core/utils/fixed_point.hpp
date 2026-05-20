// File: src/core/utils/fixed_point.hpp

#ifndef CORE_UTILS_FIXED_POINT_HPP
#define CORE_UTILS_FIXED_POINT_HPP

#include <cstdint>
#include <type_traits>
#include <limits>
#include <charconv>
#include <array>
#include <string>
#include <stdexcept>
#include <cassert>
#include <cmath>

// MSVC specific intrinsics used only in the fallback branch
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace core::utils {

    // fixed_point<Rep, Scale>
    // - Rep: ganzzahliger Repräsentationstyp (z. B. int64_t)
    // - Scale: positive Skala als ganzzahliger Faktor (z. B. 100 für Cent)
    // Anforderungen erfüllt: keine Verwendung von float/double; Hotpath-Operatoren sind allocation-free.

    template<typename Rep = std::int64_t, Rep Scale = 100>
    class fixed_point {
        static_assert(std::is_integral_v<Rep>, "Rep must be an integral type");
        static_assert(Scale > 0, "Scale must be positive");

    public:
        using rep = Rep;
        static constexpr Rep scale = Scale;

    private:
        rep value_; // internal stored value = real * Scale

        static constexpr rep rep_min() noexcept { return std::numeric_limits<rep>::min(); }
        static constexpr rep rep_max() noexcept { return std::numeric_limits<rep>::max(); }

    public:
        constexpr fixed_point() noexcept : value_(0) {}

        // construct from raw stored value (explicit to avoid accidental misuse)
        static constexpr fixed_point from_raw(rep raw) noexcept {
            fixed_point r; r.value_ = raw; return r;
        }

        // Construct from integer types (no floating conversions)
        template<typename Int, typename = std::enable_if_t<std::is_integral_v<Int>>>
        constexpr explicit fixed_point(Int iv) noexcept {
#if defined(__SIZEOF_INT128__)
            __int128 tmp = __int128(iv) * __int128(scale);
            if (tmp < __int128(rep_min())) value_ = rep_min();
            else if (tmp > __int128(rep_max())) value_ = rep_max();
            else value_ = static_cast<rep>(tmp);
#else
            // MSVC fallback: avoid __int128. scale is positive.
            if (iv >= 0) {
                if (static_cast<std::int64_t>(iv) > rep_max() / static_cast<std::int64_t>(scale)) value_ = rep_max();
                else value_ = static_cast<rep>(iv * static_cast<Int>(scale));
            }
            else {
                if (static_cast<std::int64_t>(iv) < rep_min() / static_cast<std::int64_t>(scale)) value_ = rep_min();
                else value_ = static_cast<rep>(iv * static_cast<Int>(scale));
            }
#endif
        }

        // Construct from integer part and fractional digits (all integral, no floats)
        // frac must be non-negative and less than scale
        static constexpr fixed_point from_parts(rep integer_part, rep frac) noexcept {
#if defined(__SIZEOF_INT128__)
            __int128 tmp = __int128(integer_part) * __int128(scale) + __int128(frac);
            if (tmp < __int128(rep_min())) return from_raw(rep_min());
            if (tmp > __int128(rep_max())) return from_raw(rep_max());
            return from_raw(static_cast<rep>(tmp));
#else
            // Simple checked arithmetic: use division to detect overflow conservatively
            if (integer_part > 0) {
                if (integer_part > rep_max() / scale) return from_raw(rep_max());
            }
            else if (integer_part < 0) {
                if (integer_part < rep_min() / scale) return from_raw(rep_min());
            }
            // now multiplication won't overflow
            rep raw = static_cast<rep>(integer_part * scale) + frac;
            if (raw < rep_min()) return from_raw(rep_min());
            if (raw > rep_max()) return from_raw(rep_max());
            return from_raw(raw);
#endif
        }

        constexpr rep raw() const noexcept { return value_; }

        // conversions
        constexpr rep to_integral() const noexcept { return value_ / scale; }
        
        // Remainder in [0, scale) for positive values, sign preserved for negatives
        constexpr rep fractional_part() const noexcept {
            rep r = value_ % scale;
            if (r < 0) r = -r;
            return r;
        }

        // Explizite Konvertierung an der UI Boundary (erlaubt laut ADR-0004)
        [[nodiscard]] constexpr double to_double() const noexcept {
            return static_cast<double>(value_) / static_cast<double>(scale);
        }

        // Unary operators
        constexpr fixed_point operator+() const noexcept { return *this; }
        constexpr fixed_point operator-() const noexcept {
            // handle negation safely
            if (value_ == rep_min()) return from_raw(rep_max()); // saturate on overflow
            return from_raw(static_cast<rep>(-value_));
        }

        // Addition / Subtraction (allocation-free, integer math only)
        constexpr fixed_point operator+(const fixed_point& o) const noexcept {
#if defined(__SIZEOF_INT128__)
            __int128 tmp = __int128(value_) + __int128(o.value_);
            if (tmp < __int128(rep_min())) return from_raw(rep_min());
            if (tmp > __int128(rep_max())) return from_raw(rep_max());
            return from_raw(static_cast<rep>(tmp));
#else
            if (o.value_ > 0 && value_ > rep_max() - o.value_) return from_raw(rep_max());
            if (o.value_ < 0 && value_ < rep_min() - o.value_) return from_raw(rep_min());
            return from_raw(static_cast<rep>(value_ + o.value_));
#endif
        }

        constexpr fixed_point operator-(const fixed_point& o) const noexcept {
#if defined(__SIZEOF_INT128__)
            __int128 tmp = __int128(value_) - __int128(o.value_);
            if (tmp < __int128(rep_min())) return from_raw(rep_min());
            if (tmp > __int128(rep_max())) return from_raw(rep_max());
            return from_raw(static_cast<rep>(tmp));
#else
            if (o.value_ < 0 && value_ > rep_max() + o.value_) return from_raw(rep_max());
            if (o.value_ > 0 && value_ < rep_min() + o.value_) return from_raw(rep_min());
            return from_raw(static_cast<rep>(value_ - o.value_));
#endif
        }

        // Multiplication: (a * b) / scale.
        // Use compiler 128-bit when available; on MSVC use intrinsics to perform 128-bit emulation
        constexpr fixed_point operator*(const fixed_point& o) const noexcept {
#if defined(__SIZEOF_INT128__)
            __int128 prod = __int128(value_) * __int128(o.value_);
            __int128 scaled = prod / __int128(scale);
            if (scaled < __int128(rep_min())) return from_raw(rep_min());
            if (scaled > __int128(rep_max())) return from_raw(rep_max());
            return from_raw(static_cast<rep>(scaled));
#else
            // MSVC fallback: use unsigned 128-bit emulation via intrinsics
            if (value_ == 0 || o.value_ == 0) return from_raw(0);
            bool neg = (value_ < 0) ^ (o.value_ < 0);
            unsigned long long ua = static_cast<unsigned long long>(value_ < 0 ? -static_cast<long long>(value_) : static_cast<long long>(value_));
            unsigned long long ub = static_cast<unsigned long long>(o.value_ < 0 ? -static_cast<long long>(o.value_) : static_cast<long long>(o.value_));
            unsigned long long high = 0;
            unsigned long long low = _umul128(ua, ub, &high);
            unsigned long long rem = 0;
            unsigned long long q = _udiv128(high, low, static_cast<unsigned long long>(scale), &rem);
            if (q > static_cast<unsigned long long>(rep_max())) return from_raw(neg ? rep_min() : rep_max());
            rep r = static_cast<rep>(q);
            return from_raw(neg ? -r : r);
#endif
        }

        // Division: (a * scale) / b  (b must not be zero)
        constexpr fixed_point operator/(const fixed_point& o) const {
            if (o.value_ == 0) throw std::domain_error("division by zero in fixed_point");
#if defined(__SIZEOF_INT128__)
            __int128 numer = __int128(value_) * __int128(scale);
            __int128 q = numer / __int128(o.value_);
            if (q < __int128(rep_min())) return from_raw(rep_min());
            if (q > __int128(rep_max())) return from_raw(rep_max());
            return from_raw(static_cast<rep>(q));
#else
            // MSVC fallback: compute abs(numer) as 128-bit (emulated) then divide by abs(denom)
            bool neg = (value_ < 0) ^ (o.value_ < 0);
            unsigned long long ua = static_cast<unsigned long long>(value_ < 0 ? -static_cast<long long>(value_) : static_cast<long long>(value_));
            unsigned long long ascale = static_cast<unsigned long long>(scale);
            unsigned long long high = 0;
            unsigned long long low = _umul128(ua, ascale, &high);
            unsigned long long quo = 0;
            unsigned long long rem = 0;
            unsigned long long udiv = static_cast<unsigned long long>(o.value_ < 0 ? -static_cast<long long>(o.value_) : static_cast<long long>(o.value_));
            quo = _udiv128(high, low, udiv, &rem);
            if (quo > static_cast<unsigned long long>(rep_max())) return from_raw(neg ? rep_min() : rep_max());
            rep r = static_cast<rep>(quo);
            return from_raw(neg ? -r : r);
#endif
        }

        // Compound assignment operators
        constexpr fixed_point& operator+=(const fixed_point& o) noexcept { *this = *this + o; return *this; }
        constexpr fixed_point& operator-=(const fixed_point& o) noexcept { *this = *this - o; return *this; }
        constexpr fixed_point& operator*=(const fixed_point& o) noexcept { *this = *this * o; return *this; }
        constexpr fixed_point& operator/=(const fixed_point& o) { *this = *this / o; return *this; }

        // Comparison operators
        constexpr bool operator==(const fixed_point& o) const noexcept { return value_ == o.value_; }
        constexpr bool operator!=(const fixed_point& o) const noexcept { return value_ != o.value_; }
        constexpr bool operator< (const fixed_point& o) const noexcept { return value_ <  o.value_; }
        constexpr bool operator<=(const fixed_point& o) const noexcept { return value_ <= o.value_; }
        constexpr bool operator> (const fixed_point& o) const noexcept { return value_ >  o.value_; }
        constexpr bool operator>=(const fixed_point& o) const noexcept { return value_ >= o.value_; }
    };

    // Globaler Alias-Typ laut Anweisung des Senior Architects (Abschnitt 3.0.1)
    using Money = fixed_point<std::int64_t, 10000>;

} // namespace core::utils

#endif // CORE_UTILS_FIXED_POINT_HPP

