#pragma once

#include <queue>
#include <ranges>
#include <vector>

#include "matching_system.hpp"
#include "order.hpp"

class OrderBook
{
public:
  OrderBook(MatchingSystem matching_sys)
    : m_matching_sys{ matching_sys }
  {
  }
  void add_bid(LimitOrder lo) { m_bids.push(lo); }
  void add_ask(LimitOrder lo) { m_asks.push(lo); }

  // NOTE Assume that any order is valid (i.e. agent has sufficient capital and
  // shares)
  void receive_order(int agent_id, MarketOrder mo, OrderDir order_dir);
  void receive_order(int agent_id, LimitOrder lo, OrderDir order_dir);

  [[nodiscard]] Money current_best_price(OrderDir order_dir) const
  {
    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.top().price;
      case OrderDir::Ask:
        return m_asks.top().price;
    }
  }

  [[nodiscard]] Money quoted_spread()
  {
    // Expressed in %.
    // (x-y)/midpoint == 2(x-y)/(x+y)
    const Money ask{ current_best_price(OrderDir::Ask) };
    const Money bid{ current_best_price(OrderDir::Bid) };
    return 100 * 2 * ((ask - bid) / (ask + bid));
  }

  auto orders_at_best_price(OrderDir order_dir)
  {
    const Money best_price{ current_best_price(OrderDir::Ask) };
    return std::ranges::views::filter(m_asks,
      [best_price](const LimitOrder& lo) { return lo.price == best_price; });
  }

private:
  // NOTE std::less means largest element is at front
  using BidQueue = std::
    priority_queue<LimitOrder, std::vector<LimitOrder>, std::less<LimitOrder>>;
  using AskQueue = std::priority_queue<LimitOrder,
                                       std::vector<LimitOrder>,
                                       std::greater<LimitOrder>>;
  BidQueue m_bids;
  AskQueue m_asks;
  MatchingSystem m_matching_sys;
};
