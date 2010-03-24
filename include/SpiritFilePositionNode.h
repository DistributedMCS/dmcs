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
 * @file   SpiritFilePositionNode.h
 * @author Peter Schüller
 * @date   Sat Jul  4 22:50:28 CEST 2009
 * 
 * @brief  node/factory for storing boost::spirit::file_position in boost::spirit nodes
 * 
 */

#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/classic_parse_tree.hpp>

#ifndef _MCS_SPIRIT_FILE_POSITION_NODE_H_
#define _MCS_SPIRIT_FILE_POSITION_NODE_H_


// data we want to store in the parse tree
struct FilePositionNodeData
{
  // where was the match to this node?
  BOOST_SPIRIT_CLASSIC_NS::file_position pos;

  FilePositionNodeData(): pos() {}
  FilePositionNodeData& operator=(const FilePositionNodeData& n)
   { pos = n.pos; return *this; }
};

// Factory which automatically sets the position (adapted from spirit sources)
//
// ValueT must be some FilePositionNodeData
template<typename ValueT>
class FilePositionNodeFactory
{
public:
  // IteratorT must be some boost::spirit::position_iterator
  template<typename IteratorT>
  class factory
  {
  public:
    typedef IteratorT iterator_t;
    typedef BOOST_SPIRIT_CLASSIC_NS::node_val_data<iterator_t, ValueT> node_t;

    // no other way to do this in interface
    // (no non-const ValueT& value() method)
    static void setPosition(node_t& node,
        const BOOST_SPIRIT_CLASSIC_NS::file_position& pos)
    {
      ValueT val = node.value(); // get
      val.pos = pos; // modify
      node.value(val); // set
    }

    static node_t create_node(
        iterator_t const& first, iterator_t const& last,
        bool is_leaf_node)
    {
      if (is_leaf_node)
      {
        node_t ret(first, last);
        setPosition(ret, first.get_position());
        return ret;
      }
      else
      {
        node_t ret;
        setPosition(ret, first.get_position());
        return ret;
      }
    }

    static node_t empty_node()
    {
      return node_t();
    }

    template <typename ContainerT>
    static node_t group_nodes(ContainerT const& nodes)
    {
      typename node_t::container_t c;
      typename ContainerT::const_iterator i_end = nodes.end();
      // copy all the nodes text into a new one
      for (typename ContainerT::const_iterator i = nodes.begin();
           i != i_end; ++i)
      {
          // See docs: token_node_d or leaf_node_d cannot be used with a
          // rule inside the [].
          assert(i->children.size() == 0);
          c.insert(c.end(), i->value.begin(), i->value.end());
      }
      node_t ret(c.begin(), c.end());
      setPosition(ret, nodes.begin()->value.value().pos);
      return ret;
    }
  };
};

#endif // _MCS_SPIRIT_FILE_POSITION_NODE_H_

// Local Variables:
// mode: C++
// End:
