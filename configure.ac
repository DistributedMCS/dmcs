AC_PREREQ([2.61])
AC_INIT([dmcs],[1.0.0alpha],[dmcs-devel@lists.sourceforge.net])
AC_CONFIG_SRCDIR([src/dmcsd.cpp])
AC_CONFIG_HEADER([config.h])

AC_CANONICAL_HOST


#
# the user didn't supply a path for dlv
#
if test "$DLV" = ""; then
    # we assume "dlv"
    DLV=dlv
    # and check if it exists
    AC_CHECK_PROG(DLVPATH, $DLV, $DLV, notfound)
else
    # the user supplied a 'DLV=...'
    # check if it exists (with search path=/, since the user supplied
    # an entire path!) and set DLVPATH to it!
    AC_CHECK_PROG(DLVPATH, $DLV, $DLV, notfound, [/])
fi

if test "$DLVPATH" = notfound; then
  AC_MSG_WARN([dlv not found. Download it at http://www.dlvsystem.com/])
else
  extsolver_found=true
  AC_DEFINE([HAVE_DLV], [1], [Defined if we found dlv.])
  AC_DEFINE_UNQUOTED(DLVPATH,"$DLVPATH",[Path of DLV executable])
fi



#
# the user didn't supply a path for clasp
#
if test "$CLASP" = ""; then
    # we assume "clasp"
    CLASP=clasp
    # and check if it exists
    AC_CHECK_PROG(CLASPPATH, $CLASP, $CLASP, notfound)
else
    # the user supplied a 'CLASP=...'
    # check if it exists (with search path=/, since the user supplied
    # an entire path!) and set CLASPPATH to it!
    AC_CHECK_PROG(CLASPPATH, $CLASP, $CLASP, notfound, [/])
fi

if test "$CLASPPATH" = notfound; then
    AC_MSG_ERROR([clasp not found, get it from http://www.cs.uni-potsdam.de/clasp/])
fi


AC_MSG_CHECKING([for clasp version]) # check for clasp >= 1.3.4

CLASPVERSION=`$CLASPPATH --version 2>&1 | sed -ne 's/^.*clasp[[^0-9]]*//p'`
CLASPMAJOR=`echo $CLASPVERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/"`
CLASPMINOR=`echo $CLASPVERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/"`
CLASPMICRO=`echo $CLASPVERSION | sed "s/^\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/"`

AC_MSG_RESULT([$CLASPVERSION])

# for check clasp >= 1.99.68 we have to do some arithmetic here, but
# for now it's good enough
if test "$CLASPMAJOR"0"$CLASPMINOR"0"$CLASPMICRO" -lt 10304; then 
   AC_MSG_ERROR([could not find clasp >= 1.3.4, please upgrade])
fi

#
# put the clasp path into config.h
#
AC_DEFINE_UNQUOTED(CLASPPATH,"$CLASPPATH",[Path of clasp executable])


DX_HTML_FEATURE([ON])
DX_CHM_FEATURE([OFF])
DX_CHI_FEATURE([OFF])
DX_MAN_FEATURE([OFF])
DX_RTF_FEATURE([OFF])
DX_XML_FEATURE([OFF])
DX_PDF_FEATURE([OFF])
DX_PS_FEATURE([OFF])
DX_INIT_DOXYGEN([dmcs], [doxygen.cfg], [doc])


AM_INIT_AUTOMAKE

# Check for C/C++ compiler
save_CFLAGS="$CFLAGS"
save_CXXFLAGS="$CXXFLAGS"
${CFLAGS=""}
${CXXFLAGS=""}
AC_PROG_CC([clang gcc cc])
AC_PROG_CXX([clang++ g++ c++])
AC_PROG_CXXCPP([clang gcc cc])

CFLAGS="$CFLAGS $save_CFLAGS"
CXXFLAGS="$CXXFLAGS $save_CFLAGS"

# set default warning parameters for g++ compatible compilers
if test "$GXX" = "yes"; then
   CXXFLAGS="$CXXFLAGS -W -Wall"
   CFLAGS="$CFLAGS -W -Wall"
fi

# Checks for header files.
AC_HEADER_DIRENT
AC_CHECK_HEADERS([sys/param.h])

# by default we look into /opt/local and /sw for include files (macports vs. fink)
#case $host_os in
#     darwin*|macosx*)
#	CPPFLAGS="$CPPFLAGS -I/opt/local/include -I/sw/include"
#     ;;
#esac 


LT_INIT
AC_CONFIG_MACRO_DIR([m4])

#
# Check for external libraries
#


#
# check for libcurl
#
LIBCURL_CHECK_CONFIG([], [], [], [AC_MSG_ERROR([could not find CURL library. Download it at http://curl.haxx.se/.])])


#
# check for bmagic
#
AC_ARG_WITH(bmagic,
            [  --with-bmagic=DIR       Prefix of bitmagic library (DIR=local forces local bmagic)]
)

BM_CPPFLAGS=""
BM_VERSION=""

if test x$with_bmagic != xlocal; then

   TMP_CPPFLAGS=$CPPFLAGS
   CPPFLAGS="$CPPFLAGS -I$with_bmagic"
   AC_LANG_PUSH([C++])
   AC_CHECK_HEADER([bm/bm.h], [have_bm=true], [have_bm=false])
   # todo: do some sanity compilations with this bmagic
   AC_LANG_POP([C++])
   CPPFLAGS=$TMP_CPPFLAGS

   if test x$have_bm = xfalse; then
         AC_MSG_WARN([bmagic not found. Using local bmagic.])
   else
      BM_CPPFLAGS="-I$with_bmagic"
      have_bm=true
   fi

fi

if test x$have_bm != xtrue; then
   # force local bmagic
   BM_CPPFLAGS="-I\${top_srcdir}/bm3.7.0/"
   BM_VERSION=3.7.0
fi

AC_SUBST([BM_CPPFLAGS])

# checking for boost libs
BOOST_REQUIRE([1.42.0])
BOOST_SMART_PTR
BOOST_STRING_ALGO
BOOST_TOKENIZER
BOOST_IOSTREAMS
BOOST_GRAPH #right now, we don't care about the libboost-graph shared library
BOOST_PROGRAM_OPTIONS
BOOST_SYSTEM
BOOST_THREADS
BOOST_SERIALIZATION
BOOST_TEST
BOOST_DATE_TIME
BOOST_ASIO

# checking for log4cxx
PKG_CHECK_MODULES([LOG4CXX], [liblog4cxx >= 0.10.0])

# debug
AC_ARG_ENABLE(debug,
              [  --enable-debug          switch on debug routines (timers, ...)],
              [AC_DEFINE(DEBUG, [], [Turns on timer routines for profiling])
               debug_support=yes],
	      [CPPFLAGS="-DNDEBUG" CXXFLAGS="`echo $CXXFLAGS | sed 's/-O[[0-9]]//g'` -O3" CFLAGS="`echo $CFLAGS | sed 's/-O[[0-9]]//g'` -O3"]
              )


# stats
AC_ARG_ENABLE(stats,
              [  --enable-stats          switch on statistic information],
              [AC_DEFINE(DMCS_STATS_INFO, [], [Turns on statistic information])
               stats_support=yes]
              )


# Call relsat's ./configure script recursively.
AC_CONFIG_SUBDIRS([relsat-20070104])


# output autogenerated files
AC_CONFIG_FILES([
           Makefile 
           include/Makefile
           include/dmcs/Makefile
           include/generator/Makefile
           include/mcs/Makefile
           include/network/Makefile
           include/parser/Makefile
           include/process/Makefile
           include/solver/Makefile
           src/Makefile
           src/dmcs/Makefile
           src/generator/Makefile
           src/mcs/Makefile
           src/network/Makefile
           src/parser/Makefile
           src/process/Makefile
           src/solver/Makefile
           testsuite/Makefile
])


AC_OUTPUT

dnl output configuration
cat <<EOF
 
$PACKAGE_TARNAME $PACKAGE_VERSION configuration:

 - Compiler settings
 CC: $CC
 CXX: $CXX
 CFLAGS: $CFLAGS
 CPPFLAGS: $CPPFLAGS
 CXXFLAGS: $CXXFLAGS
 LDFLAGS: $LDFLAGS

 - Path settings
 prefix: $prefix
 exec_prefix: $exec_prefix

 bindir: $bindir
 libdir: $libdir
 pkglibdir: $libdir/$PACKAGE_TARNAME
 datadir: $datadir

 - Using the following binaries:
 clasp: $CLASPPATH

 - Using the following libraries:
 Bitmagic: $BM_VERSION $BM_CPPFLAGS
 Boost: $boost_major_version $BOOST_CPPFLAGS
 relsat:
 minisat:
 picosat:

 - Using the following solver backends:
 DLV: $DLVPATH

 - Options:
 Debug: $debug_support
 Stats: $stats_support

EOF
