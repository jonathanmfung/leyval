#include <algorithm>
#include <cmath>
#include <iostream>

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
};

template<int N>
Fixed<N>
operator+(Fixed<N> a, Fixed<N> b)
{
  // TODO: I don't know if long -> int is always safe in this type
  return { static_cast<int>(a.underlying_value) +
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::min(N, M)>
operator+(Fixed<N> a, Fixed<M> b)
{
  return (a.template rescale<std::min(N, M)>()) +
         (b.template rescale<std::min(N, M)>());
}

template<int N>
Fixed<N>
operator-(Fixed<N> a, Fixed<N> b)
{
  return { static_cast<int>(a.underlying_value) -
           static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::min(N, M)>
operator-(Fixed<N> a, Fixed<M> b)
{
  return (a.template rescale<std::min(N, M)>()) -
         (b.template rescale<std::min(N, M)>());
}

template<int N>
bool
operator==(Fixed<N> a, Fixed<N> b)
{
  return a.underlying_value == b.underlying_value;
}

template<int N, int M>
bool
operator==(Fixed<N> a, Fixed<M> b)
{
  return a == (b.template rescale<N>());
}

template<int N>
Fixed<N>
operator*(Fixed<N> a, Fixed<N> b)
{

  return Fixed<N>{ static_cast<int>(a.underlying_value) *
                   static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::max(N, M)>
operator*(Fixed<N> a, Fixed<M> b)
{
  constexpr auto mm{ std::minmax({ N, M }) };
  return ((a.template rescale<mm.first>()) * (b.template rescale<mm.first>()))
    .template rescale<mm.second>();
}

template<int N>
Fixed<N>
operator/(Fixed<N> a, Fixed<N> b)
{

  return Fixed<N>{ static_cast<int>(a.underlying_value) /
                   static_cast<int>(b.underlying_value) };
}

template<int N, int M>
Fixed<std::max(N, M)>
operator/(Fixed<N> a, Fixed<M> b)
{
  constexpr auto mm{ std::minmax({ N, M }) };
  return ((a.template rescale<mm.first>()) / (b.template rescale<mm.first>()))
    .template rescale<mm.second>();
}

// Only for Catch2 printing
template<int N>
std::ostream&
operator<<(std::ostream& os, Fixed<N> const& f)
{
  os << "Fixed<" << N << ">(" << f.underlying_value << ")";
  return os;
}

} // namespace leyval
