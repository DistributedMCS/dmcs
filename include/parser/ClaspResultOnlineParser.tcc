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
 * @file   ClaspResultOnlineParser.tcc
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Mon Aug  2 14:48:36 2010
 * 
 * @brief  
 * 
 * 
 */


#if !defined(_CLASP_RESULT_ONLINE_PARSER_TCC)
#define _CLASP_RESULT_ONLINE_PARSER_TCC


namespace dmcs {

struct handle_int
{
  BeliefStatePtr& belief_state;
  ProxySignatureByLocal& mixed_sig;


  handle_int(BeliefStatePtr& b, ProxySignatureByLocal& m) 
    : belief_state(b), mixed_sig(m)
  { 
    assert(belief_state->size() > 0);
  }


  void
  operator() (int i, qi::unused_type, qi::unused_type) const
  {
    // we only need to take care of positive answer
    if (i > 0)
      {
	// query the signature to get the context it comes from as
	// well as its original id
	SignatureByLocal::const_iterator loc_it = mixed_sig.find(i);

	// it must show up in the Signature
	assert(loc_it != mixed_sig.end());

	// then set the bit in the right context, at the right position
	std::size_t cid = loc_it->ctxId - 1;
	std::size_t oid = loc_it->origId;

	BeliefSet& b = belief_state->at(cid);
	b.set(oid, true);

	// turn on the epsilon bit of the neighbor here
	setEpsilon(b);
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


template<typename Iterator>
ClaspResultOnlineGrammar<Iterator>::ClaspResultOnlineGrammar(const ProxySignatureByLocal& m, std::size_t system_size)
  : ClaspResultOnlineGrammar::base_type(start), 
    belief_state(new BeliefState(system_size, 0)),
    mixed_sig(m), got_answer(false)
{
  sentinel = qi::char_('0');
  
  literal = qi::int_ - sentinel;
  
  // a partial model can be a model, and when it is the case, we
  // turn got_answer to true
  partial_model = qi::char_('v') >> +literal[handle_int(belief_state, mixed_sig)] 
				 >> -(sentinel[handle_model(got_answer)]);
  
  solution = qi::char_('s') >> (qi::string("SATISFIABLE") |
				qi::string("UNSATISFIABLE") | 
				qi::string("UNKNOWN"));
  
  comment = qi::char_('c') >> *(qi::char_ - qi::eol);
  
  start = partial_model | comment | solution;
}


} // namespace dmcs

#endif // _CLASP_RESULT_ONLINE_PARSER_TCC

// Local Variables:
// mode: C++
// End:

