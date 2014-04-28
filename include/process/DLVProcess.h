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
 * @file   DLVProcess.h
 * @author Thomas Krennwallner
 * @date   
 * 
 * @brief  Process interface to DLV programs.
 * 
 * 
 */


#ifndef DLV_PROCESS_H
#define DLV_PROCESS_H

#include "process/NewProcess.h"
#include "process/NewProcessBuf.h"

#include <iosfwd>
#include <vector>
#include <string>
#include <cassert>


namespace dmcs {

/**
 * @brief A wrapper process for the DLV/DLVDB ASP engine.
 * @todo this class should have a name similar to DebuggingSupportChildProcess to really capture its purpose
 */
class DLVProcess : public Process
{
 protected:
  /// communication buffer
  NewProcessBuf proc;
  
  /// iostreams to the dlv process
  std::istream* ipipe;
  std::ostream* opipe;
  
  /// executable path/name
  std::string executable;

  /// command line options
  std::vector<std::string> argv;

  /// initialize in/out streams
  void
  setupStreams();
  
 public:
  DLVProcess();

  virtual
  ~DLVProcess();

  virtual void
  addOption(const std::string&);

  virtual void
  setPath(const std::string&);

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

  // wait for end of process
  // if kill is true, kill if not already ended
  virtual int
  close(bool kill=false);

  virtual std::ostream&
  getOutput();

  virtual std::istream&
  getInput();
};

} // namespace dmcs

#endif // _DLV_PROCESS_H


// Local Variables:
// mode: C++
// End:
