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
 * @file   ClaspResultOnlineParser.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jul  30 11:42:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef CLASP_RESULT_ONLINE_PARSER_H
#define CLASP_RESULT_ONLINE_PARSER_H

#include <boost/spirit/include/qi.hpp>

#include "BeliefState.h"
#include "ProxySignatureByLocal.h"
#include "Signature.h"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace dmcs {


/**
 * Parses single model at-a-time.
 */
template <typename Iterator>
struct ClaspResultOnlineGrammar : public qi::grammar<Iterator, ascii::space_type >
{
  qi::rule<Iterator, ascii::space_type > sentinel;
  qi::rule<Iterator, int(), ascii::space_type > literal;
  qi::rule<Iterator, ascii::space_type > partial_model;
  qi::rule<Iterator, ascii::space_type > solution;
  qi::rule<Iterator, ascii::space_type > comment;
  qi::rule<Iterator, ascii::space_type > start;

  BeliefStatePtr belief_state;	/**< the current partial belief state */
  ProxySignatureByLocal mixed_sig; /**< local signature + guessing signature */
  bool got_answer;		/**< true iff we parsed a whole partial belief state */

  /**
   * Ctor.
   */
  ClaspResultOnlineGrammar(const ProxySignatureByLocal& m, std::size_t system_size);

  /** 
   * Reset the model.
   */
  inline void
  reset()
  {
    // reset this flag
    got_answer = false;

    // reset the container for the model, all belief sets to zero
    belief_state->assign(belief_state->size(), (BeliefSet)0);
  }
};



/**
 * enumerate models.
 */
class ClaspResultOnlineParser
{
public:
  ClaspResultOnlineParser(std::istream& is_, ProxySignatureByLocal local_sig_, std::size_t system_size_)
    : is(is_), crog(local_sig_, system_size_)
  { }
  
  /** 
   *
   * @return the next BeliefStatePtr, or NULL
   */  
  BeliefStatePtr
  getNextAnswer();

private:
  std::istream& is;
  ClaspResultOnlineGrammar<std::string::const_iterator> crog;
};

} // namespace dmcs

#endif // CLASP_RESULT_ONLINE_PARSER_H


#include "ClaspResultOnlineParser.tcc"


// Local Variables:
// mode: C++
// End:
