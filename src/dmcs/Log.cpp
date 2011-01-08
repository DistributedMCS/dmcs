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
 * @file   Log.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sat Jan  8 02:20:59 2011
 * 
 * @brief  
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "dmcs/Log.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>


namespace dmcs {

  log4cxx::LoggerPtr logger;


  void
  init_loggers(const char* name)
  {
    logger = log4cxx::Logger::getLogger(name);

    // setup log4cxx
    logger->setLevel(log4cxx::Level::getDebug());
    log4cxx::BasicConfigurator::configure();
  }

}

// Local Variables:
// mode: C++
// End:
