AC_PREREQ(2.57)
AC_INIT(libchoicebox, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(EFL,[eina-0 evas edje])
PKG_CHECK_MODULES(ECORE,[ecore ecore-evas])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])

AC_OUTPUT([Makefile src/Makefile themes/Makefile po/Makefile data/Makefile libchoicebox.pc])
