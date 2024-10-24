#include <cassert>
#include <ranges>

#include "my_spdlog.hpp"
#include "matching_system.hpp"

auto
fmt::formatter<TransactionRequest>::format(const TransactionRequest& treq,
                                           format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(ctx.out(),
                        "(TREQ: {{bid_id: {}, ask_id: {}, prc: {}, vol: {}}})",
                        treq.bidder_id,
                        treq.asker_id,
                        treq.price,
                        treq.volume);
}

auto
fmt::formatter<MatchingSystem>::format(const MatchingSystem& match_sys,
                                       format_context& ctx) const
  -> format_context::iterator
{
  return fmt::format_to(
    ctx.out(), "MatchingSystem({})", match_sys.get_type_string());
}

std::vector<TransactionRequest>
MatchingSystem::operator()(const MarketOrderReq mor, OrderBook& order_book)
{
  SPDLOG_DEBUG("MS Invoke");
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
      for ([[maybe_unused]] const int i : std::views::iota(0, mor.volume)) {
        SPDLOG_TRACE("MS::(FIFO) Loop {}", i);

        auto best_orders{ order_book.orders_at_best_price(mor.order_dir) };
        Money best_price =
          best_orders.first->first; // best_orders.first_iterator->key
                                    // (arbitrarily use first iterator)

        SPDLOG_TRACE("best_orders.first->second: a_id: {}, {}, vol: {}",
                     best_orders.first->second.agent_id,
                     best_orders.first->second.order_dir,
                     best_orders.first->second.volume);
        SPDLOG_TRACE("MS:: best_price: {}", best_price);

        // Pop earliest-timestamp LimitOrderVal at best_price
        auto early_comp = [](const LimitOrder& lo1, const LimitOrder& lo2) {
          return lo1.second.timestamp < lo2.second.timestamp;
        };

        auto earliest_best_order{ std::min_element(
          best_orders.first, best_orders.second, early_comp) };

        SPDLOG_TRACE("earliest_best_order->second.agent_id: {}",
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

    case pro_rata: {
      // TODO: Implement pro_rata
      // mor.volume = 11, total limit size = 100 (3 orders of 20, 30, 50)
      // weight = 11 / 100
      // shares_per_order = (20, 30, 50) .* weight -> floor/round
      // excess = total limit size - sum(shares_per_order)
      // if excess > 0: FIFO the excess

      [[maybe_unused]] auto best_orders{ order_book.orders_at_best_price(
        mor.order_dir) };
      [[maybe_unused]] int total_orders{ 5 };
      [[maybe_unused]] double weight{ static_cast<double>(mor.volume) /
                                      total_orders };
      break;
    }
    case random_selection:
      // TODO: Implement random_selection
      break;
  }
  return trans_reqs;
};
