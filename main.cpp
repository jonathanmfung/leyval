#include <cassert>
#include <chrono>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdlib>

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
concept IsOrder =
  std::three_way_comparable<T, std::strong_ordering> && requires(T a, T b) {
    { a.volume } -> std::same_as<int&>;
    { a.id } -> std::same_as<int&>;
    { a.time } -> std::same_as<time_point&>;
  };

template<IsOrder... Ts>
using OrderVar = std::variant<Ts...>;

struct MarketOrder
{
  int volume{};
  int id{};
  time_point time{ now() };
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
  int volume{};
  int id{};
  time_point time{ now() };
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

class MatchingSystem
{
public:
  // MatchingSystem takes in MarketOrder + OrderDir and matches to LimitOrders
  // in OrderBook. Adjusts OrderBook. Returns a std::pair<int volume, Money
  // total_price> that the agent uses to adjust themselves.
  enum Type
  {
    fifo,
    pro_rata,
    random_selection,
  };

  MatchingSystem(Type type)
    : m_type{ type }
  {
  }

  void operator()();

  Type get_type() const { return m_type; }

private:
  Type m_type;
};

class OrderBook
{
public:
  OrderBook(MatchingSystem matching_sys)
    : m_matching_sys{ matching_sys }
  {
  }
  void add_bid(LimitOrder lo) { m_bids.push(lo); }
  void add_ask(LimitOrder lo) { m_asks.push(lo); }

  // NOTE Assume
  void receive_order(int agent_id, MarketOrder mo, OrderDir order_dir);

  // TODO successful match is current agent.m_capital =-
  // ob.current_best_price(order_ddir) * mo.volume
  //      but need to do this so that current_best_price is always updated
  //      (while --mo.volume > 0)

  void receive_order(int agent_id, LimitOrder lo, OrderDir order_dir);

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

  [[nodiscard]] Money current_best_price(OrderDir order_dir) const
  {
    switch (order_dir) {
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
    , m_shares{ 0 }
  {
  }

  // generate instance of variant: https://stackoverflow.com/a/74303228
  std::pair<Order_t, OrderDir> generate_order(Order_t, OrderDir);

  void submit_order(std::unique_ptr<OrderBook> ob,
                    Order_t order,
                    OrderDir order_dir)
  {
    std::visit(overloaded{ [this, &ob, &order_dir](MarketOrder& mo) {
                            ob->receive_order(m_id, mo, order_dir);
                          },
                           [this, &ob, &order_dir](LimitOrder& lo) {
                             ob->receive_order(m_id, lo, order_dir);
                           } },
               order);
  }

  // Do proper sell/buy arithmetic to this agent.

  // Could visit on std::variant<MarketOrder, LimitOrder>
  // which dispatches to:
  //   submit(MarketOrder, ...)
  //   submit(LimitOrder, ...)

private:
  int m_id{};
  Money m_capital{};
  int m_shares{};

  int new_id()
  {
    static int id{ 0 };
    return ++id;
  }
};

///////////////////////
class Exchange {
private:
  OrderBook m_order_book;
  std::vector<Agent> m_agents;
};

///////////////////////

int
main()
{
  Agent a1{ 100 };
  Agent a2{ 80 };

  const MatchingSystem ms{ MatchingSystem::fifo };
  OrderBook ob{ ms };

  return 0;
}
