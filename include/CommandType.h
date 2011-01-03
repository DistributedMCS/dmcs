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
 * @file   CommandType.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Wed Jan 25 2010
 *
 * @brief 
 *
 *
 */

#ifndef COMMAND_TYPE_H
#define COMMAND_TYPE_H

#include <vector>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "BeliefState.h"
//#include "DynamicConfiguration.h"

namespace dmcs {

template<typename MessageType, typename RetVal>
class CommandType
{
public:
  typedef MessageType input_type;
  typedef RetVal value_type;
  typedef boost::shared_ptr<value_type> return_type;

  return_type
  execute(input_type& mess);

  virtual bool
  continues(input_type& mess) = 0;
};


template<typename MessageType>
class CommandType<MessageType, bool>
{
public:
  typedef MessageType input_type;
  typedef bool value_type;
  typedef bool return_type;

  return_type
  execute(input_type& mess);

  virtual bool
  continues(input_type& mess) = 0;
};



  // this is for async commands
template<typename MessageType>
class CommandType<MessageType, void>
{
public:
  typedef MessageType input_type;
  typedef void value_type;
  typedef void return_type;

  return_type
  execute(input_type& mess);

  virtual bool
  continues(input_type& mess) = 0;
};





} // namespace dmcs

#endif // COMMAND_TYPE_H

// Local Variables:
// mode: C++
// End:
