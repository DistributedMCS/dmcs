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
 * @file   BRGrammar.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sat Jan  01 17:44:24 2010
 * 
 * @brief  
 * 
 * @todo Reimplement using Boost Spirit 2
 * 
 */

#ifndef BR_GRAMMAR_H
#define BR_GRAMMAR_H

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_common.hpp>


namespace dmcs {


struct BRGrammar
  : public BOOST_SPIRIT_CLASSIC_NS :: grammar<BRGrammar>
{
  enum RuleTags
    {
      None = 0,
      Root,
      Naf,
      Cons,
      Num,
      Atom,
      BridgeAtom,
      BridgeLiteral,
      Disj,
      BridgeBody,
      BridgeRule
    };

  // S = ScannerT
  template<typename S>
  struct definition
  {
    // shortcut
    typedef BOOST_SPIRIT_CLASSIC_NS :: parser_context<> Ctx;
    template<int Tag> struct tag : public BOOST_SPIRIT_CLASSIC_NS :: parser_tag<Tag> {};
    
    definition(BRGrammar const& self);
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Root> > const& start() const { return root_; }

    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Num> >           num_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Naf> >           naf_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S>                           cons_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Atom> >          atom_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<BridgeAtom> >    bridge_atom_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<BridgeLiteral> > bridge_literal_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Disj> >          disj_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<BridgeBody> >    bridge_body_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<BridgeRule> >    bridge_rule_;
    BOOST_SPIRIT_CLASSIC_NS :: rule<S, Ctx, tag<Root> >          root_;
  };
};

} // namespace dmcs

#endif // BR_GRAMMAR_H

#include "BRGrammar.tcc"

// Local Variables:
// mode: C++
// End:
