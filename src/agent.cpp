#include "agent.hpp"

std::pair<Order_t, OrderDir> Agent::generate_order(Order_t, OrderDir)
{
  // TODO: Implement generate_order
  return {MarketOrder{2}, OrderDir::Ask};
}

void
Agent::submit_order(std::unique_ptr<OrderBook> ob,
                    Order_t order,
                    OrderDir order_dir)
{
  // TODO: Do proper sell/buy arithmetic to this agent.
  std::visit(overloaded{ [this, &ob, &order_dir](MarketOrder& mo) {
                          ob->receive_order(m_id, mo, order_dir);
                        },
                         [this, &ob, &order_dir](LimitOrder& lo) {
                           ob->receive_order(m_id, lo, order_dir);
                         } },
             order);
}
