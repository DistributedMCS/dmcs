AUTOMAKE_OPTIONS = gnu

#CLEANFILES = 

#EXTRA_DIST = run-dmcs-tests.sh

# global preprocessor flags
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    $(BOOST_CPPFLAGS)

# global linker flags
AM_LDFLAGS = $(BOOST_SYSTEM_LDFLAGS) $(BOOST_THREAD_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS)

# global libs
LIBS = $(BOOST_THREAD_LIBS) $(BOOST_GRAPH_LIBS) $(BOOST_SERIALIZATION_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(BOOST_SYSTEM_LIBS)


TESTS = testClasp testParsers testTranslation testSolver testSignature # testMCSParser testCombination testBeliefComparison
TESTS_ENVIRONMENT = EXAMPLESDIR=$(top_srcdir)/examples

check_PROGRAMS = testClasp testParsers testTranslation testSolver testSignature testQueryPlan testReadGraph #testMCSParser testSolver testCombination testBeliefComparison

testClasp_SOURCES = testClasp.cpp 
testClasp_LDADD = $(top_builddir)/src/libsatsolver.a # $(top_builddir)/src/libcontext.a

testParsers_SOURCES = testParsers.cpp
testParsers_LDADD = $(top_builddir)/src/libcache.a

testTranslation_SOURCES = testTranslation.cpp
testTranslation_LDADD = $(top_builddir)/src/libtranslation.a

testSolver_SOURCES = testSolver.cpp
testSolver_LDADD = $(top_builddir)/src/libsatsolver.a $(top_builddir)/src/libcache.a $(top_builddir)/src/libtranslation.a

testSignature_SOURCES = testSignature.cpp

testReadGraph_SOURCES = testReadGraph.cpp

testQueryPlan_SOURCES = testQueryPlan.cpp \
                        $(top_builddir)/src/QueryPlan.cpp 

testCombination_SOURCES = testCombination.cpp
testBeliefComparison_SOURCES = testBeliefComparison.cpp
testMCSParser_SOURCES = testMCSParser.cpp





