#pragma once

#include <chrono>

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
struct std::formatter<OrderDir> : std::formatter<std::string>
{
  auto format(const OrderDir& order_dir, format_context& ctx) const
  {
    return formatter<string>::format(
      std::format("OrderDir {}", order_dir == OrderDir::Bid ? "Bid" : "Ask"),
      ctx);
  }
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
struct std::formatter<MarketOrderReq> : std::formatter<std::string>
{
  auto format(const MarketOrderReq& mor, format_context& ctx) const
  {
    return formatter<string>::format(
      std::format("(MOR: {{a_id: {}, vol: {}, {}, {}}})",
                  mor.agent_id,
                  mor.volume,
                  mor.order_dir,
                  std::chrono::duration_cast<std::chrono::microseconds>(mor.timestamp - INIT_TS)),
      ctx);
  }
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
struct std::formatter<LimitOrderReq> : std::formatter<std::string>
{
  auto format(const LimitOrderReq& lor, format_context& ctx) const
  {
    return formatter<string>::format(
      std::format("(LOR: {{a_id: {}, prc: {}, vol: {}, {}, {}}})",
                  lor.agent_id,
		  lor.price,
                  lor.volume,
                  lor.order_dir,
                  std::chrono::duration_cast<std::chrono::microseconds>(lor.timestamp - INIT_TS)),
      ctx);
  }
};

// TODO: Add CancelOrder (no dir, refer to specific LimitOrder in OrderBook)

////////////////////////////////////////////////////////////////////////////////

using OrderReq_t = OrderReqVar<MarketOrderReq, LimitOrderReq>;

template<>
struct std::formatter<OrderReq_t> : std::formatter<std::string>
{
  auto format(const OrderReq_t& order_req, format_context& ctx) const
  {
    return formatter<string>::format(
      std::visit([](const auto& req) { return std::format("{}", req); },
                 order_req),
      ctx);
  }
};
