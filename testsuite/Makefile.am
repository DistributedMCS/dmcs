# global preprocessor flags
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir) \
	$(BM_CPPFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(LOG4CXX_CFLAGS) \
	@LIBCURL_CPPFLAGS@

# global linker flags
AM_LDFLAGS = \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_THREAD_LDFLAGS) \
	$(BOOST_SERIALIZATION_LDFLAGS) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)

# global libs
LIBS = \
	$(BOOST_THREAD_LIBS) \
	$(BOOST_GRAPH_LIBS) \
	$(BOOST_SERIALIZATION_LIBS) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(LOG4CXX_LIBS)


TESTS = \
	testClasp \
	testBeliefComparison \
	testParsers \
	testTranslation \
	testSignature \
	testCombination \
	testClaspResultOnlineParser \
	testBlockingClasp \
	testGraphProp \
	testConnection \
	testMessagingGateway \
	testBeliefState \
	testJoiner \
	testJoinLeak \
	testInterfaceRelSat \
	testBeliefStateCaching \
	testRemoveDuplication \
	testDispatcher \
	testBlockingAsio \
	testSATModelOrder \
	testPool \
	testWeakPointer \
	testPolymorphismSharedPtr \
	testTables \
	testID \
	testBridgeRuleEvaluation \
	testMessageDispatcher \
	testResultParser \
	testAbstractContext \
	testDLVProcess \
	testNeighbors

TESTS_ENVIRONMENT = \
	EXAMPLESDIR=$(top_srcdir)/examples \
	BOOST_TEST_REPORT_LEVEL=detailed \
	BOOST_TEST_LOG_LEVEL=all

check_PROGRAMS = \
	testClasp \
	testBeliefComparison \
	testParsers \
	testTranslation \
	testSignature \
	testQueryPlan \
	testClaspResultOnlineParser \
	testBlockingClasp \
	testGraphProp \
	testVariable \
	testMatch \
	testCombination \
	testClaspResultOnlineParser \
	testBlockingClasp \
	testGraphProp \
	testConnection \
	testMessagingGateway \
	testBeliefState \
	testJoiner \
	testJoinLeak \
	testInterfaceRelSat \
	testBeliefStateCaching \
	testRemoveDuplication \
	testDispatcher \
	testBlockingAsio \
	testSATModelOrder \
	testPool \
	testWeakPointer \
	testPolymorphismSharedPtr \
	testTables \
	testID \
	testBridgeRuleEvaluation \
	testMessageDispatcher \
	testResultParser \
	testAbstractContext \
	testDLVProcess \
	testNeighbors

testClasp_SOURCES = testClasp.cpp 
testClasp_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testBeliefComparison_SOURCES = testBeliefComparison.cpp
testBeliefComparison_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testParsers_SOURCES = testParsers.cpp
testParsers_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/dmcs/libdmcs.a

testCombination_SOURCES = testCombination.cpp
testCombination_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testTranslation_SOURCES = testTranslation.cpp
testTranslation_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testSignature_SOURCES = testSignature.cpp

testQueryPlan_SOURCES = testQueryPlan.cpp
testQueryPlan_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/liblog.a

testClaspResultOnlineParser_SOURCES = testClaspResultOnlineParser.cpp
testClaspResultOnlineParser_LDADD = \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testBlockingClasp_SOURCES = testBlockingClasp.cpp
testBlockingClasp_LDADD = \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testMatch_SOURCES = testMatch.cpp
testMatch_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testVariable_SOURCES = testVariable.cpp
testVariable_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testGraphProp_SOURCES = testGraphProp.cpp
testGraphProp_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testMessagingGateway_SOURCES = testMessagingGateway.cpp
testMessagingGateway_LDADD = \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/dmcs/liblog.a

testConnection_SOURCES = testConnection.cpp
testConnection_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/dmcs/liblog.a

testBeliefState_SOURCES = BeliefStateServer.cpp \
	BeliefStateClient.cpp \
	testBeliefState.cpp
testBeliefState_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a

testBeliefStateCaching_SOURCES = testBeliefStateCaching.cpp

testJoiner_SOURCES = testJoiner.cpp
testJoiner_LDADD = \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/liblog.a

testJoinLeak_SOURCES = testJoinLeak.cpp
testJoinLeak_LDADD = \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/liblog.a

testInterfaceRelSat_SOURCES = testInterfaceRelSat.cpp
testInterfaceRelSat_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/relsat-20070104/librelsat.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/liblog.a

testRemoveDuplication_SOURCES = testRemoveDuplication.cpp
testRemoveDuplication_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testDispatcher_SOURCES = testDispatcher.cpp
testDispatcher_LDADD = $(top_builddir)/src/network/libthreads.a \
			     $(top_builddir)/src/dmcs/liblog.a

testBlockingAsio_SOURCES = SimpleClient.cpp \
			   SimpleServer.cpp \
			   testBlockingAsio.cpp

testSATModelOrder_SOURCES = testSATModelOrder.cpp
testSATModelOrder_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/relsat-20070104/librelsat.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/liblog.a


testPool_SOURCES = testPool.cpp
testPool_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/network/libnetwork.a

testWeakPointer_SOURCES = Child.cpp \
	Parent.cpp \
	testWeakPointer.cpp

testPolymorphismSharedPtr_SOURCES = testPolymorphismSharedPtr.cpp 

testTables_SOURCES = testTables.cpp
testTables_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a

testID_SOURCES = testID.cpp
testID_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a

testBridgeRuleEvaluation_SOURCES = testBridgeRuleEvaluation.cpp
testBridgeRuleEvaluation_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a

testMessageDispatcher_SOURCES = testMessageDispatcher.cpp 
testMessageDispatcher_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/network/libthreads.a

testResultParser_SOURCES = testResultParser.cpp 
testResultParser_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/parser/libparser.a 

testAbstractContext_SOURCES = testAbstractContext.cpp 
testAbstractContext_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/mcs/libmcs.a \
	@LIBCURL@

testDLVProcess_SOURCES = testDLVProcess.cpp
testDLVProcess_LDADD = \
	$(top_builddir)/src/process/libprocess.a \
	@LIBCURL@

testNeighbors_SOURCES = testNeighbors.cpp \
	NeighborInServer.cpp \
	NeighborOutServer.cpp
testNeighbors_LDADD = \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/dmcs/libdmcs.a

testSystem_SOURCES = testSystem.cpp \
	DmcsClientTest.cpp 
testSystem_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/parser/libparser.a \
	@LIBCURL@