#pragma once

#include <map>

#include "order.hpp"

class OrderBook
{
public:
  OrderBook() = default;

  // NOTE Assume that any order is valid (i.e. agent has sufficient capital and
  // shares)

  [[nodiscard]] Money current_best_price(OrderDir order_dir) const;
  [[nodiscard]] Money quoted_spread() const;
  [[nodiscard]] int num_orders(OrderDir order_dir) const;

  // Returns pair of iterators to range of best-priced orders.
  // These are able to mutate the underlying Bid/AskContainer.
  auto orders_at_best_price(OrderDir order_dir)
  {
    const Money best_price{ current_best_price(order_dir) };

    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.equal_range(best_price);
      case OrderDir::Ask:
        return m_asks.equal_range(best_price);
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

  friend struct std::formatter<OrderBook>;
};

template<>
struct std::formatter<OrderBook> : std::formatter<std::string>
{
  auto format(const OrderBook& order_book, format_context& ctx) const
  {
    // TODO Format # of each Bid/AskContainer (need to add public method)
    return formatter<string>::format(
				     std::format("OrderBook(Bids: (#{}, ${}), Asks: (#{}, ${}))", order_book.m_bids.size(), order_book.current_best_price(OrderDir::Bid), order_book.m_asks.size(), order_book.current_best_price(OrderDir::Ask)), ctx);
  }
};
