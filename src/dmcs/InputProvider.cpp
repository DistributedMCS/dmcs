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
 * @file InputProvider.hpp
 * @author Peter Schueller
 * @date 
 *
 * @brief Input stream provider (collects input sources)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/InputProvider.h"
#include "dmcs/URLBuf.h"

#include <cassert>
#include <fstream>
#include <sstream>

namespace dmcs {

#warning TODO use boost::iostream::chain or something similar to create real streaming/incremental input provider

class InputProvider::Impl
{
public:
  std::stringstream stream;
  std::vector<std::string> contentNames;

public:
  Impl()
  {
  }
};

InputProvider::InputProvider():
  pimpl(new Impl)
{
}

InputProvider::~InputProvider()
{
}

void InputProvider::addStreamInput(std::istream& i, const std::string& contentname)
{
  pimpl->stream << i.rdbuf();
  pimpl->contentNames.push_back(contentname);
}

void InputProvider::addStringInput(const std::string& content, const std::string& contentname)
{
  pimpl->stream << content;
  pimpl->contentNames.push_back(contentname);
}

void InputProvider::addFileInput(const std::string& filename)
{
  std::ifstream ifs;
  ifs.open(filename.c_str());
  
  std::string err = "File " + filename + " not found";
  assert (ifs.is_open() && err.c_str());

  pimpl->stream << ifs.rdbuf();
  ifs.close();
  pimpl->contentNames.push_back(filename);
}

void InputProvider::addURLInput(const std::string& url)
{
  assert(url.find("http://") == 0 && "currently only processing http:// URLs");

  URLBuf ubuf;
  ubuf.open(url);
  std::istream is(&ubuf);

  pimpl->stream << is.rdbuf();

  std::string err = "Requested URL " + url + " was not found";
  assert (ubuf.responsecode() != 404 && err.c_str());

  pimpl->contentNames.push_back(url);
}

bool InputProvider::hasContent() const
{
  return !pimpl->contentNames.empty();
}

const std::vector<std::string>& InputProvider::contentNames() const
{
  return pimpl->contentNames;
}

std::istream& InputProvider::getAsStream()
{
  assert(hasContent() && "should have gotten some content before using content");
  return pimpl->stream;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:

