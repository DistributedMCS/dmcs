AUTOMAKE_OPTIONS = gnu

# global preprocessor flags
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BM_CPPFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(LOG4CXX_CFLAGS)

# global linker flags
AM_LDFLAGS = \
	$(BOOST_DATE_TIME_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_THREAD_LDFLAGS) \
	$(BOOST_SERIALIZATION_LDFLAGS) \
	$(BOOST_GRAPH_LDFLAGS)

# global libs
LIBS = \
	$(BOOST_DATE_TIME_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREAD_LIBS) \
	$(BOOST_SERIALIZATION_LIBS) \
	$(BOOST_GRAPH_LIBS) \
	$(LOG4CXX_LIBS)


noinst_PROGRAMS = dmcsGen dynGen new_dmcsGen 

dmcsGen_SOURCES = \
		dmcsGen.cpp \
		LogicVisitor.cpp \
		ContextGenerator.cpp \
		staticGen.cpp

dmcsGen_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/dmcs/libdmcs.a 

new_dmcsGen_SOURCES = \
		new_staticGen.cpp \
		new_dmcsGen.cpp \
		LogicVisitor.cpp \
		NewContextGenerator.cpp

new_dmcsGen_LDADD = \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/dmcs/libdmcs.a

dynGen_SOURCES = dynGen.cpp

dynGen_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a
