#pragma once

#include "serializable.hpp"

#include "order.hpp"
#include "order_book.hpp"


class Agent
{
public:
  Agent(Money capital)
    : m_id{ new_id() }
    , m_capital{ capital }
    , m_shares{ 0 }
  {
  }

  // generate instance of variant: https://stackoverflow.com/a/74303228
  // NOTE empty vector means agent is choosing to noop
  [[nodiscard]] std::vector<OrderReq_t> generate_order(
    const OrderBook::State& ob_state) const;

  void buy(const int volume, const Money total_price);
  void sell(const int volume, const Money total_price);

  [[nodiscard]] int get_id() const { return m_id; }

private:
  int m_id{};
  Money m_capital{};
  int m_shares{};

  static int new_id()
  {
    static int id{ 0 };
    return ++id;
  }

  friend void to_json(json& j, const Agent& agent);
};

template<>
struct fmt::formatter<Agent> : fmt::formatter<std::string_view>
{
  auto format(const Agent& agent, format_context& ctx) const -> format_context::iterator;
};

/////////////////////////////////////

class Agent_JericevichFundamentalist : public Agent{};
class Agent_JericevichChartist : public Agent{};
class Agent_JericevichProvider : public Agent{};

class Agent_Raberto : public Agent{};
