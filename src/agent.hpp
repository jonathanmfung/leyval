#pragma once

#include <chrono>

#include "order.hpp"
#include "order_book.hpp"

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
                    OrderDir order_dir);

  void buy(const int volume, const Money total_price){
    m_shares += volume;
    m_capital -= total_price;
  }

  void sell(const int volume, const Money total_price){
    m_shares -= volume;
    m_capital += total_price;
  }

  int get_id() const {return m_id;}

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
