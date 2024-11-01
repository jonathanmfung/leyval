#include <algorithm>
#include <cassert>
#include <map>

#include "my_spdlog.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "serializable.hpp"

void
to_json(json& j, const OrderBook& order_book)
{
  std::map<Money, int> bid_counts;
  std::map<Money, int> ask_counts;

  for (const auto& pair : order_book.m_bids) {
    ++bid_counts[pair.first];
  }

  for (const auto& pair : order_book.m_asks) {
    ++ask_counts[pair.first];
  }

  j = json{ { "bid_counts", bid_counts }, { "ask_counts", ask_counts } };
}
static_assert(Serializable<OrderBook>);

auto
fmt::formatter<OrderBook>::format(const OrderBook& order_book,
                                  format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "OrderBook(Bids: (#{}, ${}), Asks: (#{}, ${}))",
                        order_book.m_state.num_orders_bid,
                        order_book.m_state.best_price_bid,
                        order_book.m_state.num_orders_ask,
                        order_book.m_state.best_price_ask);
}

[[nodiscard]] Money
OrderBook::current_best_price(OrderDir order_dir) const
{
  // TODO Maybe handle this init case better? optional? don't run on init?
  // Init case
  if (m_bids.empty() && m_asks.empty()) {
    return 1;
  }

  Money best_price{};
  switch (order_dir) {
    case OrderDir::Bid:
      best_price = std::ranges::max_element(m_bids)->first;
      break;
    case OrderDir::Ask:
      best_price = std::ranges::min_element(m_asks)->first;
      break;
  }
  if (!(0 < best_price)) {
    SPDLOG_ERROR(
      "OrderBook::current_best_price: best_price must be greater than 0 ({})",
      order_dir);
    throw std::domain_error("");
  }

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
  const Money bid{current_best_price(OrderDir::Bid)};
  const int pct {100};
  return pct * 2 * ((ask - bid) / (ask + bid));
}

[[nodiscard]] int
OrderBook::num_orders(OrderDir order_dir) const
{
  switch (order_dir) {
    case OrderDir::Bid:
      return std::ssize(m_bids);
    case OrderDir::Ask:
      return std::ssize(m_asks);
    default:
      throw OrderDirInvalidValue("OrderBook::num_orders");
  }
}

void
OrderBook::insert(LimitOrder lo)
{
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
