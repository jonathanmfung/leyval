#pragma once

#include "order.hpp"

class MatchingSystem
{
public:
  // MatchingSystem takes in MarketOrder + OrderDir and matches to LimitOrders
  // in OrderBook. Adjusts OrderBook. Returns a List of [Transaction (Contract)] for the exchange to execute.
  // Since the other side (LOs) can be different agents.
  enum Type
  {
    fifo,
    pro_rata,
    random_selection,
  };

  MatchingSystem(Type type)
    : m_type{ type }
  {
  }

  void operator()(MarketOrder mo, OrderDir order_dir);

  Type get_type() const { return m_type; }

private:
  Type m_type;
};
