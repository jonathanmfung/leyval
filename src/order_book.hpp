#pragma once

#include <map>

#include "serializable.hpp"
#include "order.hpp"

class OrderBook
{
public:
  OrderBook() = default;

  // NOTE Assume that any order is valid (i.e. agent has sufficient capital and
  // shares)

  // TODO: State so that only need to calculate values once per timestep
  struct State
  {
    Money best_price_bid;
    Money best_price_ask;
    Money mid_price;
    Money quoted_spread;
    int num_orders_bid;
    int num_orders_ask;

    State(const OrderBook& ob)
      : best_price_bid{ ob.current_best_price(OrderDir::Bid) }
      , best_price_ask{ ob.current_best_price(OrderDir::Ask) }
      , mid_price{ ob.mid_price() }
      , quoted_spread{ ob.quoted_spread() }
      , num_orders_bid{ ob.num_orders(OrderDir::Bid) }
      , num_orders_ask{ ob.num_orders(OrderDir::Ask) }
    {
    }
  };

  [[nodiscard]] State update_get_state()
  {
    m_state = State(*this);
    return m_state;
  }

  // TODO: This is only used in MatchingSystem (which runs for every
  // order_request, every volume [probably add to State]).
  // Returns pair of iterators to range of best-priced orders. These are able to
  // mutate the underlying Bid/AskContainer.
  auto orders_at_best_price(OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.equal_range(m_state.best_price_bid);
      case OrderDir::Ask:
        return m_asks.equal_range(m_state.best_price_ask);
      default:
        throw OrderDirInvalidValue("OrderBook::orders_at_best_price");
    }
  }

  void insert(LimitOrder lo);

  // order_it is iterator to m_bids/asks
  template<typename T>
  T remove_order(T order_it, OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.erase(order_it);
      case OrderDir::Ask:
        return m_asks.erase(order_it);
      default:
        throw OrderDirInvalidValue("OrderBook::remove_order");
    }
  }

private:
  // NOTE std::lgreater means largest element is at beginning
  // This is an attempt to push frequent values to front of iteration. (bid ->
  // larger)
  using BidContainer = std::multimap<Money, LimitOrderVal, std::greater<>>;
  using AskContainer = std::multimap<Money, LimitOrderVal, std::less<>>;

  BidContainer m_bids;
  AskContainer m_asks;

  State m_state{ update_get_state() };

  // TODO: Maybe imbalance (rho) : [num_orders(bid) - num_orders(ask)] /
  // [num_orders(bid) + num_orders(ask)]
  [[nodiscard]] Money current_best_price(OrderDir order_dir) const;
  [[nodiscard]] Money mid_price() const;
  [[nodiscard]] Money quoted_spread() const;
  [[nodiscard]] int num_orders(OrderDir order_dir) const;

  friend struct fmt::formatter<OrderBook>;
  friend void to_json(json& j, const OrderBook& order_book);
};
template<>
struct fmt::formatter<OrderBook> : fmt::formatter<std::string_view>
{
  auto format(const OrderBook& order_book, format_context& ctx) const -> format_context::iterator;
};
