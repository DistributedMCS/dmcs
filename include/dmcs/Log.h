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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <log4cxx/logger.h>

namespace dmcs {

  /// the root logger
  extern log4cxx::LoggerPtr logger;


  /** 
   * Initializes all loggers. Must be called exactly once in every
   * program.
   * 
   * @param name logger name
   * @param config config filename
   */
  void
  init_loggers(const char* name, const char* config = 0);


} // namespace dmcs



#if defined(DEBUG)
#define DMCS_LOG_TRACE(expr) LOG4CXX_TRACE(dmcs::logger, __PRETTY_FUNCTION__ << expr)
#define DMCS_LOG_DEBUG(expr) LOG4CXX_DEBUG(dmcs::logger, expr)
#else
#define DMCS_LOG_TRACE(expr) do {} while(0)
#define DMCS_LOG_DEBUG(expr) do {} while(0)
#endif // DEBUG

#define DMCS_LOG_INFO(expr)  LOG4CXX_INFO(dmcs::logger, expr)
#define DMCS_LOG_WARN(expr)  LOG4CXX_WARN(dmcs::logger, expr)
#define DMCS_LOG_ERROR(expr) LOG4CXX_ERROR(dmcs::logger, expr)
#define DMCS_LOG_FATAL(expr) LOG4CXX_FATAL(dmcs::logger, expr)


#endif // _DMCS_LOG_H

// Local Variables:
// mode: C++
// End:
