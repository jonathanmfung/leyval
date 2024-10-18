#include <algorithm>

#include "order_book.hpp"

[[nodiscard]] Money
OrderBook::current_best_price(OrderDir order_dir) const
{
  Money best_price{};
  switch (order_dir) {
    // NOTE Using min_element since multimaps are reveresed (bid =
    // std::greater, ask = std::less)
    case OrderDir::Bid:
      best_price = std::ranges::min_element(m_bids)->first;
      break;
    case OrderDir::Ask:
      best_price = std::ranges::min_element(m_asks)->first;
      break;
  }
  return best_price;
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

void
OrderBook::insert(LimitOrder lo)
{
  // TODO: Check Implementation
  switch (lo.second.order_dir) {
    case OrderDir::Bid:
      m_bids.insert(lo);
      break;
    case OrderDir::Ask:
      m_asks.insert(lo);
      break;
  }
}
