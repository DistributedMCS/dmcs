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
 * @file   ClaspProcess.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:33:26 2009
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "ClaspProcess.h"
#include "SatSolver.h"
#include "DimacsVisitor.h"

#include <iostream>
#include <assert.h>

using namespace dmcs;


ClaspProcess::ClaspProcess()
  : proc(), ipipe(0), opipe(0)
{ }

ClaspProcess::~ClaspProcess()
{
  proc.close();
  delete opipe;
  delete ipipe;
}

BaseSolver*
ClaspProcess::createSolver()
{
  return new SatSolver<DimacsVisitor, ClaspResultBuilder<ClaspResultGrammar>, ClaspResultGrammar>(*this);
}

void
ClaspProcess::addOption(const std::string& option)
{
  argv.push_back(option);
}

std::string
ClaspProcess::path() const
{
  return CLASPPATH;
}

std::vector<std::string>
ClaspProcess::commandline() const
{
  std::vector<std::string> tmp;
  tmp.push_back(path());
  tmp.insert(tmp.end(), argv.begin(), argv.end());
  tmp.push_back("--"); ///@todo hm, strange, this seems to work

  return tmp;
}

void
ClaspProcess::setupStreams()
{
  if (ipipe == 0 && opipe == 0)
    {
      opipe = new std::iostream(&proc);
      ipipe = new std::iostream(&proc);

      opipe->exceptions(std::ios_base::badbit);
      ipipe->exceptions(std::ios_base::badbit);
    }
}

void
ClaspProcess::spawn()
{
  setupStreams();
  proc.open(commandline());
}

void
ClaspProcess::spawn(const std::vector<std::string>& opt)
{
  setupStreams();
  std::vector<std::string> tmp(opt);
  tmp.insert(tmp.begin(), path());
  proc.open(tmp);
}

void
ClaspProcess::endoffile()
{
  proc.endoffile();
}

int
ClaspProcess::close()
{
  assert(ipipe != 0 && opipe != 0);

  opipe->clear();
  ipipe->clear();

  return proc.close();
}

std::ostream&
ClaspProcess::getOutput()
{
  assert(opipe != 0);

  return *opipe;
}

std::istream&
ClaspProcess::getInput()
{
  assert(ipipe != 0);

  return *ipipe;
}
