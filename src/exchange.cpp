#include "exchange.hpp"
#include <format>
#include <iostream>

void
Exchange::run()
{
  std::cout << "Init run\n";
  std::cout << "========================================\n";
  for (const auto& agent : m_agents) {
    std::cout << std::format("Loop {}\n", agent);
    std::vector<OrderReq_t> new_order_reqs{ agent.generate_order(
      m_order_book) };
    if (!new_order_reqs.empty()) {
      std::cout << std::format("\tnew_order is not empty\n");
      for (const OrderReq_t order_req : new_order_reqs) {
	std::cout << std::format("\tPushing {}\n", order_req);
        m_current_order_requests.push_back(order_req);
      }
    }
  }
  std::cout << "========================================\n";
  for (auto& order_request : m_current_order_requests) {
    std::cout << std::format("Loop {}\n", order_request);
    std::visit(
      overloaded{ [this](const LimitOrderReq& lor) {
		   std::cout << std::format("\t{}\n", lor);
                   m_order_book.insert(lor.to_full());
                 },
                  [this](MarketOrderReq& mor) {
		   std::cout << std::format("\t{}\n", mor);
                    auto transaction_requests{ m_matching_sys(mor,
                                                              m_order_book) };
                    for (auto transaction_request : transaction_requests) {
                      execute(transaction_request);
                    }
                  } },
      order_request);
  }
  std::cout << "Clear order_request\n";
  m_current_order_requests.clear();
}

std::optional<std::reference_wrapper<Agent>>
Exchange::find_agent(const int agent_id)
{
  auto is_id = [agent_id](const Agent& elem) {
    return elem.get_id() == agent_id;
  };
  if (auto res = std::ranges::find_if(m_agents, is_id);
      res != std::end(m_agents)) {
    return std::make_optional(std::ref(*res));
  }
  return std::nullopt;
}

void
Exchange::execute(TransactionRequest trans)
{
  auto asker{ find_agent(trans.asker_id) };
  auto bidder{ find_agent(trans.bidder_id) };

  if (asker.has_value() && bidder.has_value()) {
    asker->get().sell(trans.volume, trans.price);
    bidder->get().buy(trans.volume, trans.price);
  } else {
    throw;
  }
}
