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
ClaspResultGrammar::definition<ScannerT>::definition(ClaspResultGrammar const& self)
{
  namespace spirit = BOOST_SPIRIT_CLASSIC_NS;

  // shortcut for boost::spirit::discard_node_d() const
  spirit::node_parser_gen<spirit::discard_node_op>
  rm =
  spirit::node_parser_gen<spirit::discard_node_op>();

  neg = spirit::ch_p('-');

  sentinel = spirit::token_node_d[!neg >> spirit::str_p(self.maxvariable.c_str())];

  var = spirit::token_node_d[spirit::int_p] - sentinel;

  literal = var;

  value =
    *(rm[spirit::ch_p('v')] >> (+literal % rm[spirit::ch_p('v')])) >>
    rm[spirit::ch_p('v')] >> *literal >> sentinel; // !neg >> sentinel may be removed for '0'

  solution =
    spirit::ch_p('s') >> (spirit::str_p("SATISFIABLE") |
			  spirit::str_p("UNSATISFIABLE") |
			  spirit::str_p("UNKNOWN"));

  comments = spirit::comment_p("c");

  root =
    *(rm[comments] | value) >>
    rm[solution] >>
    *(rm[comments] | value) >>
    !spirit::end_p;
}

} // namespace dmcs

#endif // _CLASP_RESULT_GRAMMAR_TCC

// Local Variables:
// mode: C++
// End:
