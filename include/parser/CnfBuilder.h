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
 * @file   CnfBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:46:30 2009
 * @deprecated not used anymore... 
 *
 * @brief  
 * 
 * 
 */


#ifndef CNF_BUILDER_H
#define CNF_BUILDER_H

#include "BaseBuilder.h"
#include "Theory.h"

namespace dmcs {

///@todo TK: are these guys still needed?
// typedef std::map<Variable, std::string> MapId2Name;
// typedef std::map<Variable, Variable> MapId2Lid;
// typedef boost::bimap<Variable, std::string> BiMapId2Name;
// typedef boost::bimap<Variable, Variable> BiMapId2Lid;
// typedef std::vector<BiMapId2Name> BiMapsId2Name;
// typedef std::vector<BiMapId2Lid> BiMapsId2Lid;


template<typename Grammar>
class CnfBuilder : public BaseBuilder<Grammar>
{
public:
  CnfBuilder(TheoryPtr, BiMapId2Name&);

  void
  buildNode (typename BaseBuilder<Grammar>::node_t& node);

  BiMapId2Lid
  getLocalSATId() const;

private:
  void 
  build_clause(typename BaseBuilder<Grammar>::node_t& node);
  
  void 
  build_atom(typename BaseBuilder<Grammar>::node_t& node, ClausePtr clause);
  
  void 
  add_atom(int sign, const std::string& atom_name, ClausePtr clause);
  
  TheoryPtr theory;
  BiMapId2Name& id2name;
};

} // namespace dmcs

#endif // _CNF_BUILDER_H

#include "CnfBuilder.tcc"

// Local Variables:
// mode: C++
// End:

