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
 * @file   ClaspProcess.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:37:06 2009
 * 
 * @brief  
 * 
 * 
 */


#ifndef CLASP_PROCESS_H
#define CLASP_PROCESS_H

#include "Process.h"
#include "ProcessBuf.h"
#include "BaseSolver.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"

#include <iosfwd>
#include <string>
#include <vector>


namespace dmcs {

class ClaspProcess : public Process<ClaspResultBuilder<ClaspResultGrammar> >
{
protected:
  ProcessBuf proc;
  
  std::istream* ipipe;
  std::ostream* opipe;
  
  std::vector<std::string> argv;
  
  void setupStreams();
  
public:
  ClaspProcess();
  
  virtual
  ~ClaspProcess();

  virtual BaseSolver*
  createSolver(ClaspResultBuilder<ClaspResultGrammar>*);

  virtual void 
  addOption(const std::string&);

  virtual std::string 
  path() const;

  virtual std::vector<std::string> 
  commandline() const;

  virtual void 
  spawn();

  virtual void 
  spawn(const std::vector<std::string>&);

  virtual void 
  endoffile();

  virtual int 
  close();

  virtual std::ostream& 
  getOutput();

  virtual std::istream& 
  getInput();    
};

} // namespace dmcs

#endif // CLASP_PROCESS_H

// Local Variables:
// mode: C++
// End:
