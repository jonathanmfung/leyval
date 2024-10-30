#pragma once

#include "agent.hpp"
#include "matching_system.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "serializable.hpp"
#include <memory>

class Exchange
{
public:
  using Agent_t = std::unique_ptr<Agent>;

  Exchange(OrderBook order_book,
           std::vector<Agent_t> agents,
           MatchingSystem matching_sys)
    : m_order_book{ std::move(order_book) }
    , m_agents{ std::move(agents) }
    , m_matching_sys{ matching_sys }
  {
  }

  // TODO: Add static tick count to help calculate agent's inter-arrival time
  void run();

  void saturate();

private:
  OrderBook m_order_book;
  std::vector<Agent_t> m_agents;
  MatchingSystem m_matching_sys;

  std::vector<OrderReq_t> m_current_order_requests;

  void execute(TransactionRequest trans);

  friend void to_json(json& j, const Exchange& exch);
  friend struct fmt::formatter<Exchange>;
};

template<>
struct fmt::formatter<Exchange> : fmt::formatter<std::string_view>
{
  auto format(const Exchange& exchange,
              format_context& ctx) const -> format_context::iterator;
};
