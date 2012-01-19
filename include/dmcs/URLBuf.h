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
 * @file   URLBuf.h
 * @author Thomas Krennwallner
 * @date   Fri Jan 25 10:00:58 GMT 2008
 * 
 * @brief  iostreams interface for libcurl.
 * 
 * 
 */


#ifndef URLBUF_H
#define URLBUF_H

#include <iostream>
#include <streambuf>
#include <string>

#include <curl/curl.h>

namespace dmcs {

/**
 * @brief A std::streambuf for reading data from URLs.
 */
class URLBuf : public std::streambuf
{
public:
  URLBuf();

  URLBuf(const URLBuf&);

  virtual
  ~URLBuf();

  /**
   * @param url open this URL string
   */
  virtual void
  open(const std::string& url);

  /**
   * @return #response
   */
  virtual long
  responsecode() const;

private:
  /// input buffer
  std::streambuf::char_type* ibuf;

  /// size of #ibuf
  unsigned bufsize;

  /// a CURL handle for accessing URLs
  CURL* easy_handle;

  /// response code from HTTP/FTP/...
  long response;

  static size_t
  writer(void *ptr, size_t size, size_t nmemb, void *stream);

  size_t
  write(void* ptr, size_t size);

protected:
  virtual std::streambuf::int_type
  underflow();
};

} // namespace dmcs

#endif // URLBUF_H

// Local Variables:
// mode: C++
// End:
