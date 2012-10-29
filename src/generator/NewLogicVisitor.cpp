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
 * @file   NewLogicVisitor.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Oct 29 2012
 * 
 * @brief
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "generator/NewLogicVisitor.h"

#include <set>
#include <iostream> 

using namespace dmcs;


NewLogicVisitor::NewLogicVisitor(std::ostream& s)
  : stream(s)
{ }


std::ostream&
NewLogicVisitor::getStream()
{
  return stream;
}


std::string
NewLogicVisitor::atom_name(const std::size_t contextId, const std::size_t atomId, const BeliefTablePtr& sigma)
{
  ID id(contextId | ID::MAINKIND_BELIEF, atomId);
  const Belief& b = sigma->getByID(id);
  return b.text;
}


void
NewLogicVisitor::visitRule(const RulePtr& t, const std::size_t contextId, const BeliefTablePtr& sigma)
{
  bool multiple = false;

  const Head& h = getHead(t);
  for (Head::const_iterator it = h.begin(); it != h.end(); ++it)
    {
      if (multiple)
	{
	  stream << "v ";
	}
      
      
      stream << atom_name(contextId, *it, sigma);

      multiple = true;
    }  

  stream << ":- ";

  multiple = false;
  const PositiveBody& pb = getPositiveBody(t);
  for (PositiveBody::const_iterator it = pb.begin(); it != pb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      stream << atom_name(contextId, *it, sigma);
      multiple = true;      
    }
  
  const NegativeBody& nb = getNegativeBody(t);
  for (NegativeBody::const_iterator it = nb.begin(); it != nb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      stream << "not " << atom_name(contextId, *it, sigma);
      multiple = true;     
    }
  stream << "." << std::endl;
}



void
NewLogicVisitor::visitBridgeRule(const BridgeRulePtr& t, const std::size_t contextId, const BeliefTableVecPtr& sigma_vec)
{
  bool multiple = false;

  const Head& h = getHead(t);
  for (Head::const_iterator it = h.begin(); it != h.end(); ++it)
    {
      if (multiple)
	{
	  stream << "v ";
	}
      stream << atom_name(contextId, *it, (*sigma_vec)[contextId]);
      multiple = true;
    }

  stream << ":- ";      

  multiple = false;
  const PositiveBridgeBody& pb = getPositiveBody(t);
  for (PositiveBridgeBody::const_iterator it = pb.begin(); it != pb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      printPositiveBridgeRule(stream, *it, sigma_vec);
      multiple = true;      
    }
 
  const NegativeBridgeBody& nb = getNegativeBody(t);
  for (NegativeBridgeBody::const_iterator it = nb.begin(); it != nb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      printNegativeBridgeRule(stream, *it, sigma_vec);
      multiple = true;     
    }

  stream << "." << std::endl;
}

void
NewLogicVisitor::printPositiveBridgeRule(std::ostream& os, const BridgeAtom& b, const BeliefTableVecPtr& sigma_vec)
{
  os << "(" << b.first << ":" << atom_name(b.first, b.second, (*sigma_vec)[b.first]) << ")";
}

void
NewLogicVisitor::printNegativeBridgeRule(std::ostream& os, const BridgeAtom& b, const BeliefTableVecPtr& sigma_vec)
{
  os << "not (" << b.first << ":" << atom_name(b.first, b.second, (*sigma_vec)[b.first]) << ")";
}
