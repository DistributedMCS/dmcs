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
  ProxySignatureByLocal(const SignaturePtr& sig_, const SignaturePtr& gsig_)
    : sig(sig_),
      gsig(gsig_),
      sig_by_local(boost::get<Tag::Local>(*sig_)),
      gsig_by_local(boost::get<Tag::Local>(*gsig_))
  { }

  /// @todo kept for historic reasons now
  ProxySignatureByLocal(const SignatureByLocal& sig_, const SignatureByLocal& gsig_)
    : sig_by_local(sig_),
      gsig_by_local(gsig_)
  { }

  SignatureByLocal::const_iterator
  find(std::size_t local_id) const
  {
    if (local_id <= sig_by_local.size())
      {
	return sig_by_local.find(local_id);
      }

    if (local_id <= sig_by_local.size() + gsig_by_local.size())
      {
	return gsig_by_local.find(local_id);
      }
    
    return gsig_by_local.end();
  }

  std::size_t
  size() const
  {
    return sig_by_local.size() + gsig_by_local.size();
  }

  SignatureByLocal::const_iterator
  begin() const
  {
    return sig_by_local.begin();
  }

  SignatureByLocal::const_iterator
  end() const
  {
    return gsig_by_local.end();
  }

private:
  const SignaturePtr sig;
  const SignaturePtr gsig;
  const SignatureByLocal& sig_by_local;
  const SignatureByLocal& gsig_by_local;
};

typedef boost::shared_ptr<ProxySignatureByLocal> ProxySignatureByLocalPtr;

} // namespace dmcs

#endif // _PROXYSIGNATUREBYLOCAL_H

// Local Variables:
// mode: C++
// End:
