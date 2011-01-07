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
 * @file   DynamicConfiguration.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Apr 27  14:08:59 2010
 * 
 * @brief  
 * 
 * 
 */

#include "dyndmcs/NoSBANeighborSortingStrategy.h"
#include "dyndmcs/NoSBACtxSortingStrategy.h"
#include "dyndmcs/NoSBAQualityCtxSortingStrategy.h"
#include "dyndmcs/ChosenCtxSortingStrategy.h"
#include "dyndmcs/ChosenQualityCtxSortingStrategy.h"
#include "dyndmcs/QualityCtxSortingStrategy.h"
#include "dyndmcs/DynamicConfiguration.h"
#include "dyndmcs/DynamicCommandType.h"
#include "dyndmcs/Match.h"

#include "network/Client.h"

#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>


namespace dmcs 
{

DynamicConfiguration::DynamicConfiguration(std::size_t ctx_id_,
					   BridgeRulesPtr bridge_rules_, 
					   const NeighborListPtr context_info_, 
					   const MatchTablePtr mt_,
					   const CountVecPtr sba_count_,
					   std::size_t limit_answers_,
					   std::size_t limit_bind_rules_,
					   std::size_t heuristics_,
					   const std::string& prefix_)
  : ctx_id(ctx_id_),
    bridge_rules(bridge_rules_),
    context_info(context_info_),
    mt(mt_),
    sba_count(sba_count_),
    limit_answers(limit_answers_),
    limit_bind_rules(limit_bind_rules_),
    heuristics(heuristics_),
    no_answers(0),
    stop(false),
    prefix(prefix_),
    signal(3)
{ }


ContextSubstitutionListPtr
DynamicConfiguration::lconfig(ConfigMessage& mess, std::size_t dfs_level)
{
#ifdef DEBUG
  std::cerr << "DynamicConfiguration::lconfig" << std::endl;
  std::cerr << "Mess = " << mess << std::endl;
#endif
  std::size_t root_ctx = mess.getRootContext();
  const ContextSubstitutionPtr ctx_sub = mess.getCtxSubstitution();

  ContextSubstitutionListPtr result = lconfig(root_ctx, bridge_rules->begin(), bridge_rules->end(), ctx_sub, dfs_level);

  if (ctx_id == root_ctx)
    {

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "Number of results from lconfig = " << result->size() << std::endl;
      std::cerr << TABS(dfs_level) << "Going to compute result's topological graphs" << std::endl;
#endif
      
      // whenever get an answer at root context, write out the
      // topology in graphviz format
      compute_topology(result, dfs_level);
    }

  return result;
}


ContextSubstitutionListPtr
DynamicConfiguration::lconfig(const std::size_t root, 
			      BridgeRules::const_iterator r_beg, 
			      BridgeRules::const_iterator r_end, 
			      ContextSubstitutionPtr ctx_substitution_sofar,
			      std::size_t dfs_level)
{
  // DFS controlled by rules iterator r_beg and r_end
  if (r_beg == r_end)
    {

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "REACHED EMPTY SET OF RULES." << std::endl;
      std::cerr << TABS(dfs_level) << "ctx_substitution_sofar = " << *ctx_substitution_sofar << std::endl;
      std::cerr << TABS(dfs_level) << "Calculating new contexts" << std::endl;
#endif

      ContextVec new_contexts;

      for (ContextSubstitution::const_iterator it = ctx_substitution_sofar->begin();
	   it != ctx_substitution_sofar->end(); ++it)
	{
	  ContextTerm ctx_variable = it->ctx_term;
	  std::size_t ctx_value    = it->tarCtx;

	  if ( (ctx_value != root) && (ctxID(ctx_variable) == ctx_id))
	    {
	      // Only add each new context once
	      ContextVec::const_iterator cit = std::find(new_contexts.begin(), new_contexts.end(), ctx_value);

	      if (cit == new_contexts.end())
		{
		  // Check whether this new context candidate was in a
		  // map by a different context (Cx) than the current
		  // context (Ck).  If it is the case then we don't
		  // need to invoke this guy, because it was/will be
		  // eventually invoked by (Cx)
		  if (!already_chosen(ctx_substitution_sofar, ctx_id, ctx_value))
		    {
		      new_contexts.push_back(ctx_value);
		    }
		}
	    }
	}

      if (new_contexts.size() > 0)
	{
#ifdef DEBUG
	  std::cerr << TABS(dfs_level) << "Going to call " << new_contexts.size() << " neighbors: ";
	  std::copy(new_contexts.begin(), new_contexts.end(), std::ostream_iterator<std::size_t>(std::cerr, " "));
	  std::cerr << std::endl;
#endif

	  // HEURISTIC: sort contexts according to some quality to invoke.
	  ContextVecIteratorListPtr new_contexts_iter(new ContextVecIteratorList);

	  for (ContextVec::iterator it = new_contexts.begin(); it != new_contexts.end(); ++it)
	    {
	      new_contexts_iter->push_back(it);
	    }

	  switch (heuristics)
	    {
	    case NO_SBA_CTX_SORTING_STRATEGY:
	      {
		NoSBANeighborSortingStrategy sort_new_contexts_strategy(new_contexts_iter, sba_count, dfs_level);
		sort_new_contexts_strategy.sort();
	      }
	      break;
	    }
	  

#ifdef DEBUG
	  std::cerr << TABS(dfs_level) << "Order of invoking neighbors:" << std::endl;
	  std::cerr << TABS(dfs_level);
	  for (ContextVecIteratorList::const_iterator it = new_contexts_iter->begin(); it != new_contexts_iter->end(); ++it)
	    {
	      std::cerr << **it << " ";
	    }
	  std::cerr << std::endl;
#endif

	  return invoke_neighbors(root, new_contexts_iter->begin(), new_contexts_iter->end(), ctx_substitution_sofar, dfs_level+1);
	}

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "No new contexts to call, return {context_substitution_sofar}" << std::endl;
#endif

      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
      ctx_subs->push_back(ctx_substitution_sofar);
      
      return ctx_subs;
    }

  // this queue stores intermediate substitutions for the rule r_beg
  ContextSubstitutionListPtr output_queue_r;
  
  PositiveBridgeBody::const_iterator pb_beg = getPositiveBody(*r_beg).begin();
  PositiveBridgeBody::const_iterator pb_end = getPositiveBody(*r_beg).end();
  NegativeBridgeBody::const_iterator nb_beg = getNegativeBody(*r_beg).begin();
  NegativeBridgeBody::const_iterator nb_end = getNegativeBody(*r_beg).end();
  
  // call a DFS procedure to find all substitution for this rule. 
  // the recursion is controled by 2 pairs of body iterators, for the positive and
  // negative bridge bodies, respectively
#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Start binding rule r = " << *r_beg  << std::endl;
#endif

  stop_bind_rules = false;
  no_bind_rules = 0;
  output_queue_r = bind_rule(pb_beg, pb_end, nb_beg, nb_end, ctx_substitution_sofar, dfs_level+1);

  // Don't need to sort in output_queue_r because we already applied
  // some quality in ordering the contexts to be bound

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Finished binding rule r = " << *r_beg  << std::endl;
#endif

  // now we got all substitutions for r_beg, go to the next rule.
  BridgeRules::const_iterator r_next = r_beg;
  r_next++;

  ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);

