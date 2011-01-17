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
 * @file   dmcsd.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Fri Jan  8 17:16:59 2010
 * 
 * @brief  
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif


#include "network/Server.h"
#include "network/Session.h"

#include "parser/LocalKBBuilder.h"
#include "parser/PropositionalASPGrammar.h"
#include "parser/BRGrammar.h"
#include "parser/BridgeRulesBuilder.h"
#include "parser/ParserDirector.h"

#include "loopformula/LocalLoopFormulaBuilder.h"
#include "loopformula/CNFLocalLoopFormulaBuilder.h"
#include "loopformula/LoopFormulaDirector.h"
#include "loopformula/DimacsVisitor.h"

#include "dmcs/Message.h"
#include "dmcs/ProgramOptions.h"
#include "dmcs/Neighbor.h"
#include "dmcs/CommandTypeFactory.h"
#include "dmcs/Log.h"

#include "mcs/Signature.h"

#include "dyndmcs/Match.h"
#include "dyndmcs/NoSBARedBBodySortingStrategy.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <fstream>


using namespace dmcs;

const char* TOP_EXT = ".top";
const char* OPT_EXT = ".opt";



void
read_all_signatures(SignatureVecPtr& global_sigs, std::vector<std::string>& str_sigs)
{
  for (std::vector<std::string>::const_iterator it = str_sigs.begin();
       it != str_sigs.end(); ++it)
    {
      SignaturePtr tmp_sig(new Signature);
      
      std::istringstream in(*it);
      
      in >> *tmp_sig;
      
      global_sigs->push_back(tmp_sig);
    }

  DMCS_LOG_DEBUG("Signatures from match maker:");
  DMCS_LOG_DEBUG(global_sigs);
}


void
read_all_contexts(NeighborListPtr& context_info, std::vector<std::string>& str_contexts)
{
  for (std::vector<std::string>::const_iterator it = str_contexts.begin();
       it != str_contexts.end(); ++it)
    {
      NeighborPtr tmp_context(new Neighbor);

      DMCS_LOG_DEBUG("it = " << *it);
      
      std::istringstream in(*it);
      
      in >> *tmp_context;
      context_info->push_back(tmp_context);
    }

  DMCS_LOG_DEBUG("Contexts in the pool: " << *context_info);
}


void
read_all_matches(MatchTablePtr& mt, const std::string& all_matches,
		 SignatureVecPtr& global_sigs, SignaturePtr& sig)
{
  DMCS_LOG_DEBUG("All matches from match maker:");
  DMCS_LOG_DEBUG(all_matches);

  std::istringstream iss(all_matches);

  typedef boost::escaped_list_separator<char> StringSeparator;
     
  StringSeparator ssep("\\", ",", "()");
  boost::tokenizer<StringSeparator> tok(all_matches, ssep);
	  
  StringSeparator esep("()", " ", "");
	  
  // Store matches in our internal format [(int)src, (int)sym, (int)tar, (int)img, (float)quality]
  
  SignatureBySym& sig_src = boost::get<Tag::Sym>(*sig);
  SignatureBySym::const_iterator src_it;
  
  for (boost::tokenizer<StringSeparator>::iterator it = tok.begin(); it != tok.end(); ++it)
    {
      std::string trimmed = *it;
      
      boost::trim(trimmed);
      boost::tokenizer<StringSeparator> mtok(trimmed, esep);
      
      boost::tokenizer<StringSeparator>::iterator m_it = mtok.begin();
      
      if (m_it == mtok.end())
	{
	  throw boost::escaped_list_error("Got no match");
	}
      
      std::size_t src_ctx = std::atoi(m_it->c_str());
      
      ++m_it;
      if (m_it == mtok.end())
	{
	  throw boost::escaped_list_error("Match length == 1");
	}
      
      src_it = sig_src.find(*m_it);
      if (src_it == sig_src.end())
	{
	  DMCS_LOG_ERROR("Unknown atom: " << m_it->c_str() << " in context " << src_ctx);
	  exit(1);
	}

      std::size_t sym = src_it->origId;
      
      ++m_it;
      if (m_it == mtok.end())
	{
	  throw boost::escaped_list_error("Match length == 2");
	}
      
      std::size_t tar_ctx = std::atoi(m_it->c_str());
      
      ++m_it;
      if (m_it == mtok.end())
	{
	  throw boost::escaped_list_error("Match length == 3");
	}
      
      SignaturePtr target_signature = (*global_sigs)[tar_ctx - 1];
      SignatureBySym& tar_sig = boost::get<Tag::Sym>(*target_signature);
      SignatureBySym::iterator tar_it = tar_sig.find(*m_it);
      
      if (tar_it == tar_sig.end())
	{
	  DMCS_LOG_ERROR("Unknown atom: " << *m_it << " in context " << tar_ctx);
	  exit(1);
	}

      std::size_t img = tar_it->origId;
      
      ++m_it;
      if (m_it == mtok.end())
	{
	  throw boost::escaped_list_error("Match length == 4");
	}
      
      std::istringstream iss(*m_it);
      float qual;
      if (!(iss >> qual))
	{
	  throw boost::escaped_list_error("quality not a float");
	}
      
      mt->insert(Match(src_ctx, sym, tar_ctx, img, qual));
    }

  DMCS_LOG_DEBUG("All matches in our internal format:");
  DMCS_LOG_DEBUG(*mt);
}


int
main(int argc, char* argv[])
{
  try
    {
      int myport = 0;
      std::size_t myid = 0;
      std::size_t pool_size = 0;
      std::size_t system_size = 0;
      std::string filename_local_kb;
      std::string filename_bridge_rules;
      std::string filename_topo;
      std::string filename_match_maker;
      std::string prefix;
      std::size_t limit_answers = 0;
      std::size_t limit_bind_rules = 0;
      std::size_t heuristics = 0;
      bool dynamic = false;
      std::string logging;

      boost::program_options::options_description desc("Allowed options");
      desc.add_options()
	(HELP, "produce help and usage message")
	(CONTEXT_ID, boost::program_options::value<std::size_t>(&myid), "set context ID")
	(PORT, boost::program_options::value<int>(&myport)->default_value(DEFAULT_PORT), "set port")
	(KB, boost::program_options::value<std::string>(&filename_local_kb), "set Knowledge Base file name")
	(BR, boost::program_options::value<std::string>(&filename_bridge_rules), "set Bridge Rules file name")
	(MANAGER, boost::program_options::value<std::string>(), "set Manager HOST:PORT")
	(TOPOLOGY, boost::program_options::value<std::string>(&filename_topo), "set Topology file name")
	(DYNAMIC, boost::program_options::value<bool>(&dynamic)->default_value(false), "set to dynamic mode")
	(MATCH_MAKER, boost::program_options::value<std::string>(&filename_match_maker), "set Match-Maker file name")
	(LIMIT_ANSWERS, boost::program_options::value<std::size_t>(&limit_answers)->default_value(10), "set the limitation of answers to be computed")
	(LIMIT_BIND_RULES, boost::program_options::value<std::size_t>(&limit_bind_rules)->default_value(100), "set the limitation of binding computed for each rule")
	(HEURISTICS, boost::program_options::value<std::size_t>(&heuristics)->default_value(1), "choose heuristics")
	(LOGGING, boost::program_options::value<std::string>(&logging)->default_value(""), "log4cxx config file")
	;
      
      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    
      
      if (vm.count(HELP)) 
	{
	  std::cerr << desc << std::endl;
	  return 1;
	}
      
      if (vm.count(MANAGER)) 
	{
	  std::cerr << "We are sorry, but the manager feature is under implementation, please try the other alternatives" << std::endl;

	  ///@todo read manager host and port here

	  return 1;
	}

      if (myid == 0 || filename_local_kb.empty() || filename_bridge_rules.empty())
	{
	  std::cerr << "The following options are mandatory: --context, --kb, --br." << std::endl;
	  std::cerr << desc << std::endl;
	  return 1;
	}


      // setup log4cxx
      if (logging.empty())
	{
	  init_loggers("dmcsd");
	}
      else
	{
	  init_loggers("dmcsd", logging.c_str());
	}


      if (filename_topo.empty())
	{
	  DMCS_LOG_WARN("No topology given.");
	}

      DMCS_LOG_DEBUG("Context ID: " << myid);
      DMCS_LOG_DEBUG("KB:         " << filename_local_kb);
      DMCS_LOG_DEBUG("BR:         " << filename_bridge_rules);
      DMCS_LOG_DEBUG("Topology:   " << filename_topo);


      boost::asio::io_service io_service;
      boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), myport);
      
      // for dynamic mode 
      MatchTablePtr mt(new MatchTable);
      NeighborListPtr context_info(new NeighborList);
      CountVecPtr sba_count(new CountVec);
      BridgeRulesPtr schematic_bridge_rules(new BridgeRules);
      NeighborListPtr schematic_neighbor_list(new NeighborList);

      // for static mode
      QueryPlanPtr query_plan(new QueryPlan);
      RulesPtr local_kb(new Rules);
      BridgeRulesPtr bridge_rules(new BridgeRules);
      TheoryPtr loopFormula(new Theory);
      NeighborListPtr neighbor_list(new NeighborList);

      // shared between two modes
      SignatureVecPtr global_sigs(new SignatureVec);
      SignaturePtr sig;

      if (dynamic) 
	{ // in dynamic mode
	  if (filename_match_maker.empty())
	    {
	      std::cerr << "No MatchMaker given." << std::endl;
	      std::cerr << desc << std::endl;
	      return 1;
	    }

	  // extract prefix from filename_bridge_rules
	  std::size_t slash_pos = filename_bridge_rules.find_last_of("/");
	  std::string pure_filename = filename_bridge_rules.substr(slash_pos + 1, filename_bridge_rules.length() - slash_pos - 1);

	  std::size_t dot_pos = pure_filename.find_last_of("-");
	  prefix = pure_filename.substr(0, dot_pos);
	  
	  // in dynamic mode
	  DMCS_LOG_DEBUG("In dynamic mode.");
	  DMCS_LOG_DEBUG("MatchMaker: " << filename_match_maker);

	  // open connection to Mr. Match-Maker
	  std::ifstream ifs(filename_match_maker.c_str());
	  if (!ifs)
	    {
	      DMCS_LOG_ERROR("File " << filename_match_maker << " not found.");
	      return 1;
	    }

	  boost::program_options::options_description config("Signature");
	  config.add_options()
	    ("PoolSize", boost::program_options::value< std::size_t >(&pool_size), "number of dynamic contexts");

	  // First run just to get the pool size
	  boost::program_options::store(boost::program_options::parse_config_file(ifs, config, true), vm);
	  boost::program_options::notify(vm);

	  // Empty pool is not allowed!
	  if (pool_size == 0)
	    {
	      DMCS_LOG_ERROR("Empty pool is not allowed.");
	      return 1;
	    }

	  // reopen the config file to read the signatures
	  ifs.close();
	  ifs.open(filename_match_maker.c_str());
	
	  // prepare for all signatures from Mr. Match-Maker
	  std::stringstream out;
	  std::string signature_location;
	  std::string context_info_location;
	  std::vector<std::string> str_sigs(pool_size);
	  std::vector<std::string> str_contexts(pool_size);
	  
	  std::string nosba;
	  std::string all_matches;
	  config.add_options()
	    ("MM.nosba", boost::program_options::value< std::string >(&nosba), "Number of schematic bridge atoms of contexts")
	    ("MM.matches", boost::program_options::value< std::string >(&all_matches), "All matches");
	  
	  // get the global signature from the query plan. Later we might
	  // find another way to pass it to dmcsd, so that we don't have
	  // to depend on the query plan, which is just for the purpose of
	  // opt-dmcs and should be computed from neighbors + global_sigs,
	  // by the manager.
	
	  for (std::size_t i = 1; i <= pool_size; ++i)
	    {
	      out.str("");
	      out << i;
	      signature_location = "Signature.C" + out.str();
	      context_info_location = "ContextsInfo.C" + out.str();
	      
	      config.add_options()
		(signature_location.c_str(), boost::program_options::value< std::string >(&str_sigs[i-1]), "signature of this context")
		(context_info_location.c_str(), boost::program_options::value< std::string >(&str_contexts[i-1]), "identification of this context");
	    }
	  
	  // get the local signature and all matches from Mr. Match-Maker
	  boost::program_options::store(boost::program_options::parse_config_file(ifs, config, true), vm);
	  boost::program_options::notify(vm);

	  // convert strings read to our internal data structure

	  // signatures
	  read_all_signatures(global_sigs, str_sigs);
	  sig = (*global_sigs)[myid-1];
	  
	  // context information == all contexts in the pool 
	  read_all_contexts(context_info, str_contexts);

	  // all matches from the Match Maker, and store them in the Match Table
	  read_all_matches(mt, all_matches, global_sigs, sig);

	  // Now extract information concerning the number of
	  // schematic bridge atoms in each context. Now we use
	  // hardcoding, but later this information must be computed
	  // from bridge rules parsing.

	  boost::tokenizer<> no_sbatoms(nosba);
	  for (boost::tokenizer<>::const_iterator n_it = no_sbatoms.begin();
	       n_it != no_sbatoms.end(); ++n_it)
	    {
	      std::size_t no = std::atoi(n_it->c_str());
	      sba_count->push_back(no);
	    }

	  // Let's not read the local kb now and concentrate on the schematic bridge rules
	  BridgeRulesBuilder<BRGrammar> builder_br(myid, schematic_bridge_rules, schematic_neighbor_list, global_sigs);
	  ParserDirector<BRGrammar> parser_director_br;
	  parser_director_br.setBuilder(&builder_br);
	  parser_director_br.parse(filename_bridge_rules);

	  DMCS_LOG_DEBUG("Schematic bridge rules:");
	  DMCS_LOG_DEBUG(schematic_bridge_rules);

	  // extract non-ordinary schematic bridge atoms, sort the
	  // corresponding iterators according to some quality, and
	  // then give list of sorted iterators to dconf

	  ReducedBridgeBodyVecPtr reduced_bridge_bodies(new ReducedBridgeBodyVec); 
	  for (BridgeRules::const_iterator r = schematic_bridge_rules->begin(); r != schematic_bridge_rules->end(); ++r)
	    {
	      ReducedBridgeBodyPtr reduced_br(new ReducedBridgeBody);

	      // walk through the positve, then the negative body
	      const PositiveBridgeBody& pos_body = getPositiveBody(*r);
	      const NegativeBridgeBody& neg_body = getNegativeBody(*r);

	      for (PositiveBridgeBody::const_iterator it = pos_body.begin(); it != pos_body.end(); ++it)
		{
		  ContextTerm ctt = it->first;

		  if (isCtxVar(ctt))
		    {
		      reduced_br->push_back(*it);
		    }
		}

	      for (NegativeBridgeBody::const_iterator it = neg_body.begin(); it != neg_body.end(); ++it)
		{
		  ContextTerm ctt = it->first;

		  if (isCtxVar(ctt))
		    {
		      reduced_br->push_back(*it);
		    }
		}

	      if (reduced_br->size() > 0)
		{
		  reduced_bridge_bodies->push_back(reduced_br);
		}
	    }

	  ReducedBridgeBodyIteratorListVecPtr reduced_bridge_bodies_iterators_list_vec(new ReducedBridgeBodyIteratorListVec);
	  for (ReducedBridgeBodyVec::const_iterator rb = reduced_bridge_bodies->begin(); rb != reduced_bridge_bodies->end(); ++rb)
	    {
	      ReducedBridgeBodyIteratorListPtr rb_iter(new ReducedBridgeBodyIteratorList);
	      for (ReducedBridgeBody::iterator it = (*rb)->begin(); it != (*rb)->end(); ++it)
		{
		  rb_iter->push_back(it);
		}

	      // now sort each list of iterators pointing to the
	      // non-ordinary schematic bridge atoms

	      NoSBARedBBodySortingStrategy sort_strategy(rb_iter, sba_count, mt, 0);
	      sort_strategy.sort();
	      
#ifdef DEBUG
	      DMCS_LOG_DEBUG("Order in which sbridge atoms will be bound:");
	      for (ReducedBridgeBodyIteratorList::const_iterator it = rb_iter->begin();
		   it != rb_iter->end(); ++it)
		{
		  BridgeAtom sba = **it;
		  ContextTerm ctt = sba.first;
		  SchematicBelief sb = sba.second;

		  DMCS_LOG_DEBUG(ctx2string(ctt) << ", " << sb2string(sb));
		}
#endif // DEBUG

	      // and store this sorted list to use in the configuration
	      reduced_bridge_bodies_iterators_list_vec->push_back(rb_iter);
	    }
	}
      // ***************************************************************************************************************************
      else // ground mode
	{
	  if (filename_local_kb.empty())
	    {
	      std::cerr << "No KB given." << std::endl;
	      std::cerr << desc << std::endl;
	      return 1;
	    }

	  DMCS_LOG_DEBUG("In ground mode.");

	  ///@todo change when the manager is added
	  query_plan->read_graph(filename_topo);
	  system_size = query_plan->getSystemSize();

	  // Empty MSCs are not allowed!
	  if (system_size == 0)
	    {
	      DMCS_LOG_ERROR("MCSen with no contexts are not allowed.");
	      return 1;
	    }
	  
	  // extract the global signature from the query plan
	  for (std::size_t i = 1; i <= system_size; ++i)
	    {
	      const Signature& loc_sig = query_plan->getSignature(i);
	      SignaturePtr loc_sig_ptr(new Signature(loc_sig));
	      global_sigs->push_back(loc_sig_ptr);
	    }

	  DMCS_LOG_DEBUG("Global signatures:");
	  DMCS_LOG_DEBUG(global_sigs);

	  // my local signature
	  sig = (*global_sigs)[myid - 1];
	  
	  // parsing local kb
	  LocalKBBuilder<PropositionalASPGrammar> builder_local_kb(local_kb, sig);
	  ParserDirector<PropositionalASPGrammar> parser_director;
	  parser_director.setBuilder(&builder_local_kb);
	  parser_director.parse(filename_local_kb);

	  // parse the bridge rules
	  BridgeRulesBuilder<BRGrammar> builder_br(myid, bridge_rules, neighbor_list, global_sigs);
	  ParserDirector<BRGrammar> parser_director_br;
	  parser_director_br.setBuilder(&builder_br);
	  parser_director_br.parse(filename_bridge_rules);
	  
	  DMCS_LOG_DEBUG("Finished parsing bridge rules.");
	  
	  for (NeighborList::const_iterator it = neighbor_list->begin(); it != neighbor_list->end(); ++it)
	    {
	      NeighborPtr nb = *it;
	      nb->hostname = query_plan->getHostname(nb->neighbor_id);
	      nb->port     = query_plan->getPort(nb->neighbor_id);
	    }
	  
	  DMCS_LOG_DEBUG("My neighbors: " << *neighbor_list);
	  DMCS_LOG_DEBUG("Neighbor list size: " << neighbor_list->size());
	  DMCS_LOG_DEBUG("Global signatures:");
	  DMCS_LOG_DEBUG(global_sigs);
	  DMCS_LOG_DEBUG("System size: " << system_size);

	  // setup my context
	  ContextPtr ctx(new Context(myid, system_size, sig, local_kb, bridge_rules, neighbor_list));
	  
	  // compute size of local signature
	  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);
	  
	  SignatureByCtx::const_iterator low = local_sig.lower_bound(myid);
	  SignatureByCtx::const_iterator up  = local_sig.upper_bound(myid);
	  	  
	  std::size_t size = std::distance(low, up);
	  
	  DMCS_LOG_DEBUG("Sig input to LF: " << *sig);
	  
	  // construct loop formulas
	  CNFLocalLoopFormulaBuilder lf_builder(sig, size);
	  LoopFormulaDirector director;
	  director.setBuilder(&lf_builder);
	  director.construct(local_kb, bridge_rules);

	  loopFormula = lf_builder.getFormula();
	  
#ifdef DEBUG      
	  DimacsVisitor v(std::cerr);
	  v.visitTheory(loopFormula, sig->size());
#endif // DEBUG
     
	  // this result Sig will only be different in case of using an EquiCNF builder
	  //      SignaturePtr resultSig;
	  //      resultSig = lf_builder.getSignature();
	}

      // Store all information into a CommandTypeFactory, which is
      // responsible for creating the command types later
      CommandTypeFactoryPtr ctf(new CommandTypeFactory(myid, system_size, local_kb, 
						       neighbor_list, schematic_bridge_rules, 
						       bridge_rules, context_info,
						       mt, sba_count, limit_answers, 
						       limit_bind_rules, heuristics, 
						       prefix, global_sigs, sig, query_plan,
						       loopFormula));
      
      // Server can deal with different kinds of messages
      ServerPtr server(new Server(ctf, io_service, endpoint));
      
      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
      io_service.run();
      t.join();
    }
  catch (std::exception& e)
    {
      DMCS_LOG_FATAL("Bailing out: " << e.what());
      return 1;
    }

  return 0;
}

// Local Variables:
// mode: C++
// End:
