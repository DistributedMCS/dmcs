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
 * @file   DLVResultParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  12 12:28:40 2012
 * 
 * @brief  
 * 
 * 
 */

#include <sstream>
#include <iostream>

#include "parser/DLVResultParser.h"
#include "parser/ParserState.h"
#include "parser/DLVResultGrammar.h"

namespace dmcs {

DLVResultParser::DLVResultParser(std::size_t cid, BeliefTablePtr b)
  : ctx_id(cid),
    btab(b)
{ }

void
DLVResultParser::parse(std::istream& is,
		       BeliefStateAdder adder)
{
  ParserState state(ctx_id, btab, adder);
  
  typedef std::string::const_iterator forward_iterator_t;
  DLVResultGrammar<forward_iterator_t> grammar(state);
  std::size_t errors = 0;

  do
    {
      std::string input;
      std::getline(is, input);
      if (input.empty() || is.bad())
	{
	  std::cerr << "leaving loop because got input size " << input.size() 
		    << ", stream bits fail " << is.fail() << ", bad " << is.bad() 
		    << ", eof " << is.eof() << std::endl;
	}
      
      forward_iterator_t fwd_begin = input.begin();
      forward_iterator_t fwd_end = input.end();

      try
	{
	  bool r = qi::phrase_parse(fwd_begin, fwd_end, grammar, ascii::space);
	  
	  // @todo: add better error message with position iterator 
	  if (!r || fwd_begin != fwd_end)
	    {
	      std::cerr << "for input '" << input << "': r=" << r 
			<< " (begin!=end)=" << (fwd_begin != fwd_end) << std::endl;
	      errors++;
	    }
	}
      catch(const qi::expectation_failure<forward_iterator_t>& e)
	{
	  std::cerr << "for input '" << input 
		    << "': could not parse DLV output(expectation failure) " << e.what_ << std::endl;
	  errors++;
	}

    }
  while (errors < 20);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