  // find substitutions for the next rule for each substitution found for r_beg.
  // (backtracking technique)
  while((output_queue_r->size() > 0) && (!stop))
    {
      ContextSubstitutionList::iterator cs_it = output_queue_r->begin();
      
      //std::cerr << "call lconfig for the next rule with substitution = " << **cs_it << std::endl;
      ContextSubstitutionListPtr intermediate_ctx_subs = lconfig(root, r_next, r_end, *cs_it, dfs_level+1);

      ctx_subs->insert(ctx_subs->end(), intermediate_ctx_subs->begin(), intermediate_ctx_subs->end());

      // for comparing the behavior of different quality criteria, we
      // pick the first "limit_answers" number of substitutions and
      // see how the topology looks like
      if (stop)
	{
	  return ctx_subs;
	}
      
      output_queue_r->pop_front();
    }

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Finished lconfig for local rules, number of substitutions = " << ctx_subs->size() << std::endl;
#endif

  return ctx_subs;
}


ContextSubstitutionListPtr
DynamicConfiguration::invoke_neighbors(const std::size_t root,
				       ContextVecIteratorList::const_iterator c_beg,
				       ContextVecIteratorList::const_iterator c_end,
				       ContextSubstitutionPtr ctx_substitution_sofar,
				       std::size_t dfs_level)
{

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "DynamicConfiguration::invoke_neighbors." << std::endl;
  std::cerr << TABS(dfs_level) << "ctx_substitution_sofar = " << *ctx_substitution_sofar << std::endl;
#endif

  if (c_beg == c_end)
    {

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "No more neighbor to invoke, return {ctx_substitution_sofar}" << std::endl;
#endif
      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
      ctx_subs->push_back(ctx_substitution_sofar);

      no_answers++;

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "Number of answers = " << no_answers << std::endl;
#endif

      if ((limit_answers > 0) && (no_answers >= limit_answers))
	{
	  // we want to stop here
#ifdef DEBUG
	  std::cerr << "Number of answers reached limitation. We want to STOP" << std::endl;
#endif
	  stop = true;
	}

      return ctx_subs;
    }

  // call context neighbor identified by **c_beg
  ContextID cid = **c_beg;
#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Invoking neighbor " << cid << std::endl;
#endif

  ContextVecIteratorList::const_iterator c_next = c_beg;
  c_next++;

  // find detail information of the neighbor: hostname, port
  NeighborList::const_iterator nb = std::find_if(context_info->begin(), context_info->end(), compareNeighbors(cid));

  assert(nb != context_info->end());

  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver(io_service);

  boost::asio::ip::tcp::resolver::query query((*nb)->hostname, (*nb)->port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *it;

  std::string header = HEADER_REQ_DYN_DMCS;
  ConfigMessage mess(root, ctx_substitution_sofar, false);

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Message = " << mess << std::endl;
#endif

  Client<DynamicCommandType> client(io_service, it, header, mess);
  io_service.run();

  dynmcs_return_type output_queue_cj = client.getResult();

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Got " << output_queue_cj->size() << " substitutions from neighbor " << cid << std::endl;
  std::cerr << TABS(dfs_level) << *output_queue_cj << std::endl;
  std::cerr << TABS(dfs_level) << "Now going to call next neighbors." << std::endl;
#endif

  ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);

  // invoke the next neighbor with substitution updated from previous
  // neighbors.
  while ((output_queue_cj->size()) > 0 && (!stop))
    {
      ContextSubstitutionList::iterator cs_it = output_queue_cj->begin();

#ifdef DEBUG
      if (c_next != c_end)
	{
	  std::cerr << TABS(dfs_level) << "Invoke neighbor " << **c_next << std::endl;
	}
      else
	{
	  std::cerr << TABS(dfs_level) << "Reached ending." << std::endl;
	}
#endif

      ContextSubstitutionListPtr intermediate_ctx_subs = invoke_neighbors(root, c_next, c_end, *cs_it, dfs_level+1);

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "intermediate_ctx_subs.size() = " << intermediate_ctx_subs->size() << std::endl;
      std::cerr << TABS(dfs_level) << "ctx_subs.size() = " << ctx_subs->size() << std::endl;
#endif

      ctx_subs->insert(ctx_subs->end(), intermediate_ctx_subs->begin(), intermediate_ctx_subs->end());

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "ctx_subs.size() = " << ctx_subs->size() << std::endl;
#endif

      output_queue_cj->pop_front();
    }

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "Exit invoking neighbor " << cid << std::endl;
  std::cerr << TABS(dfs_level) << "ctx_subs.size() = " << ctx_subs->size() << std::endl;
#endif

  return ctx_subs;
}


ContextSubstitutionListPtr
DynamicConfiguration::bind_rule(PositiveBridgeBody::const_iterator pb_beg, 
				PositiveBridgeBody::const_iterator pb_end,
				NegativeBridgeBody::const_iterator nb_beg,
				NegativeBridgeBody::const_iterator nb_end,
				ContextSubstitutionPtr ctx_substitution_sofar,
				std::size_t dfs_level)
{
  // DFS controlled by 2 pairs of body iterators, for the positive
  // and negative bridge bodies, respectively

#ifdef DEBUG
  std::cerr << TABS(dfs_level) << "DynamicConfiguration::bind_rule" << std::endl 
	    << "ctx_substitution_sofar = " << *ctx_substitution_sofar << std::endl;
#endif

  // all bridge atoms matched, return the substitution at this leaf situation.
  if ((pb_beg == pb_end) && (nb_beg == nb_end))
    {

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "No more atom to bind. return {ctx_substitution_sofar}" << std::endl;
#endif

      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
      ctx_subs->push_back(ctx_substitution_sofar);

      no_bind_rules++;

      // We only want a limit of binding to our set of rules
      if (no_bind_rules >= limit_bind_rules)
	{
#ifdef DEBUG
	  std::cerr << "We want to stop binding rules here..." << std::endl;
#endif
	  stop_bind_rules = true;
	}

      return ctx_subs;
    }
  else // pick next s-bridge atom
    {
      BridgeAtom sba;
      PositiveBridgeBody::const_iterator pb_next;
      NegativeBridgeBody::const_iterator nb_next;

      if (pb_beg != pb_end)
	{
	  sba = *pb_beg;
	  pb_next = pb_beg;
	  nb_next = nb_beg;
	  ++pb_next;
	}
      else
	{
	  sba = *nb_beg;
	  pb_next = pb_beg;
	  nb_next = nb_beg;
	  ++nb_next;
	}

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "Chose " << sba << std::endl;
#endif

      // check for s-bridge atom
      ContextTerm ctt = sba.first;
      SchematicBelief sb = sba.second;

      std::size_t sb_type = sBeliefType(sb);

      if ((!isCtxVar(ctt) && (sb_type == IS_ORDINARY_BELIEF))) // ordinary s-bridge atom, then just go ahead
	{
#ifdef DEBUG
	  std::cerr << TABS(dfs_level) << "An ordinary s-bridge atom. Continues..." << std::endl;
#endif
	  return bind_rule(pb_next, pb_end, nb_next, nb_end, ctx_substitution_sofar, dfs_level+1);
	}
      else if (!isCtxVar(ctt)) // context term is a context id
	{
	  // since we are constructing the context substitution
	  // \sigma, we can move on here. If one wants to contruct the
	  // bridge substitution \theta then what needs to be done
	  // here includes:
	  // + read the term substitution \eta from mm
	  // + compute \theta = \sigma \otimes \eta

#ifdef DEBUG
	  std::cerr << TABS(dfs_level) << "Context term is a context id. Find a target atom..." << std::endl;
#endif
	  ///@todo: find a target atom with srcCtx, srcSym, and tarCtx available.
	  // ctt now holds the constant value for the target context,
	  // and source context we can only 
	  std::size_t target_ctx = ctt;
	  std::size_t context_id = ctx_id;
	  std::size_t s_const = sBelief(sb);

	  const MatchTableBySrcSymTar& sst = boost::get<Tag::SrcSymTar>(*mt);
		      
	  MatchTableBySrcSymTar::iterator low = sst.lower_bound(boost::make_tuple(context_id, s_const, target_ctx));
	  MatchTableBySrcSymTar::iterator up  = sst.upper_bound(boost::make_tuple(context_id, s_const, target_ctx));
		      
	  if (std::distance(low, up) == 0)
	    {
			  
#ifdef DEBUG
	      std::cerr << TABS(dfs_level) << "Ups, no match from the match maker. " << std::endl;
	      std::cerr << TABS(dfs_level) << "Simply backtrack by returning empty set of substitutions now." << std::endl;
#endif	    
			  
	      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
	      
	      return ctx_subs;
	    }
	  else
	    {
	      // simply take "low" at the moment. Later we
	      // will create a function for the choice
	      // part and call it from here and also for
	      // uninstantiated atoms.
	      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
	      
	      std::size_t src_sym = low->sym;
	      ContextID   tar_ctx = low->tarCtx;
	      std::size_t img_sym = low->img;
	      float       quality = low->quality;
	      
	      ContextSubstitutionPtr next_ctx_substitution(new ContextSubstitution(*ctx_substitution_sofar));
	      next_ctx_substitution->insert(ContextMatch(ctt, src_sym,tar_ctx, img_sym, quality));
			  
	      ContextSubstitutionListPtr intermediate_ctx_subs = bind_rule(pb_next, pb_end, nb_next, nb_end, next_ctx_substitution, dfs_level+1);
	      
	      ctx_subs->insert(ctx_subs->end(), intermediate_ctx_subs->begin(), intermediate_ctx_subs->end());
	      
	      return ctx_subs;
	    }

	  //return bind_rule(pb_next, pb_end, nb_next, nb_end, ctx_substitution_sofar, dfs_level+1);
	}
      else // context term is a context variable
	{

	  // consult the match maker for potential matches.
	  const MatchTableBySrcSym& ss = boost::get<Tag::SrcSym>(*mt);
 	      
	  std::size_t context_id = ctxID(ctt);
	  std::size_t s_const = sBelief(sb);

	  MatchTableBySrcSym::iterator low = ss.lower_bound(boost::make_tuple(context_id, s_const));
	  MatchTableBySrcSym::iterator up  = ss.upper_bound(boost::make_tuple(context_id, s_const));
	  
	  // empty match requires cutting off, but for the moment
	  // let's just backtrack
	  if (std::distance(low, up) == 0)
	    {

#ifdef DEBUG
	      std::cerr << TABS(dfs_level) << "Ups, no match from the match maker. " << std::endl;
	      std::cerr << TABS(dfs_level) << "Simply backtrack by returning empty set of substitutions now." << std::endl;
#endif	    

	      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
	      
	      return ctx_subs;
	    }

	  // collect iterators in the match table that has
	  // potential_neighbor as tarCtx

	  MatchTableIteratorVec mti;

	  for (; low != up; ++low)
	    {
	      ContextID potential_neighbor = low->tarCtx;

#ifdef DEBUG
	      std::cerr << "potential_neighbor = " << potential_neighbor << std::endl;
#endif

	      MatchTableIteratorVec::iterator it = std::find_if(mti.begin(), mti.end(), CompareMatch(potential_neighbor));

	      if (it == mti.end())
		{
		  mti.push_back(low);
		}
	    }
	  
	  // check whether the context variable was already instantiated	  
	  ContextSubstitution::const_iterator it = ctx_substitution_sofar->find(ctt);

	  if (it == ctx_substitution_sofar->end()) // context term has not yet been instantiated
	    {

	      // create a list of iterator of iterators that we
	      // collected before, so that we can sort potential
	      // neighbors by different heuristice criteria

	      MatchTableIteratorVecIteratorListPtr mti_iter(new MatchTableIteratorVecIteratorList);
	      for (MatchTableIteratorVec::iterator it = mti.begin(); it != mti.end(); ++it)
		{
		  mti_iter->push_back(it);
		}

	      switch (heuristics)
		{
		case NO_SBA_CTX_SORTING_STRATEGY:
		  {
		    NoSBACtxSortingStrategy sort_strategy(mti_iter,  sba_count, dfs_level);
		    sort_strategy.sort();
		  }
		  break;
		case NO_SBA_QUALITY_CTX_SORTING_STRATEGY:
		  {
		    NoSBAQualityCtxSortingStrategy sort_strategy(mti_iter,  sba_count, dfs_level);
		    sort_strategy.sort();
		  }
		  break;
		case CHOSEN_CTX_SORTING_STRATEGY:
		  {
		    ChosenCtxSortingStrategy sort_strategy(mti_iter,  ctx_substitution_sofar, context_info->size(), dfs_level);
		    sort_strategy.sort();
		  }
		  break;
		case CHOSEN_QUALITY_CTX_SORTING_STRATEGY:
		  {
		    ChosenQualityCtxSortingStrategy sort_strategy(mti_iter,  ctx_substitution_sofar, context_info->size(), dfs_level);
		    sort_strategy.sort();
		  }
		  break;
		case QUALITY_CTX_SORTING_STRATEGY:
		  {
		    QualityCtxSortingStrategy sort_strategy(mti_iter, dfs_level);
		    sort_strategy.sort();
		  }
		  break;
		}
	      
#ifdef DEBUG
	  std::cerr << TABS(dfs_level) << "Sorted possible substitutions according to some quality:" << std::endl;
	  std::cerr << TABS(dfs_level);
	  for (MatchTableIteratorVecIteratorList::const_iterator it = mti_iter->begin(); it != mti_iter->end(); ++it)
	    {
	      std::cerr << (**it)->tarCtx << " ";
	    }
	  std::cerr << std::endl;
#endif

	      ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);

	      // try all possibilities for instantiating this context variable
	      MatchTableIteratorVecIteratorList::const_iterator mti_it = mti_iter->begin();

	      while ((mti_it != mti_iter->end()) && (!stop_bind_rules))
		{

#ifdef DEBUG
		  std::cerr << TABS(dfs_level) << "Now bind " << ctx2string(ctt) << " to " << (**mti_it)->tarCtx << std::endl;
		  std::cerr << TABS(dfs_level) << "Going to bind next atom." << std::endl;
#endif
		  
		  std::size_t src_sym = (**mti_it)->sym;
		  ContextID   tar_ctx = (**mti_it)->tarCtx;
		  std::size_t img_sym = (**mti_it)->img;
		  float       quality = (**mti_it)->quality;


		  ContextSubstitutionPtr next_ctx_substitution(new ContextSubstitution(*ctx_substitution_sofar));
		  next_ctx_substitution->insert(ContextMatch(ctt, src_sym,tar_ctx, img_sym, quality));
		  
		  ContextSubstitutionListPtr intermediate_ctx_subs = bind_rule(pb_next, pb_end, nb_next, nb_end, next_ctx_substitution, dfs_level+1);
		  
		  ctx_subs->insert(ctx_subs->end(), intermediate_ctx_subs->begin(), intermediate_ctx_subs->end());

#ifdef DEBUG
		  std::cerr << TABS(dfs_level) << "There are " << intermediate_ctx_subs->size() << " substitutions from binding next atom: " << std::endl;
		  std::cerr << TABS(dfs_level) << *intermediate_ctx_subs << std::endl;
		  std::cerr << TABS(dfs_level) << "ctx_subs.size() = " << ctx_subs->size() << std::endl;
#endif		  

		  ++mti_it;
		}
	      
#ifdef DEBUG
	      std::cerr << TABS(dfs_level) << "There are " << ctx_subs->size() << " substitutions after trying all possibilities to bind : " << std::endl;
	      std::cerr << TABS(dfs_level) << *ctx_subs << std::endl;

#endif
 	      
	      return ctx_subs;
	    }
	  
	  else // context term was already instantiated
	    {
	      
#ifdef DEBUG
	      std::cerr << TABS(dfs_level) << "This context term was already instantiated." << std::endl;
#endif
	      
	      // if the instantiated value does not appear in the
	      // possible neighbor, then that substitution is
	      // inconsistent with the possibilities we have here for
	      // instantiation, hence we have to drop by returning
	      // an empty set

	      MatchTableIteratorVec::const_iterator mti_jt = std::find_if(mti.begin(), mti.end(), CompareMatch(it->tarCtx));

	      if (mti_jt == mti.end())
		{
		  
#ifdef DEBUG
		  std::cerr << TABS(dfs_level) << "Previous instantiation doesn't match the current potential neighbors." << std::endl;
		  std::cerr << TABS(dfs_level) << "Will return empty set of substitutions." << std::endl;
#endif		  

		  ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);

		  return ctx_subs;
		}
	      else
		{
		  // It can be that this context was bound to some
		  // context but the atom was not. Then we just need
		  // to find the target atom, if the schematic bridge
		  // atom hasn't been bound so far. This is a very
		  // rare case (when users write 2 similar atoms in
		  // the same rule); however, we still need to take
		  // care of this.

		  const ContextSubstitutionBySrcSym& css = boost::get<Tag::SrcSym>(*ctx_substitution_sofar);
		  ContextSubstitutionBySrcSym::const_iterator css_it = css.find(boost::make_tuple(context_id, s_const));

		  if (css_it == css.end())
		    {
		      const MatchTableBySrcSymTar& sst = boost::get<Tag::SrcSymTar>(*mt);
		      
		      std::size_t context_id = ctxID(ctt);
		      std::size_t s_const = sBelief(sb);
		      std::size_t target_ctx = it->tarCtx;
		      
		      MatchTableBySrcSymTar::iterator low = sst.lower_bound(boost::make_tuple(context_id, s_const, target_ctx));
		      MatchTableBySrcSymTar::iterator up  = sst.upper_bound(boost::make_tuple(context_id, s_const, target_ctx));
		      
		      if (std::distance(low, up) == 0)
			{
			  
#ifdef DEBUG
			  std::cerr << TABS(dfs_level) << "Ups, no match from the match maker. " << std::endl;
			  std::cerr << TABS(dfs_level) << "Simply backtrack by returning empty set of substitutions now." << std::endl;
#endif	    
			  
			  ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);
			  
			  return ctx_subs;
			}
		      else
			{
			  // simply take "low" at the moment. Later we
			  // will create a function for the choice
			  // part and call it from here and also for
			  // uninstantiated atoms.
			  ContextSubstitutionListPtr ctx_subs(new ContextSubstitutionList);

			  std::size_t src_sym = low->sym;
			  ContextID   tar_ctx = low->tarCtx;
			  std::size_t img_sym = low->img;
			  float       quality = low->quality;
			  
			  ContextSubstitutionPtr next_ctx_substitution(new ContextSubstitution(*ctx_substitution_sofar));
			  next_ctx_substitution->insert(ContextMatch(ctt, src_sym,tar_ctx, img_sym, quality));
			  
			  ContextSubstitutionListPtr intermediate_ctx_subs = bind_rule(pb_next, pb_end, nb_next, nb_end, next_ctx_substitution, dfs_level+1);
			  
			  ctx_subs->insert(ctx_subs->end(), intermediate_ctx_subs->begin(), intermediate_ctx_subs->end());
			  
			  return ctx_subs;
			}

		    }

#ifdef DEBUG
		  std::cerr << TABS(dfs_level) << "Go to the next atom." << std::endl;
#endif

		  return bind_rule(pb_next, pb_end, nb_next, nb_end, ctx_substitution_sofar, dfs_level+1);
		}
	    }
	}
    }
}


GraphListPtr
DynamicConfiguration::compute_topology(ContextSubstitutionListPtr ctx_subs, std::size_t /* dfs_level */)
{
  // Heuristic index
  std::stringstream out;
  out << heuristics;

  std::string filename_stats = prefix;
  filename_stats = "result/" + filename_stats + out.str() + ".sta";

  std::ofstream file_stats;
  file_stats.open(filename_stats.c_str());

  std::size_t ps = context_info->size();

  file_stats << "Poolsize        = " << ps << std::endl;
  file_stats << "Initial density = " << no_unique_connections(mt, ps) << std::endl;

  GraphListPtr topo(new GraphList);

  std::size_t i = 1;
  for (ContextSubstitutionList::const_iterator it = ctx_subs->begin();
       it != ctx_subs->end(); ++it, ++i)
    {

#ifdef DEBUG
      std::cerr << TABS(dfs_level) << "Computing topological graph number " << i << std::endl;
#endif

      GraphPtr g = compute_topology(*it);
      topo->push_back(g);

      std::ofstream file_topo;
      std::string filename = prefix;


      // Heuristic index
      out.str("");
      out << heuristics;
      filename = "result/" + filename + "-result-" + out.str() + "-";

      // Result index
      out.str("");
      out << i;
      filename = filename + out.str() + ".dot";

      std::cerr << "filename = " << filename << std::endl;

      file_topo.open(filename.c_str());
      boost::write_graphviz(file_topo, *g);
      file_topo.close();

      file_stats << "Resulted system size = " << resulted_system_size(g) 
		 << ". Resulted system density = " << boost::num_edges(*g) 
		 << ". Average quality = " << average_quality(*it) << std::endl;
    }

  file_stats.close();

  return topo;
}


GraphPtr
DynamicConfiguration::compute_topology(ContextSubstitutionPtr ctx_sub)
{
  GraphPtr g(new Graph(context_info->size()));

  for (ContextSubstitution::const_iterator it = ctx_sub->begin();
       it != ctx_sub->end(); ++it)
    {
      ContextTerm ctx_term = it->ctx_term;

      ContextID to = it->tarCtx;
      ContextID from = ctxID(ctx_term);

      TopoVertex u = boost::vertex(from-1, *g);
      TopoVertex v = boost::vertex(to-1, *g);

      TopoOutEdgeIterator out_i;
      TopoOutEdgeIterator out_end;

      bool had_edge = false;
      boost::tie(out_i, out_end) = out_edges(u, *g);
      for (; out_i != out_end; ++out_i)
	{
	  TopoVertex t = boost::target(*out_i, *g);
	  if (t == v)
	    {
	      had_edge = true;
	    }
	}

      if (!had_edge)
	{
	  boost::add_edge(u, v, *g);
	}
    }
  
  return g;
}


// In the instantiated system, we only count the root context and
// contexts which are used in the substitution, i.e., in the topology
// graph, corresponds to a node which has in_degree + out_degree > 0
std::size_t
DynamicConfiguration::resulted_system_size(GraphPtr g)
{
  std::size_t rss = 0;
  
  TopoVertexIter it;
  TopoVertexIter end;

  TopoInEdgeIterator in_edges_beg;
  TopoInEdgeIterator in_edges_end;

  TopoOutEdgeIterator out_edges_beg;
  TopoOutEdgeIterator out_edges_end;

  for (boost::tie(it, end) = boost::vertices(*g); it != end; ++it)
    {
      std::size_t i = boost::in_degree(*it, *g);

      if (i > 0)
	{
	  ++rss;
	}
      else
	{
	  std::size_t o = boost::out_degree(*it, *g);
	  if (o > 0)
	    {
	      ++rss;
	    }
	}
    }

  return rss;
}

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
