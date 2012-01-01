/* DMCS -- Distributed Nonmonotonic Multi-Context Systems.
 * Copyright (C) 2009, 2010 Minh Dao-Tran, Thomas Krennwallner
 * 
 * This file is part of DMCS.
 *
 *  DMCS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DMCS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DMCS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   BackwardMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 15:44:21 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef BACKWARD_MESSAGE_H
#define BACKWARD_MESSAGE_H

#include "mcs/Printhelpers.h"
#include "mcs/NewMessage.h"
#include "mcs/ReturnedBeliefState.h"

namespace dmcs {

struct BackwardMessage : 
    public NewMessage,
    private ostream_printable<BackwardMessage>
{
  BackwardMessage();

  virtual
  ~BackwardMessage(); 

  BackwardMessage(const BackwardMessage& bMess);

  BackwardMessage(const ReturnedBeliefStateListPtr& r);

  template <typename Archive>
  void
  serialize(Archive& ar, const unsigned int /* version */);

  std::ostream&
  print(std::ostream& os) const;

  ReturnedBeliefStateListPtr rbsl;
  ///TODO: add statistical information
};

} // namespace dmcs



#endif // BACKWARD_MESSAGE_H

// Local Variables:
// mode: C++
// End:
