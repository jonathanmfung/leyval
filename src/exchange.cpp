#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <set>
#include <stdexcept>

#include "my_spdlog.hpp"

#include "exchange.hpp"
#include "order.hpp"
#include "overloaded.hpp"

auto
fmt::formatter<Exchange>::format(const Exchange& exchange, format_context& ctx)
  const -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "Exchange({}, {})",
                        exchange.m_matching_sys,
                        exchange.m_order_book);
}

void
Exchange::saturate()
{
  SPDLOG_DEBUG("Exchange::saturate: Init {}", m_order_book);

  // NOTE: Assert that highest bid < lowest ask

  const int n_contracts_per_side{ 100 };

  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()

  // NOTE: Strong assumption that no agent dies in-bewtween Exhange constructor and saturate
  //       (and relying on Agent::new_id being sequential)
  auto max_agent{ std::ranges::max_element(
    m_agents, [](const Agent& a1, const Agent& a2) {
      return a1.get_id() < a2.get_id();
    }) };
  std::uniform_int_distribution<> agent_id(1, max_agent->get_id());

  // Money price;
  std::uniform_int_distribution<> bid_prices(90, 98);
  std::uniform_int_distribution<> ask_prices(102, 110);

  // int volume{};
  std::poisson_distribution<> volume(4);

  SPDLOG_DEBUG("Exchange::saturate: Gen Bids & Asks");
  // Bids
  for (const int _ : std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(gen),
                       .agent_id = agent_id(gen),
                       .price = bid_prices(gen),
                       .order_dir = OrderDir::Bid };
    m_order_book.insert(lor.to_full());
  }

  // Asks
  for (const int _ : std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(gen),
                       .agent_id = agent_id(gen),
                       .price = ask_prices(gen),
                       .order_dir = OrderDir::Ask };
    m_order_book.insert(lor.to_full());
  }

  SPDLOG_DEBUG("Exchange::saturate: Post {}", m_order_book);
}

void
Exchange::run()
{
  const OrderBook::State ob_state{ m_order_book.update_get_state() };

  for (const auto& agent : m_agents) {
    SPDLOG_TRACE("Loop {}", agent);
    std::vector<OrderReq_t> new_order_reqs{ agent.generate_order(ob_state) };
    if (!new_order_reqs.empty()) {
      SPDLOG_TRACE("new_order is not empty");
      for (const OrderReq_t order_req : new_order_reqs) {
        SPDLOG_TRACE("\tPushing {}", order_req);
        m_current_order_requests.push_back(order_req);
      }
    }
  }
  SPDLOG_DEBUG("After agents send requests: (current_order_requests)");
  for ([[maybe_unused]] const auto& order_req : m_current_order_requests)
    SPDLOG_TRACE("{}", order_req);

  SPDLOG_DEBUG("========================================");
  for (auto& order_request : m_current_order_requests) {
    SPDLOG_TRACE("Loop {}", order_request);
    std::visit(
      overloaded{ [this](const LimitOrderReq& lor) {
                   SPDLOG_TRACE("LOR Visit");
                   m_order_book.insert(lor.to_full());
                 },
                  [this](MarketOrderReq& mor) {
                    SPDLOG_TRACE("MOR Visit");
                    auto transaction_requests{ m_matching_sys(mor,
                                                              m_order_book) };
                    for (auto transaction_request : transaction_requests) {
                      SPDLOG_TRACE("{}", transaction_request);
                      execute(transaction_request);
                    }
                  } },
      order_request);
  }
  m_current_order_requests.clear();
}

std::optional<std::reference_wrapper<Agent>>
Exchange::find_agent(const int agent_id)
{
  SPDLOG_DEBUG("Exchange::find_agent - For a_id: {}", agent_id);
  auto is_id = [agent_id](const Agent& elem) {
    return elem.get_id() == agent_id;
  };
  if (auto res = std::ranges::find_if(m_agents, is_id);
      res != std::end(m_agents)) {
    SPDLOG_TRACE("Exchange::find_agent - Found {}", *res);
    return std::make_optional(std::ref(*res));
  }
  SPDLOG_DEBUG("Exchange::find_agent - Found NONE");
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

    SPDLOG_ERROR("Exchange::execute could not find asker ({}) ior bidder ({})",
                 trans.asker_id,
                 trans.bidder_id);
    throw std::logic_error("");
  }
}
