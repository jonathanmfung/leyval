#pragma once

#include <map>
#include <ranges>

#include "order.hpp"
#include "serializable.hpp"

namespace leyval {
class OrderBook
{
public:
  OrderBook() = default;

  // NOTE: Assume that any order is valid (i.e. agent has sufficient capital and
  // shares)

  // TODO: Instead of a struct with separate implementation, each method could
  // check some cache based on internal time stamp
  struct State
  {
    Money best_price_bid;
    Money best_price_ask;
    Money mid_price;
    float quoted_spread;
    Money abs_spread;
    int num_orders_bid;
    int num_orders_ask;
    float imbalance;

    State(const OrderBook& ob)
      : best_price_bid{ ob.current_best_price(OrderDir::Bid) }
      , best_price_ask{ ob.current_best_price(OrderDir::Ask) }
      // TODO: can reuse computed best_price_bid/ask
      , mid_price{ ob.mid_price() }
      , quoted_spread{ ob.quoted_spread() }
      , abs_spread{ ob.abs_spread() }
      , num_orders_bid{ ob.num_orders(OrderDir::Bid) }
      , num_orders_ask{ ob.num_orders(OrderDir::Ask) }
      // TODO: can reuse computed num_orders_bid/ask
      , imbalance{ ob.imbalance() }
    {
    }
  };

  [[nodiscard]] State update_get_state()
  {
    m_state = State(*this);
    return m_state;
  }

  // Returns pair of iterators to range of best-priced orders.
  // These are able to mutate the underlying Bid/AskContainer.
  // This is only used in FIFOMatchingSystem,
  // (which runs for every MOR * every volume).
  auto orders_at_best_price(OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.equal_range(m_state.best_price_bid);
      case OrderDir::Ask:
        return m_asks.equal_range(m_state.best_price_ask);
      default:
        throw OrderDirInvalidValue("OrderBook::orders_at_best_price");
    }
  }

  // NOTE: m_bids/bids are different types due to comparator,
  // so combined function is impossible
  auto orders_at_agentid_bid(int agent_id)
  {
    auto agent_eq = [&](const auto& e) {
      return e.second.agent_id == agent_id;
    };
    return m_bids | std::ranges::views::filter(agent_eq);
  }

  auto orders_at_agentid_ask(int agent_id) const
  {
    auto agent_eq = [&](const auto& e) {
      return e.second.agent_id == agent_id;
    };
    return m_asks | std::ranges::views::filter(agent_eq);
  }

  void insert(LimitOrderReq lor);

  // order_it is iterator to m_bids/asks
  // NOTE: I think this causes iterator invalidation
  template<typename T>
  T remove_order(T order_it, OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid:
        return m_bids.erase(order_it);
      case OrderDir::Ask:
        return m_asks.erase(order_it);
      default:
        throw OrderDirInvalidValue("OrderBook::remove_order");
    }
  }

  // Returns:
  //   true  <- earliest order successfully removed
  //   false <- earliest order not found, thus nothing removed
  bool remove_earliest_order(int agent_id, OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid: {
        bool found{ false };
        auto smallest{ m_bids.begin() };
        for (auto iter{ smallest }; iter != m_bids.end(); ++iter) {
          if ((iter->second.agent_id == agent_id) &&
              (iter->second.timestamp <= smallest->second.timestamp)) {
            found = true;
            smallest = iter;
          }
        }
        if (found) {
          remove_order(smallest, order_dir);
        }
        return found;
      }
      case OrderDir::Ask: {
        bool found{ false };
        auto smallest{ m_asks.begin() };
        for (auto iter{ smallest }; iter != m_asks.end(); ++iter) {
          if ((iter->second.agent_id == agent_id) &&
              (iter->second.timestamp <= smallest->second.timestamp)) {
            found = true;
            smallest = iter;
          }
        }
        if (found) {
          remove_order(smallest, order_dir);
        }
        return found;
      }
      default:
        throw OrderDirInvalidValue("OrderBook::remove_earliest_order");
    }
  }

  auto remove_specific_order(int agent_id, time_point tp, OrderDir order_dir)
  {
    switch (order_dir) {
      case OrderDir::Bid: {
        for (auto iter{ m_bids.begin() }; iter != m_bids.end(); ++iter) {
          if ((iter->second.agent_id == agent_id) &&
              (iter->second.timestamp == tp)) {
            return remove_order(iter, order_dir);
          }
        }
        return m_bids.end();
      }
      case OrderDir::Ask: {
        for (auto iter{ m_asks.begin() }; iter != m_asks.end(); ++iter) {
          if ((iter->second.agent_id == agent_id) &&
              (iter->second.timestamp == tp)) {
            return remove_order(iter, order_dir);
          }
        }
        return m_asks.end();
      }
      default:
        throw OrderDirInvalidValue("OrderBook::remove_specific_order");
    }
  }

private:
  // NOTE std::greater means largest element is at beginning
  // This is an attempt to push frequent values to front of iteration.
  // (bid -> larger)
  using BidContainer = std::multimap<Money, LimitOrderVal, std::greater<>>;
  using AskContainer = std::multimap<Money, LimitOrderVal, std::less<>>;

  BidContainer m_bids;
  AskContainer m_asks;

  State m_state{ update_get_state() };

  [[nodiscard]] Money current_best_price(OrderDir order_dir) const;
  [[nodiscard]] Money mid_price() const;
  [[nodiscard]] float quoted_spread() const;
  [[nodiscard]] Money abs_spread() const;
  [[nodiscard]] int num_orders(OrderDir order_dir) const;
  [[nodiscard]] float imbalance() const;

  friend struct fmt::formatter<OrderBook>;
  friend void to_json(nlohmann::json& j, const OrderBook& order_book);
};
}

template<>
struct fmt::formatter<leyval::OrderBook> : fmt::formatter<std::string_view>
{
  auto format(const leyval::OrderBook& order_book,
              format_context& ctx) const -> format_context::iterator;
};
