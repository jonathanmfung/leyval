#include "order.hpp"

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
