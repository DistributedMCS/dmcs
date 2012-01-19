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
 * @file   DLVProcess.cpp
 * @author Thomas Krennwallner
 * @date   
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "process/DLVProcess.h"

#include <iostream>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/filtering_stream.hpp>


namespace dmcs {

DLVProcess::DLVProcess()
  : proc(), ipipe(0), opipe(0), executable(), argv()
{ }


DLVProcess::~DLVProcess()
{
  proc.close();
  delete opipe;
  delete ipipe;
}

void
DLVProcess::addOption(const std::string& option)
{
  argv.push_back(option);
}


void
DLVProcess::setPath(const std::string& path)
{
  executable = path;
}

std::string
DLVProcess::path() const
{
  return executable;
}


std::vector<std::string>
DLVProcess::commandline() const
{
  std::vector<std::string> tmp;

  assert(!path().empty());
  tmp.push_back(path());
  tmp.insert(tmp.end(), argv.begin(), argv.end());

  return tmp;
}


void
DLVProcess::setupStreams()
{
  if (ipipe == 0 && opipe == 0)
    {
      opipe = new std::iostream(&proc);
      ipipe = new std::iostream(&proc);
      
      // let ProcessBuf throw std::ios_base::failure
      opipe->exceptions(std::ios_base::badbit);
      ipipe->exceptions(std::ios_base::badbit);
    }
}


void
DLVProcess::spawn()
{
  setupStreams();
  proc.open(commandline());
}


void
DLVProcess::spawn(const std::vector<std::string>& opt)
{
  setupStreams();
  std::vector<std::string> tmp(opt);
  assert(!path().empty());
  tmp.insert(tmp.begin(), path());
  proc.open(tmp);
}


void
DLVProcess::endoffile()
{
  proc.endoffile();
}


int
DLVProcess::close(bool kill)
{
  assert(ipipe != 0 && opipe != 0);

  // first reset the state of the iostreams in order to re-use them
  opipe->clear();
  ipipe->clear();
  // exit code of process
  return proc.close(kill);
}


std::ostream&
DLVProcess::getOutput()
{
  assert(opipe != 0);
  return *opipe;
}


std::istream&
DLVProcess::getInput()
{
  assert(ipipe != 0);
  return *ipipe;
}

} // namespace dmcs

// vim:se ts=8:
// Local Variables:
// mode: C++
// End:
