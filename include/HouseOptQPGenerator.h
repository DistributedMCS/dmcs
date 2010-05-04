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
 * @file   HouseOptQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sat Jan  21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef HOUSE_OPT_QP_GENERATOR_H_
#define HOUSE_OPT_QP_GENERATOR_H_

#include "Context.h"
#include "HouseQPGenerator.h"


namespace dmcs { namespace generator {

    class HouseOptQPGenerator : public HouseQPGenerator
    {
    public:
      HouseOptQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
	: HouseQPGenerator(contexts_, query_plan_)
      { }

      void
      create_interface(const ContextPtr& context)
      { 

	std::size_t id = context->getContextID();

	if (id == 1)
	  {
#ifdef DEBUG
	      std::cerr << "root case "<<std::endl;
#endif
	    BeliefStatePtr bs_12 = query_plan->getInterface1(id, id+1);	  
	    const BeliefStatePtr bs_23 = query_plan->getInterface1(id+1, id+2);

	    update(bs_12, bs_23);
	    query_plan->putInterface(id, id+1, bs_12);
#ifdef DEBUG
	      std::cerr << "done with root case"<<std::endl;
#endif
	    return;
	  }

	int remainder = id % 4;

	switch (remainder)
	  {
	  case 2:
	    {
	  
#ifdef DEBUG
	      std::cerr << "In case 2"<<std::endl;
#endif
	      std::size_t houseId = (id+2)/4;
	      std::size_t parentId = 1;
	      if(houseId % 2 == 0)
		{
		  parentId = 2*(((id-2)/4)+1);
		}
	      else
		{
		  if(id != 2)
		    {
		      parentId = 2*(((id-5)/4)+1) +1;
		    }
		}
	      BeliefStatePtr bs_23 = query_plan->getInterface1(id, id+1);
	      const BeliefStatePtr bs_p3 = query_plan->getInterface1(parentId, id+1);
	      const BeliefStatePtr bs_30 = query_plan->getInterface1(id+1, id+2);

	      update(bs_23, bs_p3);
	      update(bs_23, bs_30);
	      query_plan->putInterface(id, id+1, bs_23);
	      query_plan->remove_connection(parentId, id+1);
#ifdef DEBUG
	      std::cerr << "doen with case 2"<<std::endl;
#endif
	    }
	    break;
	  case 3:
	    {
#ifdef DEBUG
	      std::cerr << "In case 3"<<std::endl;
#endif
	      BeliefStatePtr bs_30 = query_plan->getInterface1(id, id+1);
	      std::cerr << "1 " << *bs_30 << std::endl;
	      const BeliefStatePtr bs_01 = query_plan->getInterface1(id+1, id+2);
	      std::cerr << "2" << *bs_01 << std::endl;	  
	      update(bs_30, bs_01);
	      std::cerr << "3" <<std::endl;
	      query_plan->putInterface(id, id+1, bs_30);
#ifdef DEBUG
	      std::cerr << "done with case 3"<<std::endl;
#endif
	    }
	    break;
	  case 0:
	    {
#ifdef DEBUG
	      std::cerr << "In case 0"<<std::endl;
#endif
	      BeliefStatePtr bs_01 = query_plan->getInterface1(id, id+1);	  
	      const BeliefStatePtr bs_12 = query_plan->getInterface1(id+1, id-2);
	      update(bs_01, bs_12);
	      query_plan->putInterface(id, id+1, bs_01);
	      query_plan->remove_connection(id+1, id-2);
	  
	      std::size_t childId = id + ((id/4)-1)*4 + 2;
	      if(childId < context->getSystemSize())
		{
		  BeliefStatePtr bs_p2 = query_plan->getInterface1(id, childId);
	      
		  const BeliefStatePtr bs_23 = query_plan->getInterface1(childId, childId+1);
	      
		  update(bs_p2, bs_23);
		  query_plan->putInterface(id, childId, bs_p2);
		}
#ifdef DEBUG
	      std::cerr << "done with case 0"<<std::endl;
#endif
	    }
	    break;
	  case 1:
	    {
#ifdef DEBUG
	      std::cerr << "In case 1"<<std::endl;
#endif
	      std::size_t childId = id + ((id/4)-1)*4 + 5;
	      if(childId < context->getSystemSize())
		{
		  BeliefStatePtr bs_p2 = query_plan->getInterface1(id, childId);
	      
		  const BeliefStatePtr bs_23 = query_plan->getInterface1(childId, childId+1);
	      
		  update(bs_p2, bs_23);
		  query_plan->putInterface(id, childId, bs_p2);
		}
#ifdef DEBUG
	      std::cerr << "done with case 1"<<std::endl;
#endif
	    }
	    break;
	  }
      }
    };


  } // namespace generator
} // namespace dmcs


#endif // HOUSE_OPT_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
