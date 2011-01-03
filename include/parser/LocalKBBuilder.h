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
 * @file   LocalKBBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  30 13:10:24 2009
 * 
 * @brief  
 * 
 * 
 */


#ifndef LOCAL_KB_BUILDER_H
#define LOCAL_KB_BUILDER_H

#include "parser/BaseBuilder.h"
#include "mcs/Rule.h"
#include "mcs/Theory.h"
#include "mcs/Signature.h"

namespace dmcs {

template<class Grammar>
class LocalKBBuilder : public BaseBuilder<Grammar>
{
public:
  LocalKBBuilder(RulesPtr& local_kb_, SignaturePtr& sig_);

  void
  buildNode (typename BaseBuilder<Grammar>::node_t& node);

private:
  void
  build_disjunctive_head(typename BaseBuilder<Grammar>::node_t& node, RulePtr& r);

  void
  build_body(typename BaseBuilder<Grammar>::node_t& node, RulePtr& r);

  RulesPtr& local_kb;
  SignaturePtr& sig;
};

} // namespace dmcs

#endif // LOCAL_KB_BUILDER_H

#include "LocalKBBuilder.tcc"

// Local Variables:
// mode: C++
// End:
