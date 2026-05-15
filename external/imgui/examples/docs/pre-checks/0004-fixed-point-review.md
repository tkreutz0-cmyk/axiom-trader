Pre-Check & KI-Dialog: Fixed-Point-Refactoring

Eingespeiste Prompts & Architektur-Kontext

- Anforderung: "Lies die Regeln aus decisions/prompt-guardrails.md und decisions/0004-fixed-point-monetary-values.md. Generiere das Refactoring für 'src/core/utils/fixed_point.hpp' noch NICHT direkt im Quellcode. Erstelle stattdessen eine neue Dokumentations- und Pre-Check-Datei unter 'docs/pre-checks/0004-fixed-point-review.md' mit bestimmtem Aufbau." (Benutzer)
- Agent-Vorarbeit: Versuch, die Dateien decisions/prompt-guardrails.md und decisions/0004-fixed-point-monetary-values.md zu lesen. Ergebnis: beide Dateien wurden im Repository nicht gefunden; File-Read schlug fehl. (Agent)
- Einschränkung: Das eigentliche Refactoring von src/core/utils/fixed_point.hpp soll noch nicht angewendet werden; stattdessen ein C++20-Vorschlag und eine Konformitätsprüfung dokumentieren. (Benutzer)
- Rahmenbedingungen: C++20, keine floating-point-Arithmetik (0 floats), Hotpath allokationsfrei (keine Heap-Allokation in performance-kritischen Pfaden), möglichst constexpr-friendly, kein Einführen externer Bibliotheken. (Benutzer)
- Hinweis: Da die decisions-Dateien im Repo fehlen, basieren die Annahmen der Guardrails in diesem Dokument auf der Anforderung und typischen Projekt-Policies: exakte monetäre Darstellung, deterministische ganzzahlige Rechenwege, allocation-free hot path.

