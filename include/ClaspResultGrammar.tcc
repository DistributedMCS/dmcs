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
 * @file   ClaspResultGrammar.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:43:25 2009
 * 
 * @brief  
 * 
 * 
 */

#if !defined(_CLASP_RESULT_GRAMMAR_TCC)
#define _CLASP_RESULT_GRAMMAR_TCC

namespace dmcs {

template<typename ScannerT>
ClaspResultGrammar::definition<ScannerT>::definition(ClaspResultGrammar const&)
{
  // shortcut for boost::spirit::discard_node_d() const
  BOOST_SPIRIT_CLASSIC_NS::node_parser_gen<BOOST_SPIRIT_CLASSIC_NS::discard_node_op>
  rm =
  BOOST_SPIRIT_CLASSIC_NS::node_parser_gen<BOOST_SPIRIT_CLASSIC_NS::discard_node_op>();

  neg_ = BOOST_SPIRIT_CLASSIC_NS::ch_p('-');

  number_ =
    BOOST_SPIRIT_CLASSIC_NS::token_node_d[+BOOST_SPIRIT_CLASSIC_NS::digit_p];

  dimacs_atom_ = !neg_ >> number_;

  clasp_answer_ = rm[BOOST_SPIRIT_CLASSIC_NS::ch_p('v')] >> +dimacs_atom_;

  conclusion_ = BOOST_SPIRIT_CLASSIC_NS::ch_p('s') >>
  (BOOST_SPIRIT_CLASSIC_NS::str_p("SATISFIABLE") |
  BOOST_SPIRIT_CLASSIC_NS::str_p("UNSATISFIABLE"));

  root_ = *(rm[BOOST_SPIRIT_CLASSIC_NS::comment_p("c")] | clasp_answer_) >>
    rm[conclusion_] >> *(rm[BOOST_SPIRIT_CLASSIC_NS::comment_p("c")]) >>
    !BOOST_SPIRIT_CLASSIC_NS::end_p;
}

} // namespace dmcs

#endif // _CLASP_RESULT_GRAMMAR_TCC

// Local Variables:
// mode: C++
// End:
