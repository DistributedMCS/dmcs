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

struct handle_int
{
  BeliefStatePtr V;
  ProxySignatureByLocal local_sig;

  handle_int(const BeliefStatePtr V_, const ProxySignatureByLocal local_sig_) 
    : V(V_), local_sig(local_sig_)
  { 
    assert(V->size() > 0);
  }


  void
  operator() (int i, qi::unused_type, qi::unused_type) const
  {
    // we only need to take care of positive answer
    if (i > 0)
      {
	// query the signature to get the context it comes from as
	// well as its original id
	SignatureByLocal::const_iterator loc_it = local_sig.find(i);

	// it must show up in the Signature
	assert(loc_it != local_sig.end());

	// then set the bit in the right context, at the right position
	std::size_t cid = loc_it->ctxId - 1;
	std::size_t oid = loc_it->origId;

	BeliefSet& b = V->at(cid);
	b = setBeliefSet(b, oid, true);

	// turn on the epsilon bit of the neighbor here
	b = setEpsilon(b);
      }
  }

};


struct handle_model
{
  bool& got_answer;

  handle_model(bool& got_answer_)
    : got_answer(got_answer_)
  { }

  void
  operator() (qi::unused_type, qi::unused_type, qi::unused_type) const
  {
    got_answer = true;
  }
};


template <typename Iterator>
struct ClaspResultOnlineGrammar : qi::grammar<Iterator, ascii::space_type >
{
  qi::rule<Iterator, ascii::space_type > sentinel;
  qi::rule<Iterator, int(), ascii::space_type > literal;
  qi::rule<Iterator, ascii::space_type > partial_model;
  qi::rule<Iterator, ascii::space_type > solution;
  qi::rule<Iterator, ascii::space_type > comment;
  qi::rule<Iterator, ascii::space_type > start;

  BeliefStatePtr V;
  ProxySignatureByLocal local_sig;
  bool got_answer;


  ClaspResultOnlineGrammar(BeliefStatePtr V_, ProxySignatureByLocal local_sig_)
    : ClaspResultOnlineGrammar::base_type(start), 
      V(V_), local_sig(local_sig_), got_answer(false)
  {
    sentinel = qi::char_('0');
    
    literal = qi::int_ - sentinel;

    // a partial model can be a model, and when it is the case, we
    // turn got_answer to true
    partial_model = qi::char_('v') >> +literal[handle_int(V, local_sig)] 
				   >> -(sentinel[handle_model(got_answer)]);

    solution = qi::char_('s') >> (qi::string("SATISFIABLE") |
				  qi::string("UNSATISFIABLE") | 
				  qi::string("UNKNOWN"));

    comment = qi::char_('c') >> *(qi::char_ - qi::eol);

    start = partial_model | comment | solution;
  }
};


class ClaspResultOnlineParser
{
public:
  ClaspResultOnlineParser(std::istream& is_, ProxySignatureByLocal local_sig_, std::size_t system_size_)
    : is(is_), local_sig(local_sig_), system_size(system_size_)
  { }
  
  bool
  hasAnswerLeft();
  
  BeliefStatePtr
  getNextAnswer()
  {
    return bs;
  }

private:
  std::istream& is;
  BeliefStatePtr bs;
  ProxySignatureByLocal local_sig;
  std::size_t system_size;
};

} // namespace dmcs

#endif // CLASP_RESULT_ONLINE_PARSER_H

// Local Variables:
// mode: C++
// End:
