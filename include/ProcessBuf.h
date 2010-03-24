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
 * @file   ProcessBuf.h
 * @author Thomas Krennwallner
 * @date   Sun May 21 13:22:36 2006
 * 
 * @brief  iostreams interface to external programs.
 * 
 * 
 */


#if !defined(_PROCESSBUF_H)
#define _PROCESSBUF_H

#include <iostream>
#include <streambuf>
#include <vector>
#include <string>


namespace dmcs {

/**
 * @brief A std::streambuf interface for creating child processes and
 * writing/reading data via stdin/stdout.
 */
class ProcessBuf : public std::streambuf
{
public:
  ProcessBuf();

  ProcessBuf(const ProcessBuf&);

  virtual
  ~ProcessBuf();

  virtual void
  endoffile();

  virtual pid_t
  open(const std::vector<std::string>&);

  virtual int
  close();

private:
  pid_t process;

  int status;

  int outpipes[2];
  int inpipes[2];

  unsigned bufsize;


  std::streambuf::char_type* obuf;
  std::streambuf::char_type* ibuf;

  void
  initBuffers();

protected:
  virtual std::streambuf::int_type
  overflow(std::streambuf::int_type c);

  virtual std::streambuf::int_type
  underflow();

  virtual std::streambuf::int_type
  sync();
};

} // namespace dmcs

#endif // _PROCESSBUF_H


// Local Variables:
// mode: C++
// End:
