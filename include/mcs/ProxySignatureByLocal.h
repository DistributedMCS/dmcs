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
class ProxySignatureByLocal // : public SignatureByLocal
{

public:
  ProxySignatureByLocal(const SignatureByLocal& sig_, const SignatureByLocal& gsig_)
    : sig(sig_),
      gsig(gsig_)
  { }

  SignatureByLocal::const_iterator
  find(std::size_t local_id) const
  {
    if (local_id <= sig.size())
      {
	return sig.find(local_id);
      }

    if (local_id <= sig.size() + gsig.size())
      {
	return gsig.find(local_id);
      }
    
    return gsig.end();
  }

  std::size_t
  size() const
  {
    return sig.size() + gsig.size();
  }

  SignatureByLocal::const_iterator
  begin() const
  {
    return sig.begin();
  }

  SignatureByLocal::const_iterator
  end() const
  {
    return gsig.end();
  }

private:
  const SignatureByLocal& sig;
  const SignatureByLocal& gsig;
};

typedef boost::shared_ptr<ProxySignatureByLocal> ProxySignatureByLocalPtr;

} // namespace dmcs

#endif // _PROXYSIGNATUREBYLOCAL_H

// Local Variables:
// mode: C++
// End:
