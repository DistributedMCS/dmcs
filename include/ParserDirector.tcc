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
 * @file   ParserDirector.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:59:47 2009
 * 
 * @brief  
 * 
 * 
 */


#if !defined(_PARSER_DIRECTOR_TCC)
#define _PARSER_DIRECTOR_TCC

#include "BaseBuilder.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_common.hpp>


namespace dmcs {

template<typename Grammar>
ParserDirector<Grammar>::ParserDirector()
  : builder(0)
{
}

template<typename Grammar>
void
ParserDirector<Grammar>::parse(const std::string& file)
{
  std::ifstream ifs;
  
  ifs.open(file.c_str());
  
  if (!ifs.is_open())
    {
      std::ostringstream oss;
      oss << "File " << file << " not found.";
      throw std::runtime_error(oss.str());
    }
  else
    {
      parse(ifs);
      ifs.close();
    }
}

template<typename Grammar>
void
ParserDirector<Grammar>::parse(std::istream& is)
{
  assert (builder != 0);

  ///@todo TK: do not read the whole input, do it streamable
  // put whole input from stream into a string
  // (an alternative would be the boost::spirit::multi_pass iterator
  // but it was not possible to setup/simply did not compile)
  std::ostringstream buf;
  buf << is.rdbuf();
  std::string input = buf.str();

#ifdef DEBUG
  std::cerr << std::endl;
  std::cerr << "parsing input (size=" << input.size() << "):" << std::endl;
  std::cerr << input << std::endl;
#endif //DEBUG

  typename BaseBuilder<Grammar>::iterator_t it_begin(input.c_str(), input.c_str()+input.size());
  typename BaseBuilder<Grammar>::iterator_t it_end;

  // parse ast
  BOOST_SPIRIT_CLASSIC_NS::tree_parse_info<typename BaseBuilder<Grammar>::iterator_t, typename BaseBuilder<Grammar>::factory_t> info =
    BOOST_SPIRIT_CLASSIC_NS::pt_parse<typename BaseBuilder<Grammar>::factory_t>(it_begin,
										it_end,
										builder->getGrammar(),
										BOOST_SPIRIT_CLASSIC_NS::space_p);

  // successful parse?
  if(!info.full)
    {
      std::ostringstream oss;
      oss << "Could not parse complete input, stopped at " 
	  << info.stop.get_position().line << ":" << info.stop.get_position().column
	  << std::endl;

      throw std::runtime_error(oss.str());
    }
  else
    {
      assert(info.trees.size() == 1);

      typename BaseBuilder<Grammar>::node_t& node = *info.trees.begin();

      // node is from "root" rule
      assert(node.value.id() == Grammar::Root);

      for (typename BaseBuilder<Grammar>::node_t::tree_iterator it = node.children.begin();
       	   it != node.children.end(); ++it)
       	{
       	  builder->buildNode(*it);
       	}

      ///@todo make sure that the results from the parsing are unique in case of using the equi CNF builder
      // order the list of the beliefstates
      // then make unique
    }
}

} // namespace dmcs 

#endif // PARSER_DIRECTOR_TCC

// Local Variables:
// mode: C++
// End:

