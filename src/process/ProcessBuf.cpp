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
 * @file   ProcessBuf.cpp
 * @author Thomas Krennwallner
 * @date   Sun May 21 13:22:23 2006
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "process/ProcessBuf.h"

#include <sstream>
#include <cerrno>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <map>

using namespace dmcs;

namespace
{

inline int safe_open(const char *pathname, int flags)
{
  int countdown = 100;
  int result;
  while( countdown > 0 )
  {
    result = ::open(pathname, flags);
    if( result != -1 )
    {
      return result;
    }
    else
    {
      if( errno != EINTR )
        return result;
    }
    countdown--;
  }
  throw std::runtime_error("open() system call interrupted too often");
}

inline int safe_close(int fd)
{
  int countdown = 100;
  int result;
  while( countdown > 0 )
  {
    result = ::close(fd);
    if( result != -1 )
    {
      return result;
    }
    else
    {
      if( errno != EINTR )
        return result;
    }
    countdown--;
  }
  throw std::runtime_error("close() system call interrupted too often");
}

}

namespace dmcs {

  ///@todo i don't like it, but what can we do, clasp disappears always...
  static std::map<int,int> sigchlds;

  void
  sigchild_handler(int /* sig */, siginfo_t* info, void* /* uap */)
  {
    if (info == 0) return; //??

    switch(info->si_signo)
      {
      case SIGCHLD:
	sigchlds[info->si_pid] = info->si_status;
	break;
      default:
	::perror("sigchild_handler");
	::exit(1);
      };
  }

}


#define NAMEDPIPETMPDIR "/tmp/dmcs-XXXXXX"


ProcessBuf::ProcessBuf()
  : std::streambuf(),
    process(-1),
    status(0),
    bufsize(256)
{
  // ignore SIGPIPE & SIGCHLD (otw. Boost.Test installs a signal
  // handler and fails for non-zero exit codes of child processes)
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  if (::sigaction(SIGPIPE, &sa, 0))
    {
      ::perror("sigaction");
      ::exit(1);
    }

  sa.sa_sigaction = sigchild_handler;
  sa.sa_flags = SA_SIGINFO | SA_RESTART;
  sigemptyset(&sa.sa_mask);

  if (::sigaction(SIGCHLD, &sa, 0))
    {
      ::perror("sigaction");
      ::exit(1);
    }

  initBuffers(); // don't call virtual methods in the ctor
}


ProcessBuf::ProcessBuf(const ProcessBuf& sb)
  : std::streambuf(),
    process(sb.process),
    status(sb.status),
    fifo(sb.fifo),
    bufsize(sb.bufsize),
    fifoname(sb.fifoname)
{
  ::memcpy(inpipes, sb.inpipes, 2);
  initBuffers(); // don't call virtual methods in the ctor
}




ProcessBuf::~ProcessBuf()
{
  close();

  if (::unlink(fifoname.c_str()))
    {
      ::perror("unlink");
      ::exit(1);
    }
      
  if (::rmdir(fifoname.substr(0, strlen(NAMEDPIPETMPDIR)).c_str()))
    {
      ::perror("rmdir");
      ::exit(1);
    }

  if (ibuf)
    {
      delete[] ibuf;
      ibuf = 0;
    }

  if (obuf)
    {
      delete[] obuf;
      obuf = 0;
    }
}


void
ProcessBuf::initBuffers()
{
  obuf = new std::streambuf::char_type[bufsize];
  ibuf = new std::streambuf::char_type[bufsize];
  setp(obuf, obuf + bufsize);
  setg(ibuf, ibuf, ibuf);
}



