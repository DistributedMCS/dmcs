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
 * @file   LogicVisitor.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Fri Jan 15 2010
 * 
 * @brief
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "Context.h"
#include "LogicVisitor.h"
#include <set>
#include <iostream> 

using namespace dmcs;


LogicVisitor::LogicVisitor(std::ostream& s)
  : stream(s)
{ }


std::ostream&
LogicVisitor::getStream()
{
  return stream;
}

void
LogicVisitor::visitRules(const ContextPtr& context)
{
  for (Rules::const_iterator it = context->getLocalKB()->begin(); it != context->getLocalKB()->end(); ++it)
    {
      visitRule(*it,context->getContextID());
    }
}

void
LogicVisitor::visitRule(const RulePtr& t,const std::size_t contextId )
{
  bool multiple = false;

  const Head& h = getHead(t);
  for (Head::const_iterator it = h.begin(); it != h.end(); ++it)
    {
      if (multiple)
	{
	  stream << "v ";
	}
      stream << (char)((*it)+96) << "" << contextId << " ";
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
      stream << (char)((*it)+96) << "" << contextId ;
      multiple = true;      
    }
  
  const NegativeBody& nb = getNegativeBody(t);
  for (NegativeBody::const_iterator it = nb.begin(); it != nb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      stream << "not " << (char)((*it)+96)<< "" << contextId;
      multiple = true;     
    }
  stream << "." << std::endl;
}


void
LogicVisitor::visitBridgeRules(const ContextPtr& context)
{
  for (BridgeRules::const_iterator it = context->getBridgeRules()->begin(); it != context->getBridgeRules()->end(); ++it)
    {
      visitBridgeRule(*it, context->getContextID());
    }
}


void
LogicVisitor::visitBridgeRule(const BridgeRulePtr& t, const std::size_t contextId)
{
  bool multiple = false;

  const Head& h = getHead(t);
  for (Head::const_iterator it = h.begin(); it != h.end(); ++it)
    {
      if (multiple)
	{
	  stream << "v ";
	}
      stream << (char)((*it)+96)<< ""<< contextId << " ";
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
      printPositiveBridgeRule(stream,*it);
      //      stream << "(" << it->first << ":" << (char)(it->second+96)<< "" << it->first << ")";
      multiple = true;      
    }
 
  const NegativeBridgeBody& nb = getNegativeBody(t);
  for (NegativeBridgeBody::const_iterator it = nb.begin(); it != nb.end(); ++it)
    {
      if (multiple)
	{
	  stream << ", ";
	}
      printNegativeBridgeRule(stream,*it);
      //      stream << "not (" << it->first << ":" << (char)(it->second+96)<< "" << it->first << ")";
      multiple = true;     
    }

  stream << "." << std::endl;
}

void
LogicVisitor::printPositiveBridgeRule(std::ostream& os,const BridgeAtom& b)
{
  os << "(" << b.first << ":" << (char)(b.second+96)<< "" << b.first << ")";
}

void
LogicVisitor::printNegativeBridgeRule(std::ostream& os,const BridgeAtom& b)
{
  os << "not (" << b.first << ":" << (char)(b.second+96)<< "" << b.first << ")";
}
