#pragma once

#include "serializable.hpp"

#include "order.hpp"
#include "order_book.hpp"

namespace leyval {
class Agent
{
public:
  Agent(Money capital, std::string type)
    : m_id{ new_id() }
    , m_capital{ capital }
    , m_shares{ 0 }
    , m_type{ type }
  {
  }

  virtual ~Agent() = default;

  // generate instance of variant: https://stackoverflow.com/a/74303228
  // NOTE empty vector means agent is choosing to noop
  [[nodiscard]] virtual std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const = 0;

  void buy(const int volume, const Money total_price);
  void sell(const int volume, const Money total_price);

  [[nodiscard]] int get_id() const { return m_id; }

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

  friend void to_json(nlohmann::json& j, const Agent& agent);
};
}

template<>
struct fmt::formatter<leyval::Agent> : fmt::formatter<std::string_view>
{
  auto format(const leyval::Agent& agent,
              format_context& ctx) const -> format_context::iterator;
};

/////////////////////////////////////
// https://open.uct.ac.za/items/574390a1-2466-4128-8920-6261505220e0
// class Agent_JericevichFundamentalist : public Agent{};
// class Agent_JericevichChartist : public Agent{};
// class Agent_JericevichProvider : public Agent{};

// https://arxiv.org/pdf/cond-mat/0103600
// class Agent_Raberto : public Agent{};

//
namespace leyval {
class Agent_JFProvider : public Agent
{
public:
  Agent_JFProvider(Money capital)
    : Agent{ capital, "JFProvider" }
  {
  }
  [[nodiscard]] virtual std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const;
};
// class Agent_JFTaker : public Agent{};
}
