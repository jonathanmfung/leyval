#include "my_spdlog.hpp"
#include "agent.hpp"

auto
fmt::formatter<Agent>::format(const Agent& agent,
                                      format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(), "Agent {}", agent.get_id());
}

template<typename T>
concept isAgent = requires(T a) {
  { a.m_id } -> std::same_as<int>;
  { a.m_capital } -> std::same_as<Money>;
  { a.m_shares } -> std::same_as<int>;

  { a.generate_order } -> std::same_as<std::vector<OrderReq_t>>;
};

[[nodiscard]] std::vector<OrderReq_t>
Agent::generate_order([[maybe_unused]] const OrderBook::State& ob_state) const
{
  // TODO: Implement generate_order
  // std::cout << "Agent::generate_order "
  //           << order_book.current_best_price(OrderDir::Bid) << "\n";

  // buy_sell_prob = 0.5
  // can adjust this each step? +/- 0.01

  std::vector<OrderReq_t> reqs{};

  SPDLOG_TRACE("Agent::generate_order:: get_id: {}", get_id());

  reqs.emplace_back(LimitOrderReq{ .volume = 5,
                                   .agent_id = get_id(),
                                   .price = 35,
                                   .order_dir = OrderDir::Bid });
  reqs.emplace_back(LimitOrderReq{ .volume = 5,
                                   .agent_id = get_id(),
                                   .price = 45,
                                   .order_dir = OrderDir::Ask });

  reqs.emplace_back(MarketOrderReq{
    .volume = 2, .agent_id = get_id(), .order_dir = OrderDir::Bid });
  reqs.emplace_back(MarketOrderReq{
    .volume = 2, .agent_id = get_id(), .order_dir = OrderDir::Ask });

  return reqs;
}

void
Agent::buy(const int volume, const Money total_price)
{
  m_shares += volume;
  m_capital -= total_price;
}
void
Agent::sell(const int volume, const Money total_price)
{
  m_shares -= volume;
  m_capital += total_price;
}
