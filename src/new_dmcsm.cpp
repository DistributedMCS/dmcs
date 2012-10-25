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
 * @file   new_dmcsm.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Oct  25 18:52:30 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/functional/hash.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "dmcs/ProgramOptions.h"
#include "mcs/Logger.h"

using namespace dmcs;

int 
main(int argc, char* argv[])
{
  try
    {
      std::string hostname;
      std::string port;
      std::size_t system_size;

      const char* help_description = "\nUsage: dmcsm --hostname=HOSTNAME --port=PORT --system-size=N";

      boost::program_options::options_description desc(help_description);

      desc.add_options()
	(HELP, "produce help and usage message")
	(PORT, boost::program_options::value<std::string>(&port), "set port")
	(SYSTEM_SIZE, boost::program_options::value<std::size_t>(&system_size), "set system size");

      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    

      if (vm.count(HELP))
	{
	  std::cerr << desc << std::endl;
	  exit(1);
        }

      if (hostname.empty() || port.empty() || system_size == 0)
	{
	  std::cerr << "The following options are mandatory: --port, --system-size " << std::endl;
	  std::cerr << desc << std::endl;
	  return 1;
	}
    }
  catch (std::exception& e)
    {
      DBGLOG(ERROR, "Bailing out: " << e.what());
      return 1;
    }
  catch (...)
    {
      DBGLOG(ERROR, "Unknown exception!");
      return 1;
    }
  
  return 0;
}


// Local Variables:
// mode: C++
// End:
