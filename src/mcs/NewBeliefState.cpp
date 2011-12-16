#include "mcs/NewBeliefState.h"

namespace dmcs {

std::ostream&
NewBeliefState::print(std::ostream& os) const
{
  std::size_t count = status_bit.count();
  if (count == 0)
    {
      os << "[ ]";
      return os;
    }

  os << "[";
  std::size_t bit = status_bit.get_first();
  do
    {
      if (!value_bit.test(bit))
	{
	  os << "-";
	}
      else
	{
	  os << " ";
	}
      os << bit;

      bit = status_bit.get_next(bit);
      
      if (bit)
	{
	  os << ", ";
	}
    }
  while (bit);

  return os << "]";
}

} // namespace dmcs
