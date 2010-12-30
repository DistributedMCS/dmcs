# global preprocessor flags
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BM_CPPFLAGS) \
	$(BOOST_CPPFLAGS)

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
	$(BOOST_SYSTEM_LIBS)


TESTS = \
	testClasp \
	testBeliefComparison \
	testParsers \
	testTranslation \
	testSignature \
	testCombination \
	testClaspResultOnlineParser \
	testBlockingClasp \
	testGraphProp

TESTS_ENVIRONMENT = EXAMPLESDIR=$(top_srcdir)/examples

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
	testGraphProp

testClasp_SOURCES = testClasp.cpp 
testClasp_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/dmcs/libsatsolver.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testBeliefComparison_SOURCES = testBeliefComparison.cpp
testBeliefComparison_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testParsers_SOURCES = testParsers.cpp
testParsers_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testCombination_SOURCES = testCombination.cpp
testCombination_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testTranslation_SOURCES = testTranslation.cpp
testTranslation_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testSignature_SOURCES = testSignature.cpp

testQueryPlan_SOURCES = testQueryPlan.cpp
testQueryPlan_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testClaspResultOnlineParser_SOURCES = testClaspResultOnlineParser.cpp
testClaspResultOnlineParser_LDADD = \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/dmcs/libsatsolver.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testBlockingClasp_SOURCES = testBlockingClasp.cpp
testBlockingClasp_LDADD = \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/dmcs/libsatsolver.a \
	$(top_builddir)/src/loopformula/libtranslation.a

testMatch_SOURCES = testMatch.cpp
testMatch_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testVariable_SOURCES = testVariable.cpp
testVariable_LDADD = $(top_builddir)/src/dmcs/libdmcs.a

testGraphProp_SOURCES = testGraphProp.cpp
testGraphProp_LDADD = $(top_builddir)/src/dmcs/libdmcs.a
