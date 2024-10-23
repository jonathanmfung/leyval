#include <cassert>
#include <format>
#include <iostream>
#include <random>
#include <ranges>
#include <set>
#include <stdexcept>

#include "exchange.hpp"
#include "order.hpp"
#include "overloaded.hpp"

void
Exchange::saturate()
{
  std::cout << std::format("Exchange::saturate: Init {}\n", m_order_book);

  // NOTE: Assert that highest bid < lowest ask

  const int n_contracts_per_side{ 100 };

  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()

  // int agent_id{}; TODO: get set of agent_id and randomize from it.
  // TODO: Or could just assume 0..max(agent_ids) and uniform_int_distribution
  //       (No agent dies)
  std::set<int> agent_ids{};
  for (const auto& agent : m_agents)
    agent_ids.insert(agent.get_id());

  // Money price;
  std::uniform_int_distribution<> bid_prices(90, 98);
  std::uniform_int_distribution<> ask_prices(102, 110);

  // int volume{};
  std::poisson_distribution<> volume(4);

  std::cout << "Exchange::saturate: Gen Bids & Asks\n";
  // Bids
  for (const int _ : std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(gen),
                       .agent_id = 1,
                       .price = bid_prices(gen),
                       .order_dir = OrderDir::Bid };
    m_order_book.insert(lor.to_full());
  }

  // Asks
  for (const int _ : std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(gen),
                       .agent_id = 1,
                       .price = ask_prices(gen),
                       .order_dir = OrderDir::Ask };
    m_order_book.insert(lor.to_full());
  }

  std::cout << std::format("Exchange::saturate: Post {}\n", m_order_book);
}

void
Exchange::run()
{
  std::cout << "Init run\n";
  std::cout << "========================================\n";
  const OrderBook::State ob_state{ m_order_book.update_get_state()};

  for (const auto& agent : m_agents) {
    std::cout << std::format("Loop {}\n", agent);
    std::vector<OrderReq_t> new_order_reqs{ agent.generate_order(ob_state) };
    if (!new_order_reqs.empty()) {
      std::cout << std::format("\tnew_order is not empty\n");
      for (const OrderReq_t order_req : new_order_reqs) {
        std::cout << std::format("\tPushing {}\n", order_req);
        m_current_order_requests.push_back(order_req);
      }
    }
  }
  std::cout << "After agents send requests: (current_order_requests)\n";
  for (const auto& order_req : m_current_order_requests)
    std::cout << std::format("{}\n", order_req);

  std::cout << "\n========================================\n";
  for (auto& order_request : m_current_order_requests) {
    std::cout << std::format("Loop {}\n", order_request);
    std::visit(
      overloaded{ [this](const LimitOrderReq& lor) {
                   std::cout << "LOR Visit\n";
                   m_order_book.insert(lor.to_full());
                 },
                  [this](MarketOrderReq& mor) {
                    std::cout << "MOR Visit\n";
                    auto transaction_requests{ m_matching_sys(mor,
                                                              m_order_book) };
                    for (auto transaction_request : transaction_requests) {
                      std::cout << std::format("{}\n", transaction_request);
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
  std::cout << std::format("Exchange::find_agent - For a_id: {}\n", agent_id);
  auto is_id = [agent_id](const Agent& elem) {
    return elem.get_id() == agent_id;
  };
  if (auto res = std::ranges::find_if(m_agents, is_id);
      res != std::end(m_agents)) {
    std::cout << std::format("Exchange::find_agent - Found {}\n", *res);
    return std::make_optional(std::ref(*res));
  }
  std::cout << std::format("Exchange::find_agent - Found NONE\n");
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
    throw std::logic_error(
      std::format("Exchange::execute could not find asker ({}) ior bidder ({})",
                  trans.asker_id,
                  trans.bidder_id));
    // assert(false && "Exchange::execute could not find asker ior bidder");
  }
}
