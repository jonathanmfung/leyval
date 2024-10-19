#include "agent.hpp"

#include <format>
#include <iostream>

[[nodiscard]] std::vector<OrderReq_t>
Agent::generate_order([[maybe_unused]] const OrderBook& order_book) const
{
  // TODO: Implement generate_order
  // std::cout << "Agent::generate_order "
  //           << order_book.current_best_price(OrderDir::Bid) << "\n";
  std::vector<OrderReq_t> reqs{};

  std::cout << std::format("Agent::generate_order:: get_id: {}\n", get_id());

  reqs.emplace_back(LimitOrderReq{ .volume = 5,
                                   .agent_id = get_id(),
                                   .price = 35,
                                   .order_dir = OrderDir::Bid });
  reqs.emplace_back(LimitOrderReq{ .volume = 5,
                                   .agent_id = get_id(),
                                   .price = 45,
                                   .order_dir = OrderDir::Ask });

  reqs.emplace_back(MarketOrderReq{
    .volume = 2, .agent_id = get_id(), .order_dir = OrderDir::Bid });
  reqs.emplace_back(MarketOrderReq{
    .volume = 2, .agent_id = get_id(), .order_dir = OrderDir::Ask });

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
