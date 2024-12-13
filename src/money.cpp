#include "money.hpp"

namespace leyval {
Money
operator+(const Money& m1, const Money& m2)
{
  std::div_t sub{ std::div(m1.get_sub() + m2.get_sub(), 100) };
  return Money{ m1.get_main() + m2.get_main() + sub.rem, sub.quot };
}
}
