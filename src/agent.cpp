#include "agent.hpp"

#include <iostream>

[[nodiscard]] std::vector<OrderReq_t>
Agent::generate_order(const OrderBook& order_book) const
{
  // TODO: Implement generate_order
  std::cout << "Agent::generate_order " << order_book.current_best_price(OrderDir::Bid) << "\n";
  std::vector<OrderReq_t> reqs{};
  reqs.emplace_back(LimitOrderReq{ 5, get_id(), 40, OrderDir::Bid});
  reqs.emplace_back(MarketOrderReq{ 2, get_id(), OrderDir::Ask});
  return reqs;
}

void
Agent::buy(const int volume, const Money total_price)
{
  m_shares += volume;
  m_capital -= total_price;
}
void
Agent::sell(const int volume, const Money total_price)
{
  m_shares -= volume;
  m_capital += total_price;
}
