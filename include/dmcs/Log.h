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
 * @file   Log.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sat Jan  8 02:17:19 2011
 * 
 * @brief  log facility
 * 
 * 
 */

#ifndef _DMCS_LOG_H
#define _DMCS_LOG_H

#include <log4cxx/logger.h>

namespace dmcs {

  /// the root logger
  extern log4cxx::LoggerPtr logger;


  /** 
   * Initializes all loggers. Must be called exactly once in every
   * program.
   * 
   * @param name logger name
   */
  void
  init_loggers(const char* name);


} // namespace dmcs

#endif // _DMCS_LOG_H

// Local Variables:
// mode: C++
// End:
