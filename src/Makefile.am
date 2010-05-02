AUTOMAKE_OPTIONS = gnu

SUBDIRS = tests

# global preprocessor flags
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    $(BOOST_CPPFLAGS)

# global linker flags
AM_LDFLAGS = $(BOOST_DATE_TIME_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_THREAD_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) $(BOOST_GRAPH_LDFLAGS)

# global libs
LIBS = $(BOOST_DATE_TIME_LIBS) $(BOOST_PROGRAM_OPTIONS_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_THREAD_LIBS) $(BOOST_SERIALIZATION_LIBS) $(BOOST_GRAPH_LIBS)

# install dmcsd and dmcsc into $prefix/bin
bin_PROGRAMS = dmcsd dmcsc 
noinst_LIBRARIES = libsatsolver.a libcache.a libtranslation.a libdmcs.a

noinst_PROGRAMS = dmcsGen

# the translation library
libtranslation_a_SOURCES = \
		IMPLocalLoopFormulaBuilder.cpp \
		LocalLoopFormulaBuilder.cpp \
		CNFLocalLoopFormulaBuilder.cpp \
		DimacsVisitor.cpp 


# the cache library
libcache_a_SOURCES = Cache.cpp


# the satsolver library
libsatsolver_a_SOURCES = \
                ClaspProcess.cpp \
		DimacsVisitor.cpp \
		ProcessBuf.cpp \
		LogicVisitor.cpp

# the DMCS library
libdmcs_a_SOURCES = \
		BaseDMCS.cpp \
		OptDMCS.cpp \
		PrimitiveDMCS.cpp \
                QueryPlan.cpp \
                BeliefCombination.cpp

# the dmcsd binary
dmcsd_SOURCES = \
		dmcsd.cpp

dmcsd_LDADD = libdmcs.a libsatsolver.a libcache.a libtranslation.a


# the dmcsc binary
dmcsc_SOURCES = \
		dmcsc.cpp 

dmcsc_LDADD = libdmcs.a

# the dmcsGen binary
dmcsGen_SOURCES = \
                  ContextGenerator.cpp \
                  LogicVisitor.cpp \
		  DLVVisitor.cpp \
                  QueryPlanGenerator.cpp \
                  genTest.cpp

dmcsGen_LDADD = libdmcs.a