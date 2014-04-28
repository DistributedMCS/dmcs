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
 * @file   InputProvider.h
 * @author Peter Schueller
 * @date
 *
 * @brief Input stream provider (collects input sources)
 */

#ifndef INPUT_PROVIDER_H
#define INPUT_PROVIDER_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>
#include <string>
#include <iosfwd>

namespace dmcs {

class InputProvider
{
public:
  InputProvider();
  ~InputProvider();

  void addStreamInput(std::istream& i, const std::string& contentname);
  void addStringInput(const std::string& content, const std::string& contentname);
  void addFileInput(const std::string& filename);
  void addURLInput(const std::string& url);
  
  bool hasContent() const;
  const std::vector<std::string>& contentNames() const;

  std::istream& getAsStream();

private:
  class Impl;
  boost::scoped_ptr<Impl> pimpl;
};

typedef boost::shared_ptr<InputProvider> InputProviderPtr;

} // namespace dmcs

#endif // INPUT_PROVIDER_H
