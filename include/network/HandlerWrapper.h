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
 * @file   HandlerWrapper.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Mar  15 9:45:03 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef HANDLER_WRAPPER_H
#define HANDLER_WRAPPER_H

#include "network/connection.hpp"
#include "network/NewHandler.h"
#include "mcs/Registry.h"

namespace dmcs {

class HandlerWrapper
{
public:
  HandlerWrapper()
  { }

  ~HandlerWrapper()
  { }

  void
  operator()(NewHandlerPtr handler,
	     connection_ptr conn,
	     NewConcurrentMessageDispatcherPtr md,
	     NewOutputDispatcherPtr od);
};

} // namespace dmcs

#endif // HANDLER_WRAPPER_H

// Local Variables:
// mode: C++
// End:
