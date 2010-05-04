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
 * @file   ProxySignatureByLocal.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sat Jan 16 18:38:40 2010
 * 
 * @brief  Signature accessing and storing.
 * 
 * 
 */

#if !defined(_PROXYSIGNATUREBYLOCAL_H)
#define _PROXYSIGNATUREBYLOCAL_H

#include "Signature.h"

namespace dmcs {


///@brief proxy to local and guessing signature
class ProxySignatureByLocal : public SignatureByLocal
{
  std::size_t l; // upper bound of sig
  std::size_t s; // upper bound of gsig
  SignatureByLocal& sig;
  SignatureByLocal& gsig;


  find()

  begin()

  end()
};


} // namespace dmcs

#endif // _PROXYSIGNATUREBYLOCAL_H

// Local Variables:
// mode: C++
// End:
