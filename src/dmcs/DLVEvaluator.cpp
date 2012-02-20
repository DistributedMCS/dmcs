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
 * @file   DLVEvaluator.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  19 15:10:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/BeliefStateResultAdder.h"
#include "dmcs/DLVEvaluator.h"
#include "parser/DLVResultParser.h"
#include "dmcs/Instantiator.h"

namespace dmcs {

DLVEvaluator::Options::Options()
  : Evaluator::GenericOptions(),
    arguments()
{
  arguments.push_back("-silent");
}




DLVEvaluator::Options::~Options()
{ }


DLVEvaluator::DLVEvaluator(const InstantiatorWPtr& inst)
  : Evaluator(inst)
{ }



void
DLVEvaluator::solve(std::size_t ctx_id, 
		    Heads* heads,
		    BeliefTablePtr btab,
		    NewConcurrentMessageDispatcherPtr md)
{
  // setupProcess
  proc.setPath(DLVPATH);
  if (options.includeFacts)
    {
      proc.addOption("-facts");
    }
  else
    {
      proc.addOption("-nofacts");
    }

  for (std::vector<std::string>::const_iterator it = options.arguments.begin();
       it != options.arguments.end(); ++it)
    {
      proc.addOption(*it);
    }

  // request stdin as last parameter
  proc.addOption("--");
  
  // fork dlv process
  proc.spawn();

  std::ostream& programStream = proc.getOutput();

  // copy stream
  InstantiatorPtr instantiator_p = instantiator.lock();
  Instantiator* instantiator_star = instantiator_p.get();

  assert (instantiator_star);
  std::string local_kb = instantiator_p->getKB();
  programStream << local_kb;

  // putting heads into programStream
  NewBeliefState* head_input = heads->getHeads();
  std::size_t pos = head_input->getFirst();
  do
    {
      IDKind kind = ID::MAINKIND_BELIEF | ctx_id;
      IDAddress address = pos;

      ID id(kind, address);
      const Belief& belief = btab->getByID(id);

      programStream << belief.text << ".\n";
      pos = head_input->getNext(pos);
    }
  while (pos);

  programStream.flush();

  proc.endoffile();

  BeliefStateResultAdder adder(out_queue, md, heads);
  DLVResultParser dlv_parser(ctx_id, btab);

  std::istream& is = proc.getInput();
  do
    {
      std::string input;
      std::getline(is, input);

      if ( input.empty() || is.bad() )
      {
	/*std::cerr << "leaving loop because got input size " << input.size() 
		  << ", stream bits fail " << is.fail() << ", bad " << is.bad() 
		  << ", eof " << is.eof() << std::endl;*/
	break;
      }

      // discard weak answer set cost lines
      if( 0 == input.compare(0, 22, "Cost ([Weight:Level]):") )
      {
	//std::cerr << "discarding weak answer set cost line" << std::endl;
      }
      else
      {
	// parse line
	std::istringstream iss(input);
	dlv_parser.parse(iss, adder);
      }      
    }
  while (1);

  // nomore answer wrt this heads. Send a NULL to EVAL_OUT
  int timeout = 0;

  HeadsBeliefStatePair* null_ans = new HeadsBeliefStatePair();
  null_ans->first = heads;
  null_ans->second = NULL;
  md->send(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, out_queue, null_ans, timeout);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
