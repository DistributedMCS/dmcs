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
 * @file   ProgramOptions.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Thu March  25 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef PROGRAM_OPTIONS_H_
#define PROGRAM_OPTIONS_H_

namespace dmcs {

#define HELP               "help"
#define PORT               "port"
#define DEFAULT_PORT       5001
#define DEFAULT_QUEUE_SIZE 50
#define DEFAULT_PACK_SIZE  5
#define INVOKER_ID         1023
#define GENDATA            "gen-data"
#define QUERY_VARS         "query-variables"
#define SYSTEM_SIZE        "system-size"
#define QUEUE_SIZE         "queue-size"
#define BS_SIZE            "belief-state-size"
#define MANAGER            "manager"
#define HOSTNAME           "hostname"
#define CONTEXT_ID         "context"
#define KB                 "kb"
#define BR                 "br"
#define QP                 "queryplan"
#define OPTQP              "optqueryplan"
#define RP                 "returnplan"
#define TOPOLOGY           "topology"
#define CONTEXTS           "contexts"
#define SIGNATURE          "signature"
#define ATOMS              "atoms"
#define INTERFACE          "interface"
#define BRIDGE_RULES       "bridge_rules"
#define TEMPLATE           "template"
#define PREFIX             "prefix"
#define DMCSPATH           "dmcspath"
#define LOGGING            "logging"
#define DAEMONIZE          "daemon"
#define DYNAMIC            "dynamic"
#define MATCH_MAKER        "mm"
#define ROOT_CTX           "root"
#define LIMIT_ANSWERS      "n"
#define LIMIT_BIND_RULES   "b"
#define HEURISTICS         "h"
#define STREAMING          "streaming"
#define PACK_SIZE          "packsize"
#define MQ_SIZE            "mq_size"
#define CONFLICTS_DRIVEN   "cd"
#define MAX_RESOURCES      "reso"
#define K1                 "k1"
#define K2                 "k2"
#define STARTUP_TIME       "startup-time"
#define TIMEOUT            "timeout"
#define LOOP               "loop"
#define LOCAL_KB_TYPE      "local-kb-type"

} // namespace dmcs

#endif // PROGRAM_OPTIONS_H_

// Local Variables:
// mode: C++
// End:
