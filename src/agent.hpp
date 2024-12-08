#pragma once

#include "serializable.hpp"

#include "order.hpp"
#include "order_book.hpp"
#include <random>

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
  // NOTE empty vector means agent is choosing to noop
  // TODO Maybe make this an Optional, not sure how many agents will actually
  // submit multiple, unless they are cancels
  [[nodiscard]] virtual std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const = 0;

  void buy(const int volume, const Money total_price);
  void sell(const int volume, const Money total_price);

  [[nodiscard]] virtual int get_id() const { return m_id; }

protected:
  PRNG& m_prng;

private:
  int m_id{};
  Money m_capital{};
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

namespace leyval {
template<class PRNG>
{
}

template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JFProvider<PRNG>::generate_order(const OrderBook::State& ob_state) const
{
  SPDLOG_TRACE("JFProvider::generate_order:: get_id: {}", this->get_id());

  std::bernoulli_distribution bid_prob(0.5);
  std::uniform_int_distribution price_offset(-1, 4);
  std::poisson_distribution volume(3);

  std::vector<OrderReq_t> reqs{};

  if (bid_prob(this->m_prng)) {
    reqs.emplace_back(LimitOrderReq{ .volume = volume(this->m_prng),
                                     .agent_id = this->get_id(),
                                     .price = ob_state.best_price_bid -
                                              price_offset(this->m_prng),
                                     .order_dir = OrderDir::Bid });
  } else {
    reqs.emplace_back(LimitOrderReq{ .volume = volume(this->m_prng),
                                     .agent_id = this->get_id(),
                                     .price = ob_state.best_price_ask +
                                              price_offset(this->m_prng),
                                     .order_dir = OrderDir::Ask });
  }

  return reqs;
}
}
