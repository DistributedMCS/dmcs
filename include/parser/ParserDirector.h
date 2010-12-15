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
 * @file   ParserDirector.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:59:18 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef PARSER_DIRECTOR_H
#define PARSER_DIRECTOR_H

#include "parser/BaseBuilder.h"

#include <iosfwd>
#include <fstream>
#include <string>
#include <sstream>


namespace dmcs {

template<class Grammar>
class ParserDirector
{
public:
  ParserDirector();

  void
  setBuilder(BaseBuilder<Grammar>* b)
  {
    // just use the new builder
    builder = b;
  }

  void 
  parse(const std::string& file);

  void
  parse(std::istream& is);

private:
  BaseBuilder<Grammar>* builder;
};

} // namespace dmcs

#endif // _PARSER_DIRECTOR_H_

#include "ParserDirector.tcc"

// Local Variables:
// mode: C++
// End:
