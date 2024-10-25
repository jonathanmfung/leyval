#pragma once

#include <chrono>

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/std.h>


// NOTE: can't use std::strong_ordering with floats
using Money = int;
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

template<>
struct fmt::formatter<OrderDir> : fmt::formatter<std::string_view>
{
  auto format(const OrderDir& od, format_context& ctx) const -> format_context::iterator;
};

///////////////////

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

template<>
struct fmt::formatter<MarketOrderReq> : fmt::formatter<std::string_view>
{
  auto format(const MarketOrderReq& mor, format_context& ctx) const -> format_context::iterator;
};

////////////////////////////////////////////////////////////////////////////////

// Helper method for second in Ask/Bid Container
struct LimitOrderVal
{
  int volume{};
  int agent_id{};
  OrderDir order_dir{};
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
    return { price, LimitOrderVal{ volume, agent_id, order_dir, timestamp } };
  }
};
std::strong_ordering
operator<=>(const LimitOrderReq& lor1, const LimitOrderReq& lor2);

bool
operator==(const LimitOrderReq& lor1, const LimitOrderReq& lor2);

template<>
struct fmt::formatter<LimitOrderReq> : fmt::formatter<std::string_view>
{
  auto format(const LimitOrderReq& lor, format_context& ctx) const -> format_context::iterator;
};

// TODO: Add CancelOrder (no dir, remove specific LimitOrder in OrderBook)

////////////////////////////////////////////////////////////////////////////////

using OrderReq_t = OrderReqVar<MarketOrderReq, LimitOrderReq>;

// NOTE: OrderReq fmt provided by <fmt/std.h> (default variant)
