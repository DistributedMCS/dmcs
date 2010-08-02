# global preprocessor flags
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    $(BOOST_CPPFLAGS)

# global linker flags
AM_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS) $(BOOST_THREAD_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)

# global libs
LIBS = $(BOOST_THREAD_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_SERIALIZATION_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(BOOST_SYSTEM_LIBS)


TESTS = \
	testClasp \
	testBeliefComparison \
	testParsers \
	testTranslation \
	testSignature \
	testCombination \
	testClaspResultOnlineParser

TESTS_ENVIRONMENT = EXAMPLESDIR=$(top_srcdir)/examples

check_PROGRAMS = \
	testClasp \
	testBeliefComparison \
	testParsers \
	testTranslation \
	testSignature \
	testQueryPlan \
	testCombination \
	testClaspResultOnlineParser

testClasp_SOURCES = testClasp.cpp 
testClasp_LDADD = $(top_builddir)/src/libsatsolver.a $(top_builddir)/src/libdmcs.a

testBeliefComparison_SOURCES = testBeliefComparison.cpp
testBeliefComparison_LDADD = $(top_builddir)/src/libdmcs.a

testParsers_SOURCES = testParsers.cpp
testParsers_LDADD = $(top_builddir)/src/libdmcs.a

testCombination_SOURCES = testCombination.cpp
testCombination_LDADD = $(top_builddir)/src/libdmcs.a

testTranslation_SOURCES = testTranslation.cpp
testTranslation_LDADD = $(top_builddir)/src/libdmcs.a $(top_builddir)/src/libtranslation.a

testSignature_SOURCES = testSignature.cpp

testQueryPlan_SOURCES = testQueryPlan.cpp
testQueryPlan_LDADD = $(top_builddir)/src/libdmcs.a

testClaspResultOnlineParser_SOURCES = testClaspOnlineParser.cpp
testClaspResultOnlineParser_LDADD = $(top_builddir)/src/libsatsolver.a