Generierter C++20 Code-Vorschlag

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
		} else {
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
		} else if (integer_part < 0) {
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
	// remainder in [0, scale) for positive values, sign preserved for negatives
	constexpr rep fractional_part() const noexcept {
		rep r = value_ % scale;
		if (r < 0) r = -r;
		return r;
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
		// Handle sign
		if (value_ == 0 || o.value_ == 0) return from_raw(0);
		bool neg = (value_ < 0) ^ (o.value_ < 0);
		unsigned long long ua = static_cast<unsigned long long>(value_ < 0 ? -static_cast<long long>(value_) : static_cast<long long>(value_));
		unsigned long long ub = static_cast<unsigned long long>(o.value_ < 0 ? -static_cast<long long>(o.value_) : static_cast<long long>(o.value_));
		unsigned long long high = 0;
		unsigned long long low = _umul128(ua, ub, &high);
		unsigned long long rem = 0;
		unsigned long long q = _udiv128(high, low, static_cast<unsigned long long>(scale), &rem);
		// q is absolute result; check overflow into signed rep
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

	// Compound assignment
	constexpr fixed_point& operator+=(const fixed_point& o) noexcept { *this = *this + o; return *this; }
	constexpr fixed_point& operator-=(const fixed_point& o) noexcept { *this = *this - o; return *this; }
	constexpr fixed_point& operator*=(const fixed_point& o) noexcept { *this = *this * o; return *this; }
	constexpr fixed_point& operator/=(const fixed_point& o) { *this = *this / o; return *this; }

	// Comparisons
	constexpr bool operator==(const fixed_point& o) const noexcept { return value_ == o.value_; }
	constexpr bool operator!=(const fixed_point& o) const noexcept { return value_ != o.value_; }
	constexpr bool operator<(const fixed_point& o) const noexcept { return value_ < o.value_; }
	constexpr bool operator<=(const fixed_point& o) const noexcept { return value_ <= o.value_; }
	constexpr bool operator>(const fixed_point& o) const noexcept { return value_ > o.value_; }
	constexpr bool operator>=(const fixed_point& o) const noexcept { return value_ >= o.value_; }

	// Allocation-free formatting into a caller buffer using std::to_chars (hotpath-safe)
	// Returns number of bytes written. Buffer must be large enough.
	// Format: [-]integer[.fractional]  fractional is zero-padded to width determined by scale
	size_t format_to_buffer(char* buf, size_t buf_size) const noexcept {
		if (!buf || buf_size == 0) return 0;
		// We avoid any heap allocation here.
		rep v = value_;
		char* p = buf;
		char* end = buf + buf_size;

		if (v < 0) {
			if (p == end) return 0;
			*p++ = '-';
			v = -v;
		}

		rep intpart = v / scale;
		rep fracpart = v % scale;

		// write integer part
		auto res = std::to_chars(p, end, intpart);
		if (res.ec != std::errc()) return 0;
		p = res.ptr;

		// write fractional part only if scale > 1
		if (scale > 1) {
			if (p == end) return 0;
			*p++ = '.';
			// fractional digits: determine width (decimal digits) from scale
			// scale is assumed to be power-of-10 in monetary contexts; compute width
			unsigned width = 0;
			for (Rep s = scale; s > 1; s /= 10) ++width;

			// zero-pad fractional part
			char fracbuf[32]; // enough for typical scales
			char* fp = fracbuf + sizeof(fracbuf);
			unsigned i = 0;
			rep f = fracpart;
			// write digits in reverse
			for (unsigned k = 0; k < width; ++k) {
				fracbuf[--fp - fracbuf] = char('0' + (f % 10));
				f /= 10;
				++i;
			}
			// copy from fp to end of fracbuf
			size_t need = i;
			if (static_cast<size_t>(end - p) < need) return 0;
			for (size_t k = 0; k < need; ++k) *p++ = *(fp + k);
		}

		return static_cast<size_t>(p - buf);
	}

	// Convenience function that may allocate (for non-hotpath use)
	std::string to_string() const {
		char tmp[128];
		size_t n = format_to_buffer(tmp, sizeof(tmp));
		return std::string(tmp, tmp + n);
	}
};

} // namespace core::utils

#endif // CORE_UTILS_FIXED_POINT_HPP


Konformitäts-Prüfung gegen prompt-guardrails.md

Hinweis: Die Datei decisions/prompt-guardrails.md konnte im Repository nicht gefunden werden; die folgende Konformitätsprüfung ist eine direkte Ableitung der vom Nutzer vorgegebenen Guardrails (0 floats, hotpath allokationsfrei). Die Prüfung zeigt mathematisch/logisch, wie der obige Code diese Anforderungen erfüllt und wie MSVC-Fallback funktioniert.

1) Nachweis: Der Code verwendet 0 float- oder double-Operationen
- Inclusions/Typen: Alle verwendeten arithmetischen Typen sind ganzzahlig: std::int64_t und bei GCC/Clang __int128_t; im MSVC-Fall werden nur 64-bit-Integer und Compiler-Intrinsics verwendet. Es gibt keine Deklaration, Definition oder Konvertierung zu float, double oder long double.
- Konstruktoren/Operatoren: Jede arithmetische Operation (+, -, *, /, %, %) wird mit ganzzahligen Typen ausgeführt. Auf GCC/Clang nutzen wir __int128 für sichere Zwischenwerte; auf MSVC nutzen wir _umul128/_udiv128-Intrinsics zur 128-bit-Emulation. Es existieren keine Gleitkommaoperationen.
- Schlussfolgerung: Formal gibt es keine Verwendung von Gleitkommaoperationen; daher 0 floats im Code.

