#pragma once

#include <stdexcept>

#include "order.hpp"
#include "order_book.hpp"
#include "serializable.hpp"

struct TransactionRequest
{
  int bidder_id;
  int asker_id;
  int volume;
  Money price;

  TransactionRequest(int initiator_agent,
                     int provider_agent,
                     int _volume,
                     Money _price,
                     OrderDir market_order_dir)
    : volume{ _volume }
    , price{ _price }
  {
    switch (market_order_dir) {
      case OrderDir::Bid:
        bidder_id = initiator_agent;
        asker_id = provider_agent;
        break;
      case OrderDir::Ask:
        bidder_id = provider_agent;
        asker_id = initiator_agent;
        break;
      default:
        throw OrderDirInvalidValue("TransactionRequest()");
    }
  }
};

// TransactionRequest(initiator_agent, provider_agent, volume, price, order_dir)

template<>
struct fmt::formatter<TransactionRequest> : fmt::formatter<std::string_view>
{
  auto format(const TransactionRequest& treq, format_context& ctx) const -> format_context::iterator;
};


//////////////////////////////////////////////////////////

// TODO: Think about splitting MatchingSystem to concept or abstract base class
class MatchingSystem
{
public:
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

  std::vector<TransactionRequest> operator()(const MarketOrderReq mo,
                                             OrderBook& order_book);

  [[nodiscard]] Type get_type() const { return m_type; }
  [[nodiscard]] std::string get_type_string() const
  {
    switch (m_type) {
      case fifo:
        return "FIFO";
      case pro_rata:
        return "Pro_Rata";
      case random_selection:
        return "RSS";
      default:
        throw std::invalid_argument("MatchingSystem::get_type_string()");
    }
  }

private:
  Type m_type;
};

template<>
struct fmt::formatter<MatchingSystem> : fmt::formatter<std::string_view>
{
  auto format(const MatchingSystem& match_sys, format_context& ctx) const -> format_context::iterator;
};
