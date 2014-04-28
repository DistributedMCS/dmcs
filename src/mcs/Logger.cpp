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
 * @file   Logger.cpp
 * @author Peter Schueller <ps@kr.tuwien.ac.at>
 * 
 * @brief  Implementation of logging facility.
 */

#include "mcs/Logger.h"

namespace
{
  Logger* instance = 0;
}

Logger& Logger::Instance()
{
  if( instance == 0 )
    instance = new Logger();
  return *instance;
}

void Logger::setPrintLevels(Levels levels)
{
  if( levels & ERROR != ERROR )
    out << "Logger warning: deactivated ERROR level" << std::endl;
  printlevels = levels;
}

void Logger::setPrintLevelWidth(int width)
{
  assert(width >= 0 );
  levelwidth = width;
}

// Local Variables:
// mode: C++
// End:
