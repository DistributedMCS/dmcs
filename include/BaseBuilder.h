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
 * @file   BaseBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:33:26 2009
 * 
 * @brief  
 * 
 * 
 */


#ifndef BASE_BUILDER_H_
#define BASE_BUILDER_H_

#include <cstdlib>
#include <iostream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

#include "SpiritFilePositionNode.h"


namespace dmcs {

/**
 * @brief Base class for builders.
 */
template<class Grammar>
class BaseBuilder
{
protected:
  /// a generic Grammar
  Grammar grammar;

  /// pass Grammar as parameter
  BaseBuilder(const Grammar& g) : grammar(g) { }

  /// default ctor if Grammar has a default ctor
  BaseBuilder() : grammar() { }

public:
  typedef FilePositionNodeFactory<FilePositionNodeData> factory_t;
  
  // iterator which remembers file positions (useful for error messages)
  typedef BOOST_SPIRIT_CLASSIC_NS::position_iterator<const char*> iterator_t;

  // node type for spirit PT
  typedef BOOST_SPIRIT_CLASSIC_NS::tree_match<iterator_t, factory_t>::node_t node_t;

  // build up AST from node node
  virtual void
  buildNode (node_t& node) = 0;

  virtual Grammar&
  getGrammar()
  {
    return grammar;
  }

  std::string 
  createStringFromNode (node_t& node)
  {
    // verify the tag
    // assert(verifyRuleTag == SatGrammar::None || node.value.id() == verifyRuleTag);

    // descend as long as there is only one child and the node has no value
    node_t& at = node;

    while ((at.children.size() == 1) && (at.value.begin() == at.value.end()) )
      {
	at = at.children[0];
      }

    // if we find one child which has a value, we return it
    if (at.value.begin() != at.value.end())
      {
	std::string ret(at.value.begin(), at.value.end());
	boost::trim(ret);
	return ret;
      }

    // if we find multiple children which have a value, this is an error
    assert(false && "found multiple value children in createStringFromNode");
  }  
};


} // namespace dmcs

#endif // _BASE_BUILDER_H_

// Local Variables:
// mode: C++
// End:
