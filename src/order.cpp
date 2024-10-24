#include "my_spdlog.hpp"
#include "order.hpp"

auto
fmt::formatter<OrderDir>::format(const OrderDir& od, format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(
    ctx.out(), "OrderDir {}", od == OrderDir::Bid ? "Bid" : "Ask");
}

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

auto
fmt::formatter<MarketOrderReq>::format(const MarketOrderReq& mor,
                                       format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "(MOR: {{a_id: {}, vol: {}, {}, {}}})",
                        mor.agent_id,
                        mor.volume,
                        mor.order_dir,
                        std::chrono::duration_cast<std::chrono::microseconds>(
                          mor.timestamp - INIT_TS));
}

////////////////////////////////////////////////////////////////////////////////

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

auto
fmt::formatter<LimitOrderReq>::format(const LimitOrderReq& lor,
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
                          lor.timestamp - INIT_TS));
}
