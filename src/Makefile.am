AUTOMAKE_OPTIONS = gnu

SUBDIRS = mcs dmcs generator network parser process 

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
bin_PROGRAMS = new_dmcsd new_dmcsc new_dmcsm

# the new_dmcsd binary
new_dmcsd_SOURCES = \
		new_dmcsd.cpp

new_dmcsd_LDADD = \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/process/libprocess.a \
	$(top_builddir)/src/mcs/libmcs.a \
	@LIBCURL@

# the dmcsc binary
new_dmcsc_SOURCES = \
	new_dmcsc.cpp 

new_dmcsc_LDADD = \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/network/libthreads.a \
	$(top_builddir)/src/mcs/libmcs.a 

# the dmcsm binary
new_dmcsm_SOURCES = \
	new_dmcsm.cpp 

new_dmcsm_LDADD = \
	$(top_builddir)/src/network/libnetwork.a \
	$(top_builddir)/src/parser/libparser.a \
	$(top_builddir)/src/dmcs/libdmcs.a \
	$(top_builddir)/src/mcs/libmcs.a 


