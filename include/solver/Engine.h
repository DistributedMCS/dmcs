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
 * @file   Engine.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Sep  18 12:13:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ENGINE_H
#define ENGINE_H

namespace dmcs {

template <typename KBType, typename InputType, typename OutputType>
class Engine
{
public:
  virtual void
  solve(KBType& kb, InputType& input, OutputType& output);
};

  // From Engine, we can instantiate to specific engines, e.g., DLVEngine where
  // KBType: vector of string
  // InputType: vector of string
  // OutputType: ostream

class DLVEngine : public Engine<std::string, StrVec, std::ostream>
{
  void
  solve(std::string& kb, StrVec& input, std::ostream& os);
}

} // namespace dmcs

#endif // ENGINE_H

// Local Variables:
// mode: C++
// End:
