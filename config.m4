dnl $Id$
dnl config.m4 for extension xmldiff

PHP_ARG_WITH(xmldiff, for xmldiff support,
[  --with-xmldiff          Include XMLDiff support])
PHP_ARG_WITH(libdiffmark, whether to use system diffmark library,
[  --with-libdiffmark=DIR  XMLDiff: diffmark install prefix], no, no)

if test -z "$PHP_LIBXML_DIR"; then
  PHP_ARG_WITH(libxml-dir, libxml2 install dir,
  [  --with-libxml-dir=DIR   XMLDiff: libxml2 install prefix], no, no)
fi

if test "$PHP_XMLDIFF" != "no"; then
  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(stdc++,,XMLDIFF_SHARED_LIBADD)

  if test "$PHP_LIBXML" = "no"; then
    AC_MSG_ERROR([XMLDiff extension requires LIBXML extension])
  fi
  if test "$PHP_DOM" = "no"; then
    AC_MSG_ERROR([XMLDiff extension requires DOM extension])
  fi

  XMLDIFF_CFLAGS="-Wall"

  if test "$PHP_LIBDIFFMARK" != "no"; then
    SEARCH_PATH="/usr /usr/local"
    SEARCH_FOR="/include/diffmark/diff.hh"
    AC_MSG_CHECKING([for diffmark headers])
    if test -r $PHP_LIBDIFFMARK/$SEARCH_FOR; then # path given as parameter
      LIBDIFFMARK_DIR=$PHP_LIBDIFFMARK
    else
      for i in $SEARCH_PATH ; do
        if test -r $i/$SEARCH_FOR; then
          LIBDIFFMARK_DIR=$i
        fi
      done
    fi
  
    if test -z "$LIBDIFFMARK_DIR"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([The required diffmark library was not found.])
    else
      AC_MSG_RESULT(found in $LIBDIFFMARK_DIR)
    fi

    PHP_ADD_INCLUDE($LIBDIFFMARK_DIR/include/diffmark)
    AC_DEFINE(HAVE_LIBDIFFMARK,1,[Use system diffmark library])
    PHP_ADD_LIBRARY_WITH_PATH(diffmark, $LIBDIFFMARK_DIR/$PHP_LIBDIR, XMLDIFF_SHARED_LIBADD) 

    PHP_SETUP_LIBXML(XMLDIFF_SHARED_LIBADD, [
      AC_DEFINE(HAVE_XMLDIFF,1,[ ])
      PHP_SUBST(XMLDIFF_SHARED_LIBADD)
      PHP_NEW_EXTENSION(xmldiff, xmldiff.cpp, $ext_shared,, $XMLDIFF_CFLAGS)
      PHP_ADD_EXTENSION_DEP(xmldiff, dom, true)
      PHP_ADD_EXTENSION_DEP(xmldiff, libxml, true)
      PHP_ADD_INCLUDE($ext_srcdir/simplexml_compat)
      PHP_ADD_INCLUDE($ext_builddir/simplexml_compat)
      PHP_INSTALL_HEADERS([ext/xmldiff/php_xmldiff.h])
    ], [
      AC_MSG_ERROR([xml2-config not found. Please check your libxml2 installation.])
    ])
  else
    PHP_DIFFMARK_SOURCES="diffmark/lib/compare.cc \
      diffmark/lib/diff.cc \
      diffmark/lib/link.cc \
      diffmark/lib/merge.cc \
      diffmark/lib/namespacecollector.cc \
      diffmark/lib/nspace.cc \
      diffmark/lib/target.cc \
      diffmark/lib/xbuffer.cc \
      diffmark/lib/xdoc.cc \
      diffmark/lib/xutil.cc"

    PHP_SETUP_LIBXML(XMLDIFF_SHARED_LIBADD, [
      AC_DEFINE(HAVE_XMLDIFF,1,[ ])
      PHP_SUBST(XMLDIFF_SHARED_LIBADD)
      PHP_NEW_EXTENSION(xmldiff, $PHP_DIFFMARK_SOURCES xmldiff.cpp, $ext_shared,, $XMLDIFF_CFLAGS)
      PHP_ADD_EXTENSION_DEP(xmldiff, dom, true)
      PHP_ADD_EXTENSION_DEP(xmldiff, libxml, true)
      PHP_ADD_BUILD_DIR($ext_builddir/diffmark/lib)
      PHP_ADD_INCLUDE($ext_srcdir/diffmark/lib)
      PHP_ADD_INCLUDE($ext_builddir/diffmark/lib)
      PHP_ADD_INCLUDE($ext_srcdir/simplexml_compat)
      PHP_ADD_INCLUDE($ext_builddir/simplexml_compat)
      PHP_INSTALL_HEADERS([ext/xmldiff/php_xmldiff.h])
      PHP_INSTALL_HEADERS([diffmark/lib/compare.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/compareimpl.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/diff.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/lcs.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/lcsimpl.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/link.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/merge.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/namespacecollector.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/nspace.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/target.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/xbuffer.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/xdoc.hh])
      PHP_INSTALL_HEADERS([diffmark/lib/xutil.hh])
    ], [
      AC_MSG_ERROR([xml2-config not found. Please check your libxml2 installation.])
    ])
  fi

fi
