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
 * @file   ClaspResultOnlineParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jul  30 14:34:21 2010
 * 
 * @brief  
 * 
 * 
 */

#include "ClaspResultOnlineParser.h"

namespace dmcs {

bool
ClaspResultOnlineParser::hasAnswerLeft()
{
  std::string line;
  BeliefStatePtr tmp(new BeliefState(system_size, 0));

  typedef std::string::const_iterator forward_iterator_type;
  ClaspResultOnlineGrammar<forward_iterator_type> crog(tmp, local_sig);

  while (!crog.got_answer && !is.eof())
    {
      std::getline(is, line);

      if (!line.empty())
	{

#ifdef DEBUG
	  std::cerr << "Processing line: \"" << line << "\""<< std::endl; 
#endif

	  forward_iterator_type beg = line.begin();
	  forward_iterator_type end = line.end();
	  
	  bool succeeded = boost::spirit::qi::phrase_parse(beg, end, crog, boost::spirit::ascii::space);
	  
	  assert (succeeded);
	  
	  if (crog.got_answer)
	    {
#ifdef DEBUG
	      std::cerr << "Got a complete answer: ";
	      std::copy(tmp->begin(), tmp->end(), std::ostream_iterator<BeliefSet>(std::cerr, " "));
	      std::cerr << std::endl;
#endif
	      bs = tmp;
	      return true;
	    }
#ifdef DEBUG
	  else
	    {
	      std::cerr << "Got a partial answer, or a (comment), or a (solution): ";
	      std::copy(tmp->begin(), tmp->end(), std::ostream_iterator<BeliefSet>(std::cerr, " "));
	      std::cerr << std::endl;
	    }
#endif
	}
    }

  return false;
}

} // namespace dmcs
