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
 * @file   PropositionalASPGrammar.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  30 11:52:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef PROPOSITIONAL_ASP_GRAMMAR_TCC
#define PROPOSITIONAL_ASP_GRAMMAR_TCC

#include <boost/spirit/include/qi_numeric.hpp>


namespace dmcs {

template<typename ScannerT>
PropositionalASPGrammar::definition<ScannerT>::definition(PropositionalASPGrammar const&)
{
  // shortcut for boost::spirit::discard_node_d() const
  BOOST_SPIRIT_CLASSIC_NS::node_parser_gen<BOOST_SPIRIT_CLASSIC_NS::discard_node_op>
  rm =
  BOOST_SPIRIT_CLASSIC_NS::node_parser_gen<BOOST_SPIRIT_CLASSIC_NS::discard_node_op>();

  num_
    = BOOST_SPIRIT_CLASSIC_NS::token_node_d[+BOOST_SPIRIT_CLASSIC_NS::digit_p];

  naf_ 
    = BOOST_SPIRIT_CLASSIC_NS::str_p("not");

  BOOST_SPIRIT_CLASSIC_NS::chset<> alnum_("a-zA-Z0-9_");

  atom_ =
    BOOST_SPIRIT_CLASSIC_NS::token_node_d[BOOST_SPIRIT_CLASSIC_NS::lower_p >> *alnum_];

  literal_
    = !naf_ >> atom_;

  disj_
    = atom_ >> *(rm[BOOST_SPIRIT_CLASSIC_NS::ch_p('v')] >> atom_);

  body_
    = literal_ >> *(rm[BOOST_SPIRIT_CLASSIC_NS::ch_p(',')] >> literal_);

  cons_
    = BOOST_SPIRIT_CLASSIC_NS::str_p(":-");

  rule_
    = disj_ >> !(rm[cons_] >> body_) >> rm[BOOST_SPIRIT_CLASSIC_NS::ch_p('.')];

  root_ = *(rm[BOOST_SPIRIT_CLASSIC_NS::comment_p("%")] | rule_) >>
    rm[!BOOST_SPIRIT_CLASSIC_NS::end_p];
}


} // namespace dmcs

#endif // PROPOSITIONAL_ASP_GRAMMAR_TCC

// Local Variables:
// mode: C++
// End:
