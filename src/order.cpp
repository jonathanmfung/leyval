#include "order.hpp"

std::strong_ordering
operator<=>(const MarketOrder& mo1, const MarketOrder& mo2)
{
  return mo1.time <=> mo2.time;
}

bool
operator==(const MarketOrder& mo1, const MarketOrder& mo2)
{
  return mo1.id == mo2.id;
}

static_assert(IsOrder<MarketOrder>);

std::strong_ordering
operator<=>(const LimitOrder& lo1, const LimitOrder& lo2)
{
  return std::tie(lo1.time, lo1.price) <=> std::tie(lo2.time, lo2.price);
}

bool
operator==(const LimitOrder& lo1, const LimitOrder& lo2)
{
  return std::tie(lo1.time, lo1.price) == std::tie(lo2.time, lo2.price);
  // return lo1.id == lo2.id;
}

static_assert(IsOrder<LimitOrder>);
