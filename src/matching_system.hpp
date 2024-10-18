#pragma once

#include "order.hpp"
#include "order_book.hpp"

struct TransactionRequest
{
  int bidder_id;
  int asker_id;
  int volume;
  Money price;
};

class MatchingSystem
{
public:
  // MatchingSystem takes in MarketOrder + OrderDir and matches to LimitOrders
  // in OrderBook. Adjusts OrderBook. Returns a List of [Transaction (Contract)]
  // for the exchange to execute. Since the other side (LOs) can be different
  // agents.
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

  std::vector<TransactionRequest> operator()(MarketOrderReq mo,
                                             OrderBook order_book);

  [[nodiscard]] Type get_type() const { return m_type; }

private:
  Type m_type;
};
