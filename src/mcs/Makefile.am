AUTOMAKE_OPTIONS = gnu

# global preprocessor flags
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir) \
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
	$(BOOST_SERIALIZATION_LIBS) 
	$(BOOST_GRAPH_LIBS)


noinst_LIBRARIES = libmcs.a

# the mcs library
libmcs_a_SOURCES = \
		BackwardMessage.cpp \
		BaseJoiner.cpp \
		Belief.cpp \
		BeliefStateOffset.cpp \
		BridgeRuleEvaluator.cpp \
		ForwardMessage.cpp \
		ID.cpp \
		Logger.cpp \
		NewBeliefState.cpp \
		Pool.cpp \
		QueryID.cpp \
	  	ResourceManager.cpp \
		ReturnedBeliefState.cpp \
		StreamingJoiner.cpp 