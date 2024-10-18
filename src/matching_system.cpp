#include <ranges>

#include "matching_system.hpp"

std::vector<TransactionRequest>
MatchingSystem::operator()(MarketOrderReq mor, OrderBook order_book)
{
  std::vector<TransactionRequest> trans_reqs{};
  switch (m_type) {
    case fifo: {

      // Each share traded needs to recalculate the best price
      for (const int _ : std::views::iota(1, mor.volume)) {
        auto best_orders{ order_book.orders_at_best_price(mor.order_dir) };
        Money best_price =
          best_orders.first->first; // best_orders.first_iterator->key

        // Pop earliest-timestamp LimitOrderVal at best_price
        auto early_comp = [](const LimitOrder& lo1, const LimitOrder& lo2) {
          return lo1.second.timestamp < lo2.second.timestamp;
        };

        auto earliest_best_order{ std::min_element(
          best_orders.first, best_orders.second, early_comp) };
        for (auto it = best_orders.first; it != best_orders.second;) {
          if (it == earliest_best_order) {
            it = order_book.remove_order(it, mor.order_dir);
            break;
          } else {
            ++it;
          };
        }

        trans_reqs.emplace_back(
          TransactionRequest{ mor.agent_id,
                              earliest_best_order->second.agent_id,
                              earliest_best_order->second.volume,
                              best_price });
      }
      break;
    }

      // Defer to Exchange::execute_transaction}

    case pro_rata:
      // TODO: Implement pro_rata
      // Get all LOs at current_best_price (best_los =
      // std::ranges::views::filter(order_book.m_asks, [](const LimitOrder&
      // lo){lo.price == order_book.current_best_price(OrderBid::Ask)()}
      // Calculate weight (weight = mor.volume /. std::size(best_los))
      break;
    case random_selection:
      // TODO: Implement random_selection
      break;
  }
  return trans_reqs;
};
