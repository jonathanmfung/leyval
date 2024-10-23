#include <cassert>
#include <format>
#include <iostream>
#include <ranges>

#include "matching_system.hpp"

std::vector<TransactionRequest>
MatchingSystem::operator()(const MarketOrderReq mor, OrderBook& order_book)
{
  std::cout << "MS Invoke\n";
  assert(mor.volume > 0 && "MarketOrderReq must be positive");
  // TODO: This falsely throws during an empty init.
  // if (mor.volume > order_book.num_orders(!mor.order_dir))
  //   throw std::logic_error(
  //     std::format("MarketOrderReq.volume({}, {}) must be not greater than
  //     OrderBook's {}({})",
  //                 mor.volume,
  //                 mor.order_dir,
  //                 !mor.order_dir,
  //                 order_book.num_orders(!mor.order_dir)));

  std::vector<TransactionRequest> trans_reqs{};
  switch (m_type) {
    case fifo: {
      // Each share traded needs to recalculate the best price
      // TODO: Check that iota should start at 0
      for (const int i : std::views::iota(0, mor.volume)) {
        std::cout << std::format("MS::(FIFO) Loop {}\n", i);

        auto best_orders{ order_book.orders_at_best_price(mor.order_dir) };
        Money best_price =
          best_orders.first->first; // best_orders.first_iterator->key
                                    // (arbitrarily use first iterator)

        std::cout << std::format(
          "best_orders.first->second: a_id: {}, {}, vol: {}\n",
          best_orders.first->second.agent_id,
          best_orders.first->second.order_dir,
          best_orders.first->second.volume);
        std::cout << std::format("MS:: best_price: {}\n", best_price);

        // Pop earliest-timestamp LimitOrderVal at best_price
        auto early_comp = [](const LimitOrder& lo1, const LimitOrder& lo2) {
          return lo1.second.timestamp < lo2.second.timestamp;
        };

        auto earliest_best_order{ std::min_element(
          best_orders.first, best_orders.second, early_comp) };

        std::cout << std::format("earliest_best_order->second.agent_id: {}\n",
                                 earliest_best_order->second.agent_id);

        for (auto it = best_orders.first; it != best_orders.second;) {
          if (it == earliest_best_order) {
            it = order_book.remove_order(it, mor.order_dir);
            break;
          } else {
            ++it;
          };
        }

        trans_reqs.emplace_back(
          TransactionRequest(mor.agent_id,
                             earliest_best_order->second.agent_id,
                             earliest_best_order->second.volume,
                             best_price,
                             mor.order_dir));
      }
      break;
    }

      // Defer to Exchange::execute_transaction}

    case pro_rata:
      {
      // TODO: Implement pro_rata
      // Get all LOs at current_best_price (best_los =
      // std::ranges::views::filter(order_book.m_asks, [](const LimitOrder&
      // lo){lo.price == order_book.current_best_price(OrderBid::Ask)()}
      // Calculate weight (weight = mor.volume /. std::size(best_los))
      auto best_orders{ order_book.orders_at_best_price(mor.order_dir) };
      break;
      }
    case random_selection:
      // TODO: Implement random_selection
      break;
  }
  return trans_reqs;
};