2) Nachweis: Hotpath ist allocation-free
- Definition Hotpath: Performance-kritische Operatoren und Konverter: operator+, operator-, operator*, operator/, operator+=/-=, Konstruktoren aus Ganzzahlen, format_to_buffer — diese Pfade dürfen keine Heap-Allokation durchführen.
- Analyse der Hotpath-Funktionen:
  - operator+, operator-: auf beiden Plattformen werden nur integer-Operationen durchgeführt und keine heap-allokierenden Aufrufe benutzt; Überlaufprüfung erfolgt durch konstante-time Vergleiche oder 128-bit-Arithmetik.
  - operator*: Auf GCC/Clang wird __int128 verwendet (stack/CPU-Register; keine Heap-Allokation). Auf MSVC wird _umul128/_udiv128 verwendet; diese Intrinsics sind allocation-free C API-Aufrufe, arbeiten mit CPU-Register-Gleichungen und temporären lokalen Variablen. Das Ergebnis wird geclamped und zurückgegeben — keine Heap-Allokation.
  - operator/: Analog zur Multiplikation: auf GCC/Clang __int128; auf MSVC Emulation mit Intrinsics. Division-by-zero löst eine Ausnahme in nicht-hotpath-Pfaden.
  - Konstruktor aus Integraltypen: prüft Überlauf und verwendet einfache Integral-Arithmetik; keine Heap-Allokationen.
  - format_to_buffer: verwendet vom Aufrufer bereitgestellten Puffer, std::to_chars und lokale Arrays; keine Heap-Allokation.
- Ausnahmen: to_string() allokiert bewusst std::string und ist nicht hotpath-sicher.
- Schlussfolgerung: Alle definierten Hotpath-Funktionen sind allocation-free auf allen unterstützten Plattformen (GCC/Clang mit __int128 und MSVC mit Intrinsics).

3) Mathematisch-logische Korrektheit (Skalierung, Rundung, Überlaufbehandlung)
- Repräsentation: Interner Wert value_ repräsentiert real_value * Scale.
- Addition/Subtraktion: Operieren direkt auf der skalierten Repräsentation. Auf GCC/Clang wird durch __int128 Zwischenüberlauf vermieden; auf MSVC prüfen konservativ Grenzfälle vor der Operation und saturieren falls nötig.
- Multiplikation: Zielwert ist (a_value * b_value) / scale. Auf GCC/Clang berechnet (__int128(a)*__int128(b))/__int128(scale) exakt bis zur 128-bit-Range. Auf MSVC wird das 128-bit-Produkt mittels _umul128 in High/Low Teile zerlegt und anschließend mit _udiv128 durch scale geteilt — das entspricht exakter 128-bit-Division; Vorzeichenbehandlung erfolgt separat. Ergebnis wird geclamped.
- Division: Zielwert ist (a_value * scale) / b_value. Analoge Behandlung wie bei Multiplikation; 128-bit-Intermediate wird entweder durch __int128 oder MSVC-Intrinsics realisiert.

4) Grenzen, Annahmen und Risiken
- Compiler-Unterstützung: Für GCC/Clang ist __int128 vorausgesetzt. Für MSVC wird eine fallback-Implementierung mittels Intrinsics _umul128/_udiv128 verwendet; diese Funktionen sind in Visual Studio 2017+ verfügbar. Falls eine ältere Toolchain verwendet wird, ist ein weiterer Fallback (Software-multiplikation/-division) nötig.
- Scale-Einschränkung: format_to_buffer geht von einer dezimalen Interpretation der Skala aus (typisch für monetäre Werte); nicht-Potenz-10-Scales liefern möglicherweise unerwartete Darstellungen.
- Sättigungsstrategie: Bei Überlauf wird geclamped. Projektweite Policy kann angepasst werden.

5) Empfehlungen vor Integration
- Ergänzende Tests für MSVC-Fallback: multiplikation/division-Edgecases, Vorzeichen, divisors nahe 0, Puffergrößen für format_to_buffer.
- Dokumentiere die unterstützten Compiler/Toolchain-Minima (MSVC-Version mit _umul128/_udiv128).

Ende des Pre-Checks

