#include "order_book.hpp"

void OrderBook::receive_order([[maybe_unused]] int agent_id, [[maybe_unused]]MarketOrder mo, [[maybe_unused]]OrderDir order_dir){
  // TODO: Implement receive_order(MarketOrder)
};

void OrderBook::receive_order([[maybe_unused]]int agent_id, [[maybe_unused]]LimitOrder lo, [[maybe_unused]]OrderDir order_dir){
  // TODO: Implement receive_order(LimitOrder)
};

  // NOTE: successful match is current agent.m_capital =-
  // ob.current_best_price(order_ddir) * mo.volume
  //      but need to do this so that current_best_price is always updated
  //      (while --mo.volume > 0)
