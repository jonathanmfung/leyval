#include <cstdlib>

namespace leyval {
class Money
{
public:
  Money(int main_unit, int sub_unit)
    : m_main{ main_unit }
    , m_sub{ sub_unit }
  {
  }

  [[nodiscard]] int get_main() const { return m_main; }
  [[nodiscard]] int get_sub() const { return m_sub; }

private:
  int m_main;
  int m_sub;
};

Money
operator+(const Money& m1, const Money& m2);

} // namespace leyval
