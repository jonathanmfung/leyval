#include <cassert>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdlib>
#include <chrono>

#include <ctime>
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

// NOTE can't use std::strong_ordering with floats
using Money = int;
const auto now = std::chrono::steady_clock::now;
using time_point = std::chrono::time_point<std::chrono::steady_clock>;

template<typename T>
concept IsOrder = std::three_way_comparable<T, std::strong_ordering> &&
  requires(T a, T b) {
  { a.volume } -> std::same_as<int&>;
  { a.id } -> std::same_as<int&>;
  { a.time} -> std::same_as<const time_point&>;
  // { a <=> b } -> std::same_as<std::strong_ordering>;
};

template<IsOrder... Ts>
using OrderVar = std::variant<Ts...>;

struct MarketOrder
{
  int volume {};
  int id {};
  const time_point time {now()};
};
std::strong_ordering
operator<=>(const MarketOrder& mo1, const MarketOrder& mo2)
{
  return mo1.time <=> mo2.time;
}

bool
operator==(const MarketOrder& mo1, const MarketOrder& mo2)
{
  return mo1.id == mo2.id;
}

static_assert(IsOrder<MarketOrder>);

struct LimitOrder
{
  int volume {};
  int id {};
  const time_point time {now()};
  // NOTE valid Bid Limit Order price is less than current best (highest) Bid
  // Else is just a market order
  Money price;
};
std::strong_ordering
operator<=>(const LimitOrder& lo1, const LimitOrder& lo2)
{
  return std::tie(lo1.time, lo1.price) <=> std::tie(lo2.time, lo2.price);
}
bool
operator==(const LimitOrder& lo1, const LimitOrder& lo2)
{
  return std::tie(lo1.time, lo1.price) == std::tie(lo2.time, lo2.price);
  // return lo1.id == lo2.id;
}

static_assert(IsOrder<LimitOrder>);

using Order_t = OrderVar<MarketOrder, LimitOrder>;

// TODO could unify orders and have MarketOrder be when (std::optional<Money>
// price == null)
//   But this makes it hard to add new order types?

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

  void add_bid(LimitOrder lo)
  {
    m_bids.push(lo);
  }
  void add_ask(LimitOrder lo)  {
    m_asks.push(lo);
  }

  [[nodiscard]] MatchingSystem get_matching_system() const {return m_matching_sys;}

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

  [[nodiscard]] Money current_best_price(OrderDir od) const
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

// https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

class Agent
{
public:
  Agent(Money capital)
    : m_id{ new_id() }
    , m_capital{ capital }
  {
  }

  // generate instance of variant: https://stackoverflow.com/a/74303228
  std::pair<Order_t, OrderDir> generate_order(Order_t, OrderDir);

  void submit_order(std::unique_ptr<OrderBook> ob,
                    Order_t order,
                    OrderDir order_dir)
  {
    std::visit(overloaded{ [this, &ob, &order_dir](MarketOrder& mo) {
                            this->submit(std::move(ob), mo, order_dir);
                          },
                           [this, &ob, &order_dir](LimitOrder& lo) {
                             this->submit(std::move(ob), lo, order_dir);
                           }},
               order);
  }

  void submit(std::unique_ptr<OrderBook> ob, [[maybe_unused]] MarketOrder mo, OrderDir order_dir)
  {
    switch (order_dir)
      {
      case OrderDir::Bid:
	switch (ob->get_matching_system())
	  {
	  case MatchingSystem::fifo:
	    break;
	  case MatchingSystem::pro_rata:
	    break;
	  case MatchingSystem::random_selection:
	    break;
	  }
	break;
      case OrderDir::Ask:
	break;
      }
  }
  void submit(std::unique_ptr<OrderBook> ob, LimitOrder lo, OrderDir order_dir)
  {
    switch (order_dir)
      {
      case OrderDir::Bid:
	ob->add_bid(lo);
	break;
      case OrderDir::Ask:
	ob->add_ask(lo);
	break;
      }
  }


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
