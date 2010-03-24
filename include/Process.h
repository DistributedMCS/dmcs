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
 * @file   Process.h
 * @author Thomas Krennwallner
 * @date   
 * 
 * @brief  
 * 
 * 
 */


#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <vector>


namespace dmcs {

// forward declaration
class BaseSolver;

/**
 * @brief Base class for solver processes
 */
class Process
{
public:
  virtual
  ~Process() { }

  virtual BaseSolver*
  createSolver() = 0;

  virtual void
  addOption(const std::string&) = 0;

  virtual std::string
  path() const = 0;

  virtual std::vector<std::string>
  commandline() const = 0;

  virtual void
  spawn() = 0;

  virtual void
  spawn(const std::vector<std::string>&) = 0;

  virtual void
  endoffile() = 0;

  virtual int
  close() = 0;

  virtual std::ostream&
  getOutput() = 0;

  virtual std::istream&
  getInput() = 0;
};

} // namespace dmcs

#endif // PROCESS_H


// Local Variables:
// mode: C++
// End:
