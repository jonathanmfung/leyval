#pragma once

#include <memory>
#include <random>

#include "my_spdlog.hpp"
#include "serializable.hpp"

#include "agent.hpp"
#include "constants.hpp"
#include "matching_system.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "overloaded.hpp"

namespace leyval {
template<class PRNG>
class Exchange
{
public:
  using Agent_t = std::unique_ptr<Agent<PRNG>>;

  Exchange(OrderBook order_book,
           std::vector<Agent_t> agents,
           MatchingSystem matching_sys,
           PRNG& prng)
    : m_order_book{ std::move(order_book) }
    , m_agents{ std::move(agents) }
    , m_matching_sys{ matching_sys }
    , m_prng{ prng }
  {
  }

  // TODO: Add static tick count to help calculate agent's inter-arrival time
  void run();

  void saturate();

private:
  OrderBook m_order_book;
  std::vector<Agent_t> m_agents;
  MatchingSystem m_matching_sys;
  PRNG& m_prng;

  std::vector<OrderReq_t> m_current_order_requests;

  void execute(TransactionRequest trans);

  // https://github.com/nlohmann/json/issues/542#issuecomment-290665546
  friend inline void to_json(nlohmann::json& j,
                             [[maybe_unused]] const Exchange<PRNG>& exch)
  {
    j = nlohmann::json{
      { "order_book", exch.m_order_book },
      { "agents", exch.m_agents },
      // NOTE: using this with to_json(..., MatchingSystem) does not compile
      { "matching_system", exch.m_matching_sys.get_type_string() }
    };
    static_assert(Serializable<Exchange<PRNG>>);
  }
  friend struct fmt::formatter<Exchange<PRNG>>;
};
}

template<class PRNG>
struct fmt::formatter<leyval::Exchange<PRNG>> : fmt::formatter<std::string_view>
{
  auto format(const leyval::Exchange<PRNG>& exchange,
              format_context& ctx) const -> format_context::iterator;
};

template<class PRNG>
auto
fmt::formatter<leyval::Exchange<PRNG>>::format(
  const leyval::Exchange<PRNG>& exchange,
  format_context& ctx) const -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "Exchange({}, {})",
                        exchange.m_matching_sys,
                        exchange.m_order_book);
}

namespace leyval {

template<class PRNG>
void
Exchange<PRNG>::saturate()
{
  SPDLOG_DEBUG("Exchange::saturate: Init {}", m_order_book);

  // NOTE: Assert that highest bid < lowest ask

  // NOTE: Strong assumption that no agent dies in-bewtween Exhange constructor
  // and saturate
  //       (and relying on Agent::new_id being sequential)
  auto max_agent{ std::ranges::max_element(m_agents,
                                           [](const auto& a1, const auto& a2) {
                                             return a1->get_id() < a2->get_id();
                                           }) };
  std::uniform_int_distribution<> agent_id(1, (*max_agent)->get_id());

  // Money price;
  using namespace constants::saturate;
  std::uniform_int_distribution<> bid_prices(price_center - price_far_offset,
                                             price_center - price_close_offset);
  std::uniform_int_distribution<> ask_prices(price_center + price_close_offset,
                                             price_center + price_far_offset);

  // int volume{};
  std::poisson_distribution<> volume(4);

  SPDLOG_DEBUG("Exchange::saturate: Gen Bids & Asks");
  // Bids
  for ([[maybe_unused]] const int _ :
       std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(m_prng),
                       .agent_id = agent_id(m_prng),
                       .price = bid_prices(m_prng),
                       .order_dir = OrderDir::Bid };
    m_order_book.insert(lor.to_full());
  }

  // Asks
  for ([[maybe_unused]] const int _ :
       std::views::iota(0, n_contracts_per_side)) {
    LimitOrderReq lor{ .volume = volume(m_prng),
                       .agent_id = agent_id(m_prng),
                       .price = ask_prices(m_prng),
                       .order_dir = OrderDir::Ask };
    m_order_book.insert(lor.to_full());
  }

  SPDLOG_DEBUG("Exchange::saturate: Post {}", m_order_book);
}

template<class PRNG>
void
Exchange<PRNG>::run()
{
  const OrderBook::State ob_state{ m_order_book.update_get_state() };

  for (const auto& agent : m_agents) {
    SPDLOG_TRACE("Loop {}", *agent);
    std::vector<OrderReq_t> new_order_reqs{ agent->generate_order(ob_state) };
    if (!new_order_reqs.empty()) {
      SPDLOG_TRACE("new_order is not empty");
      for (const OrderReq_t order_req : new_order_reqs) {
        SPDLOG_TRACE("\tPushing {}", order_req);
        m_current_order_requests.push_back(order_req);
      }
    }
  }
  SPDLOG_DEBUG("After agents send requests: (current_order_requests)");
  for ([[maybe_unused]] const auto& order_req : m_current_order_requests) {
    SPDLOG_TRACE("{}", order_req);
  }

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

template<class PRNG>
void
Exchange<PRNG>::execute(TransactionRequest trans)
{
  // NOTE: Treat asker/bidder_id as index of m_agents
  //       Strongly assumes m_agents is in same order as Agent::new_id

  m_agents[trans.asker_id]->sell(trans.volume, trans.price);
  m_agents[trans.bidder_id]->buy(trans.volume, trans.price);
}
}
