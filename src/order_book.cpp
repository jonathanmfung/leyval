#include <algorithm>
#include <cassert>
#include <map>

#include "my_spdlog.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "serializable.hpp"

namespace leyval {
void
to_json(nlohmann::json& j, const OrderBook& order_book)
{
  std::map<Money, int> bid_counts;
  std::map<Money, int> ask_counts;

  for (const auto& pair : order_book.m_bids) {
    ++bid_counts[pair.first];
  }

  for (const auto& pair : order_book.m_asks) {
    ++ask_counts[pair.first];
  }

  j = nlohmann::json{ { "bid_counts", bid_counts },
                      { "ask_counts", ask_counts } };
}
static_assert(Serializable<OrderBook>);
} // namespace leyval

auto
fmt::formatter<leyval::OrderBook>::format(const leyval::OrderBook& order_book,
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

namespace leyval {
[[nodiscard]] Money
OrderBook::current_best_price(OrderDir order_dir) const
{
  // TODO Maybe handle this init case better? optional? don't run on init?
  // Init case
  if (m_bids.empty() && m_asks.empty()) {
    return 1;
  }

  Money best_price{ 0 };
  switch (order_dir) {
    case OrderDir::Bid:
      best_price = std::ranges::max_element(m_bids)->first;
      break;
    case OrderDir::Ask:
      best_price = std::ranges::min_element(m_asks)->first;
      break;
    default:
      throw OrderDirInvalidValue("OrderBook::current_best_price");
  }
  if (!(Money{ 0 } < best_price)) {
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
  return (ask + bid) / Money{ 2 };
}

[[nodiscard]] float
OrderBook::quoted_spread() const
{
  // Expressed in %.
  // (x-y)/midpoint == 2(x-y)/(x+y)
  const int pct{ 100 };
  return pct * static_cast<float>(abs_spread() / mid_price());
}

[[nodiscard]] Money
OrderBook::abs_spread() const
{
  const Money ask{ current_best_price(OrderDir::Ask) };
  const Money bid{ current_best_price(OrderDir::Bid) };
  return (ask - bid);
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

[[nodiscard]] float
OrderBook::imbalance() const
{

  int bids {num_orders(OrderDir::Bid)};
  int asks{num_orders(OrderDir::Ask)};
  return (bids - asks) / static_cast<float>(bids + asks);
}

void
OrderBook::insert(LimitOrderReq lor)
{
  switch (lor.order_dir) {
    case OrderDir::Bid:
      m_bids.insert(lor.to_full());
      break;
    case OrderDir::Ask:
      m_asks.insert(lor.to_full());
      break;
    default:
      throw OrderDirInvalidValue("OrderBook::insert");
  }
}
}
