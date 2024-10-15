#include <cassert>
#include <cmath>
#include <compare>
#include <cstdlib>

#include <format>
#include <iostream>

#include <algorithm>
#include <array>
#include <memory>
#include <queue>
#include <ranges>
#include <vector>

#include <random>

// TODO add global project namespace
namespace Constants {

}

///////////////////////

struct BaseOrder
{
  int volume;
  int id;
};
std::strong_ordering
operator<=>(const BaseOrder& bo1, const BaseOrder& bo2)
{
  return bo1.volume <=> bo2.volume;
}

// NOTE can't use std::strong_ordering with floats
using Money = int;

struct MarketOrder : BaseOrder
{};

// TODO could unify orders and have MarketOrder be when (std::optional<Money> price == null)
//   But this makes it hard to add new order types?
struct LimitOrder : BaseOrder
{
  // NOTE valid Bid Limit Order price is less than current best (highest) Bid
  // Else is just a market order
  Money price;
};

std::strong_ordering
operator<=>(const LimitOrder& lo1, const LimitOrder& lo2)
{
  return std::tie(lo1.price, lo1.volume) <=> std::tie(lo2.price, lo2.volume);
}

enum class OrderDir
{
  Bid,
  Ask,
};

///////////////////////

enum class MatchingSystem
{
  fifo,
  pro_rata,
  random_selection,
};

class OrderBook
{
public:
private:
  // NOTE std::less means largest element is at front
  using BidQueue = std::
    priority_queue<LimitOrder, std::vector<LimitOrder>, std::less<LimitOrder>>;
  using AskQueue = std::priority_queue<LimitOrder,
                                       std::vector<LimitOrder>,
                                       std::greater<LimitOrder>>;
  BidQueue m_bids;
  AskQueue m_asks;
  MatchingSystem m_matching_sys;

  Money current_best_price(OrderDir od) const
  {
    switch (od) {
      case OrderDir::Bid:
        return m_bids.top().price;
      case OrderDir::Ask:
        return m_asks.top().price;
    }
  }
};

///////////////////////

class Agent
{
public:
  Agent(Money capital)
    : m_id{ new_id() }
    , m_capital{ capital }
  {
  }

  std::pair<BaseOrder, OrderDir> generate_order(BaseOrder, OrderDir);

  void submit_order(std::unique_ptr<OrderBook> ob,
                    BaseOrder order,
                    OrderDir order_dir);
  // Do proper sell/buy arithmetic to this agent.

  // Could visit on std::variant<MarketOrder, LimitOrder>
  // which dispatches to:
  //   submit(MarketOrder, ...)
  //   submit(LimitOrder, ...)

private:
  int m_id;
  Money m_capital;

  int new_id()
  {
    static int id{ 0 };
    return ++id;
  }
};

///////////////////////

int
main()
{
  Agent a1{ 100 };
  Agent a2{ 80 };

  OrderBook ob{};

  return 0;
}
