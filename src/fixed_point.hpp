#pragma once

#include <algorithm>
#include <cmath>
#include <iostream>

#include <fmt/core.h>

#include "serializable.hpp"

namespace leyval {
template<int ScaleExp> // Scale Exponent for Base 10
struct Fixed
{
  Fixed(int val)
    : underlying_value{ val } {};
  long underlying_value;

  explicit operator float() const
  {
    return underlying_value * std::pow(10, ScaleExp);
  }

  template<int NewScaleExp>
  Fixed<NewScaleExp> rescale() const
  {
    return Fixed<NewScaleExp>(underlying_value *
                              std::pow(10, ScaleExp - NewScaleExp));
  }

  friend inline void to_json(nlohmann::json& j, const Fixed<ScaleExp>& f)
  {
    j = nlohmann::json{ static_cast<float>(f) };
    static_assert(Serializable<Fixed<ScaleExp>>);
  }
};

template<int N>
bool
operator==(const Fixed<N>& a, const Fixed<N>& b)
{
  return a.underlying_value == b.underlying_value;
}

template<int N, int M>
bool
operator==(const Fixed<N>& a, const Fixed<M>& b)
{
  return a == (b.template rescale<N>());
}

template<int N>
std::strong_ordering
operator<=>(const Fixed<N>& f1, const Fixed<N>& f2)
{
  return f1.underlying_value <=> f2.underlying_value;
}

template<int N, int M>
std::strong_ordering
operator<=>(const Fixed<N>& f1, const Fixed<M>& f2)
{
  return f1.underlying_value <=> ((f2.template rescale<N>()).underlying_value);
}

template<int N>
Fixed<N>
operator+(const Fixed<N>& a, const Fixed<N>& b)
{
  // TODO: I don't know if long -> int is always safe in this type
  return { static_cast<int>(a.underlying_value) +
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::min(N, M)>
operator+(const Fixed<N>& a, const Fixed<M>& b)
{
  return (a.template rescale<std::min(N, M)>()) +
         (b.template rescale<std::min(N, M)>());
}

template<int N>
Fixed<N>&
operator+=(Fixed<N>& a, const Fixed<N>& b)
{
  a = a + b;
  return a;
}

template<int N, int M>
Fixed<N>&
operator+=(Fixed<N>& a, const Fixed<M>& b)
{
  a = a + b;
  return a.template rescale<N>();
}

template<int N>
Fixed<N>
operator-(const Fixed<N>& a, const Fixed<N>& b)
{
  return { static_cast<int>(a.underlying_value) -
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::min(N, M)>
operator-(const Fixed<N>& a, const Fixed<M>& b)
{
  return (a.template rescale<std::min(N, M)>()) -
         (b.template rescale<std::min(N, M)>());
}

template<int N>
Fixed<N>&
operator-=(Fixed<N>& a, const Fixed<N>& b)
{
  a = a - b;
  return a;
}

template<int N, int M>
Fixed<N>&
operator-=(Fixed<N>& a, const Fixed<M>& b)
{
  a = a - b;
  return a.template rescale<N>();
}

template<int N>
Fixed<N>
operator*(const Fixed<N>& a, const Fixed<N>& b)
{

  return { static_cast<int>(a.underlying_value) *
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::max(N, M)>
operator*(const Fixed<N>& a, const Fixed<M>& b)
{
  constexpr auto mm{ std::minmax({ N, M }) };
  return ((a.template rescale<mm.first>()) * (b.template rescale<mm.first>()))
    .template rescale<mm.second>();
}

template<int N>
Fixed<N>
operator/(const Fixed<N>& a, const Fixed<N>& b)
{

  return { static_cast<int>(a.underlying_value) /
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::max(N, M)>
operator/(const Fixed<N>& a, const Fixed<M>& b)
{
  constexpr auto mm{ std::minmax({ N, M }) };
  return ((a.template rescale<mm.first>()) / (b.template rescale<mm.first>()))
    .template rescale<mm.second>();
}

// Only for Catch2 printing
template<int N>
std::ostream&
operator<<(std::ostream& os, const Fixed<N>& f)
{
  os << "Fixed<" << N << ">(" << f.underlying_value << ")";
  return os;
}

} // namespace leyval

template<int N>
struct fmt::formatter<leyval::Fixed<N>> : fmt::formatter<std::string_view>
{
  auto format(const leyval::Fixed<N>& f,
              format_context& ctx) const -> format_context::iterator
  {
    return fmt::format_to(ctx.out(), "Fixed<{}>({})", N, f.underlying_value);
  }
};
