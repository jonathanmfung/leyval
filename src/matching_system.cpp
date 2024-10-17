#include <ranges>
#include "order.hpp"
#include "matching_system.hpp"

void
MatchingSystem::operator()(MarketOrder mo, OrderDir order_dir)
{
  // TODO: Implement operator()
  switch (m_type) {
    case fifo:
      for (const int i : std::views::iota(1, mo.volume))
	{
	  // best_ask = ob.m_asks.top();
	  // ob.m_asks.pop();
	  // total_price += best_ask.price;
	}

      // Defer to Exchange::execute_transaction
      break;
    case pro_rata:
      // Get all LOs at current_best_price (best_los = std::ranges::views::filter(order_book.m_asks, [](const LimitOrder& lo){lo.price == order_book.current_best_price(OrderBid::Ask)()}
      // Calculate weight (weight = mo.volume /. std::size(best_los))
      break;
    case random_selection:
      break;
  }
};
