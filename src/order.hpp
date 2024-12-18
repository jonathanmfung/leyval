#pragma once

#include <chrono>

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/std.h>

#include "fixed_point.hpp"

namespace leyval {
// NOTE: can't use std::strong_ordering with floats
// Money{3} is 3 cents
// Money{500} is 5 dollars
using Money = Fixed<-2>;
const auto now = std::chrono::steady_clock::now;
using time_point = std::chrono::time_point<std::chrono::steady_clock>;

static const time_point INIT_TS{ now() };

enum class OrderDir
{
  Bid,
  Ask,
};

// AKA Contra-side of a transaction
OrderDir
operator!(OrderDir order_dir);

class OrderDirInvalidValue : public std::invalid_argument
{
public:
  explicit OrderDirInvalidValue(const std::string& what_arg)
    : std::invalid_argument(what_arg)
  {
  }
};
}

template<>
struct fmt::formatter<leyval::OrderDir> : fmt::formatter<std::string_view>
{
  auto format(const leyval::OrderDir& od,
              format_context& ctx) const -> format_context::iterator;
};

///////////////////
namespace leyval {
template<typename T>
concept IsOrderReq =
  std::three_way_comparable<T, std::strong_ordering> && requires(T a, T b) {
    { a.volume } -> std::same_as<int&>;
    { a.agent_id } -> std::same_as<int&>;
    { a.timestamp } -> std::same_as<time_point&>;
    { a.order_dir } -> std::same_as<OrderDir&>;
  };

template<IsOrderReq... Ts>
using OrderReqVar = std::variant<Ts...>;

struct MarketOrderReq
{
  int volume{};
  int agent_id{};
  OrderDir order_dir{};
  time_point timestamp{ now() };
};
std::strong_ordering
operator<=>(const MarketOrderReq& mor1, const MarketOrderReq& mor2);

bool
operator==(const MarketOrderReq& mor1, const MarketOrderReq& mor2);
}
template<>
struct fmt::formatter<leyval::MarketOrderReq> : fmt::formatter<std::string_view>
{
  auto format(const leyval::MarketOrderReq& mor,
              format_context& ctx) const -> format_context::iterator;
};

////////////////////////////////////////////////////////////////////////////////
namespace leyval {
// Helper method for second in Ask/Bid Container
struct LimitOrderVal
{
  int volume{};
  int agent_id{};
  time_point timestamp{ now() };
};
std::strong_ordering
operator<=>(const LimitOrderVal& lov1, const LimitOrderVal& lov2);

using LimitOrder = std::pair<Money, LimitOrderVal>;

struct LimitOrderReq
{
  int volume{};
  int agent_id{};
  // NOTE: valid Bid Limit Order price is less than current best (highest) Bid
  // Else is just a market order
  Money price;
  OrderDir order_dir{};
  time_point timestamp{ now() };

  [[nodiscard]] LimitOrder to_full() const
  {
    return { price,
             LimitOrderVal{ .volume = volume,
                            .agent_id = agent_id,
                            .timestamp = timestamp } };
  }
};
std::strong_ordering
operator<=>(const LimitOrderReq& lor1, const LimitOrderReq& lor2);

bool
operator==(const LimitOrderReq& lor1, const LimitOrderReq& lor2);
}

template<>
struct fmt::formatter<leyval::LimitOrderReq> : fmt::formatter<std::string_view>
{
  auto format(const leyval::LimitOrderReq& lor,
              format_context& ctx) const -> format_context::iterator;
};

////////////////////////////////////////////////////////////////////////////////

namespace leyval {
struct CancelOrderReq
{
  int volume{};
  int agent_id{};
  Money price;
  OrderDir order_dir{};
  time_point lo_timestamp{};
  time_point timestamp{ now() };
};

std::strong_ordering
operator<=>(const CancelOrderReq& cor1, const CancelOrderReq& cor2);

bool
operator==(const CancelOrderReq& cor1, const CancelOrderReq& cor2);

////////////////////////////////////////////////////////////////////////////////

using OrderReq_t = OrderReqVar<MarketOrderReq, LimitOrderReq, CancelOrderReq>;
}
// NOTE: OrderReq fmt provided by <fmt/std.h> (default variant)
