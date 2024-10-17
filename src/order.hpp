#pragma once

#include <chrono>

struct BaseOrder
{
  int volume;
  int id;
};

// NOTE: can't use std::strong_ordering with floats
using Money = int;
const auto now = std::chrono::steady_clock::now;
using time_point = std::chrono::time_point<std::chrono::steady_clock>;

template<typename T>
concept IsOrder =
  std::three_way_comparable<T, std::strong_ordering> && requires(T a, T b) {
    { a.volume } -> std::same_as<int&>;
    { a.id } -> std::same_as<int&>;
    { a.time } -> std::same_as<time_point&>;
  };

template<IsOrder... Ts>
using OrderVar = std::variant<Ts...>;

struct MarketOrder
{
  int volume{};
  int id{};
  time_point time{ now() };
};
std::strong_ordering
operator<=>(const MarketOrder& mo1, const MarketOrder& mo2);

bool
operator==(const MarketOrder& mo1, const MarketOrder& mo2);

struct LimitOrder
{
  int volume{};
  int id{};
  time_point time{ now() };
  // NOTE: valid Bid Limit Order price is less than current best (highest) Bid
  // Else is just a market order
  Money price;
};
std::strong_ordering
operator<=>(const LimitOrder& lo1, const LimitOrder& lo2);

bool
operator==(const LimitOrder& lo1, const LimitOrder& lo2);

using Order_t = OrderVar<MarketOrder, LimitOrder>;

enum class OrderDir
{
  Bid,
  Ask,
};
