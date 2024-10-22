#pragma once

#include <format>

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
};

template<>
struct std::formatter<Agent> : std::formatter<std::string>
{
  auto format(const Agent& agent, format_context& ctx) const
  {
    return formatter<string>::format(std::format("Agent {}", agent.get_id()),
                                     ctx);
  }
};
