AC_PREREQ(2.57)
AC_INIT(echoicebox, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(ECORE,[ecore])
PKG_CHECK_MODULES(ECORE_FILE,[ecore-file])
PKG_CHECK_MODULES(ECORE_EVAS,[ecore-evas])
PKG_CHECK_MODULES(EVAS,[evas])
PKG_CHECK_MODULES(EDJE,[edje])

AC_OUTPUT([Makefile src/Makefile echoicebox.pc])
