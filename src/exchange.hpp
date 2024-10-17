#include "agent.hpp"
#include "order_book.hpp"

// TODO: Maybe all actual trading logics of price/share transactions happens in
// Exchange, which has access to Agents

class Exchange
{
private:
  OrderBook m_order_book;
  std::vector<Agent> m_agents;
  MatchingSystem m_matching_sys;

  std::optional<std::reference_wrapper<Agent>> find_agent(const int agent_id)
  {
    auto is_id = [agent_id](const Agent& elem) {
      return elem.get_id() == agent_id;
    };
    if (auto res = std::ranges::find_if(m_agents, is_id);
        res != std::end(m_agents)) {
      return std::make_optional(std::ref(*res));
    }
    return std::nullopt;
  }

  void execute_transaction(Agent& bidder,
                            Agent& asker,
                            int volume,
                            Money total_price)
  {
    asker.sell(volume, total_price);
    bidder.buy(volume, total_price);
  }
};
