AUTOMAKE_OPTIONS = gnu

SUBDIRS = dmcs dyndmcs generator network loopformula parser process solver mcs

# global preprocessor flags
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir) \
    $(BM_CPPFLAGS) \
    $(BOOST_CPPFLAGS) \
    $(LOG4CXX_CFLAGS) \
    -DNO_GMP \
    @LIBCURL_CPPFLAGS@


# global linker flags
AM_LDFLAGS = \
	$(BOOST_DATE_TIME_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LDFLAGS) \
	$(BOOST_SYSTEM_LDFLAGS) \
	$(BOOST_THREAD_LDFLAGS) \
	$(BOOST_SERIALIZATION_LDFLAGS) \
	$(BOOST_ASIO_LDFLAGS) \
	$(BOOST_GRAPH_LDFLAGS) 

# global libs
LIBS = \
	$(BOOST_DATE_TIME_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(BOOST_SYSTEM_LIBS) \
	$(BOOST_THREAD_LIBS) \
	$(BOOST_SERIALIZATION_LIBS) \
	$(BOOST_ASIO_LIBS) \
	$(BOOST_GRAPH_LIBS) \
	$(LOG4CXX_LIBS)

# install dmcsd and dmcsc into $prefix/bin
bin_PROGRAMS = dmcsd dmcsc 

# the dmcsd binary
dmcsd_SOURCES = \
		dmcsd.cpp

dmcsd_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/dmcs/libcache.a \
	$(top_builddir)/src/loopformula/libtranslation.a \
	$(top_builddir)/src/dyndmcs/libdyndmcs.a \
	$(top_builddir)/src/solver/libsolver.a \
	$(top_builddir)/src/mcs/libmcs.a \
	$(top_builddir)/relsat-20070104/librelsat.a \
	@LIBCURL@


# the dmcsc binary
dmcsc_SOURCES = \
		dmcsc.cpp 

dmcsc_LDADD = \
	$(top_builddir)/src/dmcs/liblog.a \
	$(top_builddir)/src/dmcs/libdmcs.a
