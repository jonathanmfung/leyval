#include <concepts>
namespace leyval {
class Money
{
public:
  Money(int main_unit, int sub_unit)
    : m_main{ main_unit }
    , m_sub{ sub_unit }
  {
  }

private:
  int m_main;
  int m_sub;
};
static_assert(std::integral<Money>);
} // namespace leyval
