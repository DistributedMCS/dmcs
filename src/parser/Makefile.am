AUTOMAKE_OPTIONS = gnu

# global preprocessor flags
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BM_CPPFLAGS) \
	$(BOOST_CPPFLAGS)

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
	$(BOOST_GRAPH_LIBS)

noinst_LIBRARIES = libparser.a

# the parser library
libparser_a_SOURCES = \
		BridgeRuleParser.cpp \
		DLVResultGrammar.cpp \
		DLVResultParser.cpp \
		NewDLVResultParser.cpp



