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

#include "Server.h"
#include "Message.h"
#include "DimacsVisitor.h"
#include "Session.h"
#include "LocalKBBuilder.h"
#include "Match.h"
#include "PropositionalASPGrammar.h"
#include "BRGrammar.h"
#include "BridgeRulesBuilder.h"
#include "ParserDirector.h"
#include "Signature.h"
#include "LocalLoopFormulaBuilder.h"
#include "CNFLocalLoopFormulaBuilder.h"
#include "LoopFormulaDirector.h"
#include "PrimitiveDMCS.h"
#include "OptDMCS.h"
#include "CommandType.h"
#include "ProgramOptions.h"
#include "Neighbor.h"

#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>


using namespace dmcs;

const char* TOP_EXT = ".top";
const char* OPT_EXT = ".opt";

int main(int argc, char* argv[])
{
  try
    {
      int myport = 0;
      std::size_t myid = 0;
      std::size_t pool_size = 0;
      std::string filename_local_kb = "";
      std::string filename_bridge_rules = "";
      std::string filename_topo = "";
      std::string filename_match_maker = "";
      bool dynamic;

      boost::program_options::options_description desc("Allowed options");
      desc.add_options()
	(HELP, "produce help and usage message")
	(CONTEXT_ID, boost::program_options::value<std::size_t>(&myid), "set context ID")
	(PORT, boost::program_options::value<int>(&myport)->default_value(DEFAULT_PORT), "set port")
	(KB, boost::program_options::value<std::string>(&filename_local_kb), "set Knowledge Base file name")
	(BR, boost::program_options::value<std::string>(&filename_bridge_rules), "set Bridge Rules file name")
	(MANAGER, boost::program_options::value<std::string>(), "set Manager HOST:PORT")
	(TOPOLOGY, boost::program_options::value<std::string>(), "set Topology file name")
	(DYNAMIC, boost::program_options::value<bool>(&dynamic)->default_value(false), "set to dynamic mode")
	(MATCH_MAKER, boost::program_options::value<std::string>(&filename_match_maker), "set Match-Maker file name")
	;
      
      boost::program_options::variables_map vm;        
      boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
      boost::program_options::notify(vm);    
      
      if (vm.count(HELP)) 
	{
	  std::cerr << "Usage: " << argv[0] << " --" << CONTEXT_ID << "=ID [--" << PORT << "=PORT] --";
	  std::cerr << KB << "=LOCAL_KB(FILE) --" << BR << "=BRIDGE_RULES(FILE) [--" << MANAGER << "=HOSTNAME:PORT| --";
	  std::cerr << TOPOLOGY << "=TOPOLOGY(FILE)| -- " << DYNAMIC << "=<TRUE/FALSE>| --";
	  std::cerr << MATCH_MAKER << "=MATCH_MAKER(FILE)] "<< std::endl;
	  return 1;
	}
      
      int optionalCount = 0;
      
      if (vm.count(MANAGER)) 
	{
	  optionalCount++;
	  std::cerr << "We are sorry, but the manager feature is under implementation, please try the other alternatives";
	  //read manager host and port
	  return 1;
	}

      if (vm.count(TOPOLOGY)) 
	{
	  filename_topo = vm[TOPOLOGY].as<std::string>();
	  optionalCount++;
	}
      


      ServerPtr server;
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), myport);    
	  
      
      if (dynamic)
	{
	  if (myid == 0 || 
	      filename_bridge_rules.compare("") == 0|| 
	      (filename_match_maker.compare("") == 0))
	    {
	      std::cout << desc << "\n";
	      return 1;
	    }

	  // in dynamic mode
#ifdef DEBUG
	  std::cerr << "In dynamic mode" << std::endl;
#endif

	  // open connection to Mr. Match-Maker
	  std::ifstream ifs(filename_match_maker.c_str());
	  if (!ifs)
	    {
	      std::cerr << "File " << filename_match_maker << " not found!" << std::endl;
	      return 1;
	    }

	  boost::program_options::options_description config("Signature");
	  config.add_options()
	    ("PoolSize", boost::program_options::value< std::size_t >(&pool_size), "number of dynamic contexts");

	  boost::program_options::store(boost::program_options::parse_config_file(ifs, config, true), vm);
	  boost::program_options::notify(vm);

	  if (pool_size == 0)
	    {
	      std::cerr << "Empty pool of context!" << std::endl;
	      return 1;
	    }



 #ifdef DEBUG
	std::cerr << "myid: " << myid <<std::endl;
	std::cerr << "local KB: " << filename_local_kb <<std::endl;
	std::cerr << "Bridge Rules: " << filename_bridge_rules <<std::endl;
	std::cerr << "Topology: " << filename_topo <<std::endl;
 #endif 
	///@todo change when the manager is added
	QueryPlanPtr query_plan(new QueryPlan);

	query_plan->read_graph(filename_topo);

	std::size_t system_size = query_plan->getSystemSize();

	// reopen the config file to read the signatures
	ifs.close();
	ifs.open(filename_match_maker.c_str());
	
	assert(system_size > 0);

	// get all signatures from Mr. Match-Maker
	std::stringstream out;
	std::string signature_location;
	std::vector<std::string> str_sigs(pool_size);

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

	    config.add_options()
	      (signature_location.c_str(), boost::program_options::value< std::string >(&str_sigs[i-1]), "signature of this context");
	  }

	  // get the local signature and all matches from Mr. Match-Maker
	  boost::program_options::store(boost::program_options::parse_config_file(ifs, config, true), vm);
	  boost::program_options::notify(vm);

	  SignaturesPtr global_sigs(new Signatures);
	  for (std::vector<std::string>::const_iterator it = str_sigs.begin();
	       it != str_sigs.end(); ++it)
	    {    
	      SignaturePtr tmp_sig(new Signature);
	      std::istringstream in(*it);

	      in >> *tmp_sig;
	      global_sigs->push_back(tmp_sig);
	    }

	  SignaturePtr sig = (*global_sigs)[myid-1];

#ifdef DEBUG
	  std::cerr << "Signatures from match maker:" << std::endl << global_sigs << std::endl;
	  /*for (Signatures::const_iterator s_it = global_sigs->begin();
	       s_it != global_sigs->end(); ++s_it)
	    {
	      std::cerr << *s_it << std::endl;
	      }*/

	  std::cerr << "All matches from match maker:" << std::endl << all_matches << std::endl;
#endif

	  typedef boost::escaped_list_separator<char> StringSeparator;
	  
	  StringSeparator ssep("\\", ",", "()");
	  boost::tokenizer<StringSeparator> tok(all_matches, ssep);

	  StringSeparator esep("()", " ", "");

	  // Store matches in our internal format [(int)src, (int)sym, (int)tar, (int)img, (float)quality]
	  MatchMakerPtr mm(new MatchMaker);

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
		  std::cerr << "Uknown atom: " << m_it->c_str() << " in context " << src_ctx << std::endl;
		  return 1;
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
		  std::cerr << "Uknown atom: " << *m_it << " in context " << tar_ctx << std::endl;
		  return 1;
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

	      mm->insert(Match(src_ctx, sym, tar_ctx, img, qual));
	    }

#ifdef DEBUG
	  std::cerr << "All matches in our internal format:" << std::endl << *mm << std::endl;
#endif

	  // Let's not read the local kb now and concentrate on the schematic bridge rules
	  BridgeRulesPtr bridge_rules(new BridgeRules);
	  BridgeRulesBuilder<BRGrammar> builder_br(myid, bridge_rules, sig, global_sigs);
	  ParserDirector<BRGrammar> parser_director_br;
	  parser_director_br.setBuilder(&builder_br);
	  parser_director_br.parse(filename_bridge_rules);

#ifdef DEBUG
	  std::cerr << "Bridge rules = " << bridge_rules << std::endl;
#endif

	  DynamicConfigurationPtr dconf(new DynamicConfiguration(bridge_rules, mm));
	  DynamicCommandType dcmd(dconf);

	  ServerPtr s(new Server<DynamicCommandType>(dcmd, io_service, endpoint));
	  server = s;

	  // watch out! moving this code out will cause dconf to be destroyed,
	  // then we will lose bridge_rules and mm
	  boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	  io_service.run();
	  t.join();
	}
      // ***************************************************************************************************************************
      else // ground mode
	{
	  // in ground mode

	  if (myid == 0 || 
	      filename_local_kb.compare("") == 0 || 
	      filename_bridge_rules.compare("") == 0|| 
	      optionalCount == 0 || 
	      optionalCount == 2) 
	    {
	      std::cout << desc << "\n";
	      return 1;
	    }


#ifdef DEBUG
	  std::cerr << "In ground mode" << std::endl;
#endif
	  ///@todo change when the manager is added
	  QueryPlanPtr query_plan(new QueryPlan);

	  query_plan->read_graph(filename_topo);
	  
	  SignaturePtr sig(new Signature);
	  *sig = query_plan->getSignature(myid);
	  std::size_t system_size = query_plan->getSystemSize();
	  
#ifdef DEBUG
	  std::cerr << "Server information              " << std::endl;
	  std::cerr << "Id:                             " << myid << std::endl;
	  std::cerr << "Port:                           " << myport << std::endl;
	  std::cerr << "Filename for local KB:          " << filename_local_kb << std::endl;
	  std::cerr << "Filename for bridge rules:      " << filename_bridge_rules << std::endl;
	  std::cerr << "Filename for topology:          " << filename_topo << std::endl;
	  std::cerr << "My signature:                   " << *sig << std::endl;
#endif
	  
	  //TheoryPtr theory(new Theory);
	  RulesPtr local_kb(new Rules);
	  BridgeRulesPtr bridge_rules(new BridgeRules);
	  //BridgeAtomSetPtr br(new BridgeAtomSet);
	  
	  LocalKBBuilder<PropositionalASPGrammar> builder1(local_kb, sig);
	  ParserDirector<PropositionalASPGrammar> parser_director;
	  parser_director.setBuilder(&builder1);
	  parser_director.parse(filename_local_kb);
	  
#ifdef DEBUG
	  // for (Signature::const_iterator i = sig->begin(); i != sig->end(); ++i)
	  // 	{
	  // 	  std::cerr << *i << std::endl;
	  // 	}
#endif

	  // extract the global signature from the query plan
	  SignaturesPtr global_sigs(new Signatures);

	  for (std::size_t i = 1; i <= system_size; ++i)
	    {
	      const Signature& loc_sig = query_plan->getSignature(i);
	      SignaturePtr loc_sig_ptr(new Signature(loc_sig));
	      global_sigs->push_back(loc_sig_ptr);
	    }
#ifdef DEBUG
	  std::cerr << "Global signatures: " << std::endl << global_sigs << std::endl;
#endif	  

	  // now we got the local KB,
	  // going to parse the bridge rules

	  BridgeRulesBuilder<BRGrammar> builder_br(myid, bridge_rules, sig, global_sigs);
	  ParserDirector<BRGrammar> parser_director_br;
	  parser_director_br.setBuilder(&builder_br);
	  parser_director_br.parse(filename_bridge_rules);
	  
	  
	  // setup my context
	  ContextPtr ctx(new Context(myid, system_size, sig, query_plan, local_kb, bridge_rules));

	  
	  //compute size local signature
	  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);
	  
	  SignatureByCtx::const_iterator low = local_sig.lower_bound(myid);
	  SignatureByCtx::const_iterator up  = local_sig.upper_bound(myid);
      
#ifdef DEBUG
	  std::cerr << myid << std::endl;
#endif
	  
	  std::size_t size = std::distance(low, up);

#ifdef DEBUG
	  std::cerr << "Sig input to LF" << *sig <<std::endl;
#endif
	  
	  //construct loop formulae
	  CNFLocalLoopFormulaBuilder lf_builder(sig, size);
	  LoopFormulaDirector director;
	  director.setBuilder(&lf_builder);
	  director.construct(local_kb, bridge_rules);
	  
	  TheoryPtr loopFormula;
	  loopFormula = lf_builder.getFormula();
	  
#ifdef DEBUG      
	  DimacsVisitor v(std::cerr);
	  v.visitTheory(loopFormula, sig->size());
#endif
     
	  // this result Sig will only be different in case of using an EquiCNF builder
	  //      SignaturePtr resultSig;
	  //      resultSig = lf_builder.getSignature();
	  
	  boost::shared_ptr<BaseServer> server;

	  DMCSPtr dmcs;
	  boost::shared_ptr<BaseServer> server;
	  
	  // setup the server
	  if (filename_topo.find(TOP_EXT) != std::string::npos)
	    {
	      PrimitiveDMCSPtr d(new PrimitiveDMCS(ctx, loopFormula, global_sigs));
	      PrimitiveCommandType pdmcs(d);
	      
	      boost::shared_ptr<BaseServer> s(new Server<PrimitiveCommandType>(pdmcs, io_service, endpoint));      
	      server = s;
	    }
	  else if (filename_topo.find(OPT_EXT) != std::string::npos)
	    {
	      OptDMCSPtr d(new OptDMCS(ctx, loopFormula, global_sigs, query_plan));
	      OptCommandType odmcs(d);
	      
	      boost::shared_ptr<BaseServer> s(new Server<OptCommandType>(odmcs, io_service, endpoint));  
	      server = s;
	      
	      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	      io_service.run();
	      t.join();
	    }
	}
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << std::endl;
    }

  return 0;
}

// Local Variables:
// mode: C++
// End:
