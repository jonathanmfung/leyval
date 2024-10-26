#pragma once

#include <functional>
#include "serializable.hpp"
#include "agent.hpp"
#include "matching_system.hpp"
#include "order.hpp"
#include "order_book.hpp"

class Exchange
{
public:
  Exchange(OrderBook order_book,
           std::vector<Agent> agents,
           MatchingSystem matching_sys)
    : m_order_book{ std::move(order_book) }
    , m_agents{ std::move(agents) }
    , m_matching_sys{ matching_sys }
  {
  }

  // clang-format off
  // NOTE:
  // Each tick:
  // - Loop over m_agents, with agent.generate_orders(order_book) -> Optional [OrderReq]
  //   - agent uses information from order_book to make decision
  //   - For each agent, push [OrderReq] onto current_order_requests (vector)
  //     - (Optional since agent can decide to noop, the ticks are quick)
  // - Loop over current_order_requests, with order_book.receive(OrderReq)
  //   - case LimitOrderReq: -> {Price : LimitOrder} and inserts into order_book Bid/AskContainer
  //     - Handle case of agent inserting into existing price_level (10 : v=2, id=foo) -> (10 : v=5, id=foo)
  //   - case MarketOrderReq (MatchingSystem)
  //     - MatchingSystem(MarketOrder, OrderBook) -> [TransactionRequest{bidder, asker, volume, total_price}]
  //     - Loop Exchange.execute(TransactionRequest): updates bidder/asker by id,
  //                                                  order_book by popping off Bid/AskContainer with agent_id at price_level
  // - current_order_requests.clear()
  // clang-format on

  // TODO: Add static tick count to help calculate agent's inter-arrival time
  void run();

  void saturate();

private:
  OrderBook m_order_book;
  std::vector<Agent> m_agents;
  MatchingSystem m_matching_sys;

  std::vector<OrderReq_t> m_current_order_requests;

  std::optional<std::reference_wrapper<Agent>> find_agent(const int agent_id);

  void execute(TransactionRequest trans);

  friend void to_json(json& j, const Exchange& exch);
  friend struct fmt::formatter<Exchange>;
};

template<>
struct fmt::formatter<Exchange> : fmt::formatter<std::string_view>
{
  auto format(const Exchange& exchange, format_context& ctx) const -> format_context::iterator;
};
