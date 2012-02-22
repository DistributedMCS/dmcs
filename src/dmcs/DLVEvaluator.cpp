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
  : Evaluator(inst),
    proc(NULL)
{ }


DLVEvaluator::~DLVEvaluator()
{ }



void
DLVEvaluator::reset_process(std::size_t ctx_id,
			    Heads* heads,
			    BeliefTablePtr btab)
{
  models_counter = 0;

  assert (proc == NULL);
  proc = new DLVProcess;

  // setupProcess
  proc->setPath(DLVPATH);
  if (options.includeFacts)
    {
      proc->addOption("-facts");
    }
  else
    {
      proc->addOption("-nofacts");
    }

  for (std::vector<std::string>::const_iterator it = options.arguments.begin();
       it != options.arguments.end(); ++it)
    {
      proc->addOption(*it);
    }

  // request stdin as last parameter
  proc->addOption("--");

  // fork dlv process
  proc->spawn();

  std::ostream& programStream = proc->getOutput();

  // copy stream
  InstantiatorPtr instantiator_p = instantiator.lock();
  Instantiator* instantiator_star = instantiator_p.get();

  assert (instantiator_star);
  std::string local_kb = instantiator_p->getKB();
  programStream << local_kb;

  // putting heads into programStream
  // only for intermediate context, i.e., heads != NULL
  if (heads)
    {
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
    }

  programStream.flush();

  proc->endoffile();
}


void
DLVEvaluator::read_all(std::size_t ctx_id,
		       Heads* heads,
		       BeliefTablePtr btab,
		       NewConcurrentMessageDispatcherPtr md)
{
  BeliefStateResultAdder adder(out_queue, md, heads);
  DLVResultParser dlv_parser(ctx_id, btab);

  std::istream& is = proc->getInput();
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
}

void
DLVEvaluator::read_until_k2(std::size_t ctx_id, 
			    std::size_t k1,
			    std::size_t k2,
			    Heads* heads,
			    BeliefTablePtr btab,
			    NewConcurrentMessageDispatcherPtr md)
{
  assert (0 < k1 && k1 < k2);

  BeliefStateResultAdder adder(out_queue, md, heads);
  DLVResultParser dlv_parser(ctx_id, btab);

  std::istream& is = proc->getInput();

  // ignore  the first k1 models
  while (models_counter < k1-1)
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
      if ( 0 == input.compare(0, 22, "Cost ([Weight:Level]):") )
      {
	//std::cerr << "discarding weak answer set cost line" << std::endl;
      }
      else
      {
	++models_counter;
      }      
    }

  if (models_counter < k1-1)
    {
      return;
    }

  while (models_counter < k2)
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
	++models_counter;
	std::istringstream iss(input);
	dlv_parser.parse(iss, adder);
      }      
    }
}





void
DLVEvaluator::solve(std::size_t ctx_id, 
		    std::size_t k1,
		    std::size_t k2,
		    Heads* heads,
		    BeliefTablePtr btab,
		    NewConcurrentMessageDispatcherPtr md)
{
  reset_process(ctx_id, heads, btab);

  if (k1 == 0 && k2 == 0)
    {
      read_all(ctx_id, heads, btab, md);
    }
  else
    {
      assert (k1 <= k2);
      read_until_k2(ctx_id, k1, k2, heads, btab, md);
    }

  std::cerr << "Going to delete proc." << std::endl;
  delete proc;
  proc = 0;
  std::cerr << "Deleting done" << std::endl;

  // Either no more answer wrt this heads,
  // or k2 answers reached.
  // Send a NULL to EVAL_OUT
  int timeout = 0;
  HeadsBeliefStatePair* null_ans = new HeadsBeliefStatePair();
  null_ans->first = heads;
  null_ans->second = NULL;
  md->send(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, out_queue, null_ans, timeout);
}


void
DLVEvaluator::solve(std::size_t ctx_id, 
		    Heads* heads,
		    BeliefTablePtr btab,
		    NewConcurrentMessageDispatcherPtr md)
{
  reset_process(ctx_id, heads, btab);


  // Either no more answer wrt this heads,
  // or k2 answers reached.
  // Send a NULL to EVAL_OUT
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
