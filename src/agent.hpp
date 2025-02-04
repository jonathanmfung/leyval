#pragma once

#include <random>

#include "serializable.hpp"

#include "constants.hpp"
#include "order.hpp"
#include "order_book.hpp"
#include "util/timer.hpp"
#include "util/truncated_distribution.hpp"

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
// https://github.com/IvanJericevich/IJPCTG-ABMCoinTossX/blob/main/Scripts/ABMVolatilityAuctionProxy.jl
// class Agent_JericevichFundamentalist : public Agent{};
// class Agent_JericevichChartist : public Agent{};
// class Agent_JericevichProvider : public Agent{};

// https://arxiv.org/pdf/cond-mat/0103600
// class Agent_Raberto : public Agent{};

namespace leyval {

template<class PRNG>
double
power_law_distribution(float x_m, float alpha, PRNG& prng)
{
  // Jericevich Eq 6.2
  // f(x) = a * x_m^a / x^(a + 1)
  // F(x) = -(x_m/x)^a
  // F-1(x) = -x_m/x^(1/a)
  std::uniform_real_distribution<> unit{ 0.0, 1.0 };
  double x{ unit(prng) };
  return -x_m / (std::pow(x, 1.0 / alpha));
}

double
calc_alpha(const OrderBook::State& ob_state, const OrderDir od)
{
  // TODO: Check MOR direction (sell MO is minus)
  return (od == OrderDir::Bid)
           ? (1 - ob_state.imbalance / constants::simulation_jericevich::nu)
           : (1 + ob_state.imbalance / constants::simulation_jericevich::nu);
}

template<class PRNG>
class Agent_JericevichBase : public Agent<PRNG>
{
protected:
  Agent_JericevichBase(Money capital,
                       std::string type,
                       PRNG& prng,
                       float lambda_min,
                       float lambda_val,
                       float lambda_max)
    : Agent_JericevichBase<PRNG>{ capital, type, prng }
    , m_lambda_min{ lambda_min }
    , m_lambda_val{ lambda_val }
    , m_lambda_max{ lambda_max }
    , m_arrival_dist{ m_lambda_val }
  {
  }

  float m_lambda_min;
  float m_lambda_val;
  float m_lambda_max;

private:
  using trunc_exp_dist =
    TruncatedDistribution<std::exponential_distribution<float>, PRNG>;
  trunc_exp_dist m_arrival_dist;

protected:
  Timer m_timer{ [&]() {
    return static_cast<Timer::num_t>(arrival_dist(this->m_prng));
  } };
};

template<class PRNG>
class Agent_JericevichFundamentalist : public Agent_JericevichBase<PRNG>
{
public:
  Agent_JericevichFundamentalist(Money capital, PRNG& prng)
    : Agent_JericevichBase<PRNG>{
      capital,
      "JericevichFundamentalist",
      prng,
      constants::simulation_jericevich::taker_lambda_min,
      constants::simulation_jericevich::taker_lambda_val,
      constants::simulation_jericevich::taker_lambda_max
    }
  {
  }
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const override;

private:
  double m_fundamental_value;
  void update_fundamental_value(double m_0, double variance, PRNG& prng)
  {
    // TODO: maybe this is the same as lognormal?
    m_fundamental_value =
      m_0 *
      std::exp(std::normal_distribution<>{ 0, std::pow(variance, 2) }(prng));
  }

  double calc_xmin(const OrderBook::State& ob_state)
  {
    double delta{};
    return (std::abs(m_fundamental_value -
                     static_cast<float>(ob_state.mid_price)) <=
            delta * static_cast<float>(ob_state.mid_price))
             ? 20
             : 50;
  }
};

template<class PRNG>
class Agent_JericevichChartist : public Agent_JericevichBase<PRNG>
{
public:
  Agent_JericevichChartist(Money capital, PRNG& prng)
    : Agent_JericevichBase<PRNG>{
      capital,
      "JericevichChartist",
      prng,
      constants::simulation_jericevich::taker_lambda_min,
      constants::simulation_jericevich::taker_lambda_val,
      constants::simulation_jericevich::taker_lambda_max
    }
  {
  }
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const override;

private:
  double m_ema;
  double m_tprime;
  double m_tau;
  void update_ema(double t, const OrderBook::State& ob_state)
  {
    double lambda{ 1 - std::exp(-(t - m_tprime) / m_tau) };
    m_ema = m_ema + lambda * (static_cast<float>(ob_state.mid_price) - m_ema);
  }

  double calc_xmin(const OrderBook::State& ob_state)
  {
    double delta{};
    return (std::abs(static_cast<float>(ob_state.mid_price) - m_ema) <=
            delta * static_cast<float>(ob_state.mid_price))
             ? 20
             : 50;
  }
};

template<class PRNG>
class Agent_JericevichProvider : public Agent_JericevichBase<PRNG>
{
public:
  Agent_JericevichProvider(Money capital, PRNG& prng)
    : Agent_JericevichBase<PRNG>{
      capital,
      "JericevichProvider",
      prng,
      constants::simulation_jericevich::taker_lambda_min,
      constants::simulation_jericevich::taker_lambda_val,
      constants::simulation_jericevich::taker_lambda_max
    }
  {
  }
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const override;
};

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
Agent_JericevichFundamentalist<PRNG>::generate_order(
  [[maybe_unused]] const OrderBook::State& ob_state) const
{
  // TODO: inter-arrival time governed by a truncated exponential distribution

  std::vector<OrderReq_t> reqs{};

  int m_0{ 0 }; // TODO: implement

  if (this->m_timer.tick_and_check() == 0) {
    // TODO: reset timer
    update_fundamental_value(
      m_0,
      constants::simulation_jericevich::fundamentalist_sigma,
      this->m_prng);
    OrderDir od{ (m_fundamental_value < static_cast<float>(ob_state.mid_price))
                   ? OrderDir::Ask
                   : OrderDir::Bid };

    int volume{ power_law_distribution(
      calc_xmin(ob_state), calc_alpha(ob_state, od), this->m_prng) };

    reqs.emplace_back(MarketOrderReq{
      .volume = volume, .agent_id = this->get_id(), .order_dir = od });
  }

  return reqs;
}

template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JericevichChartist<PRNG>::generate_order(
  [[maybe_unused]] const OrderBook::State& ob_state) const
{
}

template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JericevichProvider<PRNG>::generate_order(
  [[maybe_unused]] const OrderBook::State& ob_state) const
{
}

template<class PRNG>
[[nodiscard]] std::vector<OrderReq_t>
Agent_JFProvider<PRNG>::generate_order(const OrderBook::State& ob_state) const
{
  SPDLOG_TRACE("JFProvider::generate_order:: get_id: {}", this->get_id());

  std::bernoulli_distribution place_order_prob(0.75);
  std::bernoulli_distribution bid_prob(
    static_cast<float>(ob_state.num_orders_ask) /
    static_cast<float>(ob_state.num_orders_bid + ob_state.num_orders_ask));

  std::vector<Money> price_offset;
  std::vector<int> ws;
  if (Money{ 150 } < ob_state.abs_spread) {
    // Weight inwards (negative)
    price_offset = { -2, -1, 0, 1, 2, 3 };
    ws = { 3, 3, 4, 2, 2, 2 };
  } else {
    price_offset = { -1, 0, 1, 2, 3, 4, 5 };
    ws = { 1, 3, 5, 4, 3, 2, 2 };
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
