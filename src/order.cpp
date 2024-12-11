#include "order.hpp"
#include "my_spdlog.hpp"
#include "serializable.hpp"

namespace leyval {
NLOHMANN_JSON_SERIALIZE_ENUM(OrderDir,
                             {
                               { OrderDir::Bid, "Bid" },
                               { OrderDir::Ask, "Ask" },
                             })

static_assert(Serializable<OrderDir>);
}
auto
fmt::formatter<leyval::OrderDir>::format(const leyval::OrderDir& od,
                                         format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(
    ctx.out(), "OrderDir {}", od == leyval::OrderDir::Bid ? "Bid" : "Ask");
}

namespace leyval {
OrderDir
operator!(OrderDir order_dir)
{
  switch (order_dir) {
    case OrderDir::Bid:
      return OrderDir::Ask;
    case OrderDir::Ask:
      return OrderDir::Bid;
    default:
      throw OrderDirInvalidValue("OrderBook::operator!");
  }
}

////////////////////////////////////////////////////////////////////////////////

std::strong_ordering
operator<=>(const MarketOrderReq& mor1, const MarketOrderReq& mor2)
{
  return mor1.timestamp <=> mor2.timestamp;
}

bool
operator==(const MarketOrderReq& mor1, const MarketOrderReq& mor2)
{
  return mor1.agent_id == mor2.agent_id;
}

static_assert(IsOrderReq<MarketOrderReq>);
}

auto
fmt::formatter<leyval::MarketOrderReq>::format(
  const leyval::MarketOrderReq& mor,
  format_context& ctx) const -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "(MOR: {{a_id: {}, vol: {}, {}, {}}})",
                        mor.agent_id,
                        mor.volume,
                        mor.order_dir,
                        std::chrono::duration_cast<std::chrono::microseconds>(
                          mor.timestamp - leyval::INIT_TS));
}

////////////////////////////////////////////////////////////////////////////////
namespace leyval {
std::strong_ordering
operator<=>(const LimitOrderVal& lov1, const LimitOrderVal& lov2)
{
  return lov1.timestamp <=> lov2.timestamp;
}

std::strong_ordering
operator<=>(const LimitOrderReq& lor1, const LimitOrderReq& lor2)
{
  return std::tie(lor1.timestamp, lor1.price) <=>
         std::tie(lor2.timestamp, lor2.price);
}

bool
operator==(const LimitOrderReq& lor1, const LimitOrderReq& lor2)
{
  return std::tie(lor1.timestamp, lor1.price) ==
         std::tie(lor2.timestamp, lor2.price);
}

static_assert(IsOrderReq<LimitOrderReq>);
}

auto
fmt::formatter<leyval::LimitOrderReq>::format(const leyval::LimitOrderReq& lor,
                                              format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "(LOR: {{a_id: {}, prc: {}, vol: {}, {}, {}}})",
                        lor.agent_id,
                        lor.price,
                        lor.volume,
                        lor.order_dir,
                        std::chrono::duration_cast<std::chrono::microseconds>(
                          lor.timestamp - leyval::INIT_TS));
}
////////////////////////////////////////////////////////////////////////////////

namespace leyval {
std::strong_ordering
operator<=>(const CancelOrderReq& cor1, const CancelOrderReq& cor2)
{
  return std::tie(cor1.timestamp, cor1.price) <=>
         std::tie(cor2.timestamp, cor2.price);
}

bool
operator==(const CancelOrderReq& cor1, const CancelOrderReq& cor2)
{
  return std::tie(cor1.timestamp, cor1.price) ==
         std::tie(cor2.timestamp, cor2.price);
}
static_assert(IsOrderReq<CancelOrderReq>);
}
