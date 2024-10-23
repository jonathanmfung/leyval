#include <algorithm>
#include <cassert>

#include "order.hpp"
#include "order_book.hpp"

[[nodiscard]] Money
OrderBook::current_best_price(OrderDir order_dir) const
{
  Money best_price{};
  switch (order_dir) {
    case OrderDir::Bid:
      best_price = std::ranges::max_element(m_bids)->first;
      break;
    case OrderDir::Ask:
      best_price = std::ranges::min_element(m_asks)->first;
      break;
  }
  if (!(0 < best_price))
    throw std::domain_error(
      std::format("best_price must be greater than 0 ({})", order_dir));

  return best_price;
}

[[nodiscard]] Money
OrderBook::mid_price() const
{
  const Money ask{ current_best_price(OrderDir::Ask) };
  const Money bid{ current_best_price(OrderDir::Bid) };
  return (ask + bid) / 2;
}

[[nodiscard]] Money
OrderBook::quoted_spread() const
{
  // Expressed in %.
  // (x-y)/midpoint == 2(x-y)/(x+y)
  const Money ask{ current_best_price(OrderDir::Ask) };
  const Money bid{ current_best_price(OrderDir::Bid) };
  return 100 * 2 * ((ask - bid) / (ask + bid));
}

[[nodiscard]] int
OrderBook::num_orders(OrderDir order_dir) const
{
  switch (order_dir) {
    case OrderDir::Bid:
      return m_bids.size();
    case OrderDir::Ask:
      return m_asks.size();
    default:
      throw OrderDirInvalidValue("OrderBook::num_orders");
  }
}

void
OrderBook::insert(LimitOrder lo)
{
  // TODO: Check this implementation
  switch (lo.second.order_dir) {
    case OrderDir::Bid:
      m_bids.insert(lo);
      break;
    case OrderDir::Ask:
      m_asks.insert(lo);
      break;
    default:
      throw OrderDirInvalidValue("OrderBook::insert");
  }
}