pid_t
ProcessBuf::open(const std::vector<std::string>& av)
{
  // close before re-open it
  if (process != -1)
    {
      int ret = close();
      if (ret != 0)
	{
	  return ret < 0 ? ret : -ret;
	}
    }

  inpipes[0] = 0;
  inpipes[1] = 0;

  // we want a full-duplex stream -> create one unnamed pipe for
  // writing and one temporary named pipe for reading

  if (!fifoname.empty()) // remove old temp-files
    {
      if (::unlink(fifoname.c_str()))
	{
	  ::perror("unlink");
	  return -1;
	}
      
      if (::rmdir(fifoname.substr(0, strlen(NAMEDPIPETMPDIR)).c_str()))
	{
	  ::perror("rmdir");
	  return -1;
	}
    }


  ///@todo respect $TMPDIR
  char tmpdir[] = NAMEDPIPETMPDIR;
  char* tmp = 0;

  if ((tmp = mkdtemp(tmpdir)) == 0)
    {
      ::perror("mkdtemp");
      return -1;
    }

  fifoname = tmp;
  fifoname += "/fifo";

  if (::mkfifo(fifoname.c_str(), 0600))
    {
      ::perror("mkfifo");
      return -1;
    }

  if (::pipe(inpipes) < 0)
    {
      ::perror("pipe");
      return -1;
    }

  // create a new process 
  process = ::fork();

  switch (process)
    {
    case -1: // error
      ::perror("fork");
      ::exit(process);
      break;

    case 0: // child
      {
	// setup argv

        const size_t argvsize = av.size() + 1;
	char* argv[argvsize];
	int i = 0;

	for (std::vector<std::string>::const_iterator it = av.begin();
	     it != av.end(); it++)
	  {
	    std::string::size_type size = it->size();
	    argv[i] = new char[size + 1];
	    it->copy(argv[i], size);
	    argv[i][size] = '\0';
	    i++;
	  }
	
	argv[i] = '\0';

	// open writing end of fifo

        fifo = safe_open(fifoname.c_str(), O_WRONLY);
	if ( fifo < 0 )
	  {
	    ::perror("safe_open of writing fifo end");
	    return -1;
	  }

	// redirect stdin and stdout and stderr

	if (::dup2(fifo, STDOUT_FILENO) < 0)
	  {
	    ::perror("dup2");
	    ::exit(1);
	  }
	
	if (::dup2(fifo, STDERR_FILENO) < 0)
	  {
	    ::perror("dup2");
	    ::exit(1);
	  }

	if (::dup2(inpipes[0], STDIN_FILENO) < 0)
	  {
	    ::perror("dup2");
	    ::exit(1);
	  }
	
	// stdout and stdin is redirected, close unneeded filedescr.
	safe_close(fifo);
 	safe_close(inpipes[0]);
	safe_close(inpipes[1]);
	
	// execute command, should not return
	::execvp(*argv, argv);
	
	// just in case we couldn't execute the command
	::exit(127);
      }
      break;


    default: // parent
      // open reading end of fifo

      fifo = safe_open(fifoname.c_str(), O_RDONLY);
      if (fifo < 0)
        {
          ::perror("safe_open of reading fifo end");
          return -1;
        }

      // close reading end of the input pipe
      safe_close(inpipes[0]);
      inpipes[0] = -1;
      
      break;
    }

  return process;
}


void
ProcessBuf::endoffile()
{
  // reset output buffer
  setp(obuf, obuf + bufsize);

  if (inpipes[1] != -1)
    {
      int ret = safe_close(inpipes[1]); // send EOF to stdin of child process
      inpipes[1] = -1;

      if (ret < 0) perror("close()");
    }
}

