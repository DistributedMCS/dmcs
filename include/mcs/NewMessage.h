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
 * @file   Message.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  23 10:24:54 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_MESSAGE_H
#define NEW_MESSAGE_H

namespace dmcs {

#define HEADER_REQ_PRI_DMCS "DMCS REQ PRI_DMCS"
#define HEADER_REQ_OPT_DMCS "DMCS REQ OPT_DMCS"
#define HEADER_REQ_STM_DMCS "DMCS REQ STM_DMCS"
#define HEADER_REQ_DYN_DMCS "DMCS REQ DYN_DMCS"
#define HEADER_REQ_INSTANTIATE "DMCS REQ INSTANTIATE"
#define HEADER_ANS "DMCS ANS"
#define HEADER_EOF "DMCS EOF"
#define HEADER_NEXT "DMCS NEXT"
#define HEADER_TERMINATE "DMCS TERMINATE"


struct NewMessage
{
  virtual ~NewMessage() 
  {}

  // templates cant be virtual
  template<class Archive>
  void serialize(Archive & /* ar */, const unsigned int /* version */)
  {}
};

} // namespace dmcs

#endif // NEW_MESSAGE_H

// Local Variables:
// mode: C++
// End:
