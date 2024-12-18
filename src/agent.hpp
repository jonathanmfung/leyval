#pragma once

#include <random>

#include "serializable.hpp"

#include "order.hpp"
#include "order_book.hpp"

namespace leyval {
template<class PRNG>
class Agent
{
public:
  Agent(Money capital, std::string type, PRNG& prng)
    : m_prng{ prng }
    , m_id{ new_id() }
    , m_capital{ capital }
    , m_shares{ 0 }
    , m_type{ type }
  {
  }

  virtual ~Agent() = default;

  // generate instance of variant: https://stackoverflow.com/a/74303228
  // NOTE: empty vector means agent is choosing to noop
  [[nodiscard]] virtual std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const = 0;

  void buy(const int volume, const Money total_price);
  void sell(const int volume, const Money total_price);

  [[nodiscard]] virtual int get_id() const { return m_id; }

protected:
  PRNG& m_prng;

private:
  int m_id{};
  Money m_capital{ 0 };
  int m_shares{};
  std::string m_type{};

  static int new_id()
  {
    static int id{ -1 };
    return ++id;
  }

  friend inline void to_json(nlohmann::json& j, const Agent& agent)
  {
    j = nlohmann::json{ { "id", agent.get_id() },
                        { "capital", agent.m_capital },
                        { "shares", agent.m_shares },
                        { "type", agent.m_type } };
    static_assert(Serializable<Agent<PRNG>>);
  }
};

template<class PRNG>
void
Agent<PRNG>::buy(const int volume, const Money total_price)
{
  m_shares += volume;
  m_capital -= total_price;
}

template<class PRNG>
void
Agent<PRNG>::sell(const int volume, const Money total_price)
{
  m_shares -= volume;
  m_capital += total_price;
}
}

template<class PRNG>
struct fmt::formatter<leyval::Agent<PRNG>> : fmt::formatter<std::string_view>
{
  auto format(const leyval::Agent<PRNG>& agent,
              format_context& ctx) const -> format_context::iterator;
};

template<class PRNG>
auto
fmt::formatter<leyval::Agent<PRNG>>::format(const leyval::Agent<PRNG>& agent,
                                            format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(), "Agent {}", agent.get_id());
}

/////////////////////////////////////
// https://open.uct.ac.za/items/574390a1-2466-4128-8920-6261505220e0
// class Agent_JericevichFundamentalist : public Agent{};
// class Agent_JericevichChartist : public Agent{};
// class Agent_JericevichProvider : public Agent{};

// https://arxiv.org/pdf/cond-mat/0103600
// class Agent_Raberto : public Agent{};

namespace leyval {
template<class PRNG>
class Agent_JFProvider : public Agent<PRNG>
{
public:
  Agent_JFProvider(Money capital, PRNG& prng)
    : Agent<PRNG>{ capital, "JFProvider", prng }
  {
  }
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const override;
};

template<class PRNG>
class Agent_JFTaker : public Agent<PRNG>
{
public:
  Agent_JFTaker(Money capital, PRNG& prng)
    : Agent<PRNG>{ capital, "JFTaker", prng }
  {
  }
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const override;
};
}

// Impls //////////////////////////////////////////////////////////////////////

namespace leyval {
template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JFProvider<PRNG>::generate_order(const OrderBook::State& ob_state) const
{
  SPDLOG_TRACE("JFProvider::generate_order:: get_id: {}", this->get_id());

  std::bernoulli_distribution place_order_prob(0.75);
  std::bernoulli_distribution bid_prob(
    static_cast<float>(ob_state.num_orders_ask) /
    (ob_state.num_orders_bid + ob_state.num_orders_ask));

  std::vector<Money> price_offset;
  std::vector<int> ws;
  if (Money{ 4 } < ob_state.abs_spread) {
    // Weight inwards (negative)
    price_offset = { -2, -1, 0, 1, 2 };
    ws = { 3, 3, 4, 2, 2 };
  } else {
    price_offset = { -1, 0, 1, 2, 3 };
    ws = { 1, 3, 5, 4, 3 };
  }

  assert(price_offset.size() == ws.size());

  std::discrete_distribution<> weights(ws.begin(), ws.end());
  std::poisson_distribution volume(15);

  std::vector<OrderReq_t> reqs{};

  if (place_order_prob(this->m_prng)) {
    if (bid_prob(this->m_prng)) {
      // Cancel earliest LO
      // TODO: Need a better way for agent to decide cancellation order_dir
      // TODO: use orders_at_agentid_bid/ask to construct cor
      reqs.emplace_back(
        CancelOrderReq{ .volume = 0,
                        .agent_id = this->get_id(),
                        .price = 0,
                        .order_dir = OrderDir::Bid,
                        .lo_timestamp = std::chrono::steady_clock::now() });

      // Create new LO
      reqs.emplace_back(LimitOrderReq{
        .volume = volume(this->m_prng),
        .agent_id = this->get_id(),
        .price = ob_state.best_price_bid - price_offset[weights(this->m_prng)],
        .order_dir = OrderDir::Bid });
      // Sometimes create another LO, to offset reduction from MO
      if (bid_prob(this->m_prng)) {
        reqs.emplace_back(
          LimitOrderReq{ .volume = volume(this->m_prng),
                         .agent_id = this->get_id(),
                         .price = ob_state.best_price_bid -
                                  price_offset[weights(this->m_prng)],
                         .order_dir = OrderDir::Bid });
      }
    } else {
      reqs.emplace_back(
        CancelOrderReq{ .volume = 0,
                        .agent_id = this->get_id(),
                        .price = 0,
                        .order_dir = OrderDir::Ask,
                        .lo_timestamp = std::chrono::steady_clock::now() });
      reqs.emplace_back(LimitOrderReq{
        .volume = volume(this->m_prng),
        .agent_id = this->get_id(),
        .price = ob_state.best_price_ask + price_offset[weights(this->m_prng)],
        .order_dir = OrderDir::Ask });

      if (bid_prob(this->m_prng)) {
        reqs.emplace_back(
          LimitOrderReq{ .volume = volume(this->m_prng),
                         .agent_id = this->get_id(),
                         .price = ob_state.best_price_ask +
                                  price_offset[weights(this->m_prng)],
                         .order_dir = OrderDir::Ask });
      }
    }
  }

  return reqs;
}

template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JFTaker<PRNG>::generate_order(
  [[maybe_unused]] const OrderBook::State& ob_state) const
{
  SPDLOG_TRACE("JFTaker::generate_order:: get_id: {}", this->get_id());

  std::bernoulli_distribution place_order_prob(0.5);
  std::bernoulli_distribution buy_prob(0.5);
  std::poisson_distribution volume(2);

  std::vector<OrderReq_t> reqs{};

  if (place_order_prob(this->m_prng)) {
    if (buy_prob(this->m_prng)) {
      reqs.emplace_back(MarketOrderReq{ .volume = volume(this->m_prng),
                                        .agent_id = this->get_id(),
                                        .order_dir = OrderDir::Bid });
    } else {
      reqs.emplace_back(MarketOrderReq{ .volume = volume(this->m_prng),
                                        .agent_id = this->get_id(),
                                        .order_dir = OrderDir::Ask });
    }
  }

  return reqs;
}
}