int
ProcessBuf::close()
{
  if (process == -1)
    return 0;

  int retcode = 0;
  pid_t pid = 0;

  // we're done writing
  endoffile();

  // reset input buffer
  setg(ibuf, ibuf, ibuf);

  // we're done reading
  if (fifo != -1)
    {
      int ret = ::close(fifo);
      fifo = -1;

      if (ret < 0) perror("close()");
    }

  // obviously we do not want to leave zombies around, so get status
  // code of the process
  do
    {
      pid = ::waitpid(process, &status, 0);
    }
  while (pid == -1 && errno != ECHILD);

  if (pid == process)
    {
      retcode = WEXITSTATUS(status);
    }
  else if (pid == -1 && errno == ECHILD)
    {
      // get retval from map
      retcode = sigchlds[process]; ///@todo find it and check if it exists
    }
  else
    {
      perror("waitpid()");
      retcode = -1;
    }

  process = -1;


#if 0
  // if (WIFEXITED(status))
  //   {
  //     std::cerr << "normal exit" << std::endl;
  //   }
  // else
  if (WIFSIGNALED(status))
    {
      std::cerr << "signalled: signal " << WTERMSIG(status) << ", coredump " << WCOREDUMP(status) << std::endl;
    }
  else if (WIFSTOPPED(status))
    {
      std::cerr << "process stopped: " << WSTOPSIG(status) << std::endl;
    }
  else
    {
      std::cerr << "huh?" << std::endl;
    }
#endif // 0

  // exit code of process
  return retcode;
}


std::streambuf::int_type
ProcessBuf::overflow(std::streambuf::int_type c)
{
  if (pptr() >= epptr()) // full obuf -> write buffer
    {
      if (sync() == -1)
	{
	  return traits_type::eof();
	}
    }

  // if c != EOF, put c into output buffer so next call to sync() will
  // write it
  if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
      *pptr() = traits_type::to_char_type(c);
      pbump(1); // increase put pointer by one
    }
  
  return traits_type::not_eof(c);
}


std::streambuf::int_type
ProcessBuf::underflow()
{
  if (gptr() >= egptr()) // empty ibuf -> get data
    {
      errno = 0;
      int count = 0;
      ssize_t n = 0;

      // try to receive at most bufsize bytes
      do
	{
	  count++;
	  n = ::read(fifo, ibuf, bufsize);
	}
      while (n < 0 && errno == EINTR && count < 10);

      if (n == 0) // EOF
	{
	  return traits_type::eof();
	}
      else if (count >= 10)
	{
	  std::ostringstream oss;
	  oss << "Interrupted read (errno = " << errno << ").";
	  throw std::ios_base::failure(oss.str());
	}
      else if (n < 0) // a failure occured while receiving from the stream
	{
	  std::ostringstream oss;
	  oss << "Process prematurely closed pipe before I could read (errno = " << errno << ").";
	  throw std::ios_base::failure(oss.str());
	}

      setg(ibuf, ibuf, ibuf + n); // set new input buffer boundaries
    }

  return traits_type::to_int_type(*gptr());
}



std::streambuf::int_type
ProcessBuf::sync()
{
  // reset input buffer
  setg(ibuf, ibuf, ibuf);

  const ssize_t len = pptr() - pbase();

  if (len) // non-empty obuf -> send data
    {
      errno = 0;

      // loops until whole obuf is sent
      //
      // Warning: when peer disconnects during the sending we receive
      // a SIGPIPE and the default signal handler exits the program.
      // Therefore we have to ignore SIGPIPE (in ctor) and reset the
      // obuf followed by an error return value. See chapter 5.13 of
      // W.R. Stevens: Unix Network Programming Vol.1.

      ssize_t ret = 0;

      for (ssize_t written = 0; written < len; written += ret)
	{
	  ret = ::write (inpipes[1], pbase() + written, len - written);
	  if (ret == -1 || ret == 0) break;
	}

      // reset output buffer right after sending to the stream
      setp(obuf, obuf + bufsize);

      if (ret == 0) // EOF
	{
	  return -1;
	}
      else if (ret < 0 || errno == EPIPE) // failure
	{
	  std::ostringstream oss;
	  oss << "Process prematurely closed pipe before I could write (errno = " << errno << ").";
	  throw std::ios_base::failure(oss.str());
	}
    }
  
  return 0;
}

// vim:ts=8:
// Local Variables:
// mode: C++
// End:
