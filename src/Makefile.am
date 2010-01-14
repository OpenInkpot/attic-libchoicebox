lib_LTLIBRARIES = libchoicebox.la
libchoicebox_la_SOURCES = libchoicebox.c libchoicebox-aux.c
include_HEADERS = libchoicebox.h

AM_CFLAGS=--std=gnu99 -Wall -DDATADIR='"$(datadir)"'

INCLUDES = $(EFL_CFLAGS) $(LIBKEYS_CFLAGS) $(ECORE_CFLAGS)
libchoicebox_la_LDFLAGS = $(EFL_LIBS) $(LIBKEYS_LIBS) -Wl,--no-undefined -version-info 2:0:1

noinst_PROGRAMS = teste
teste_SOURCES = teste.c
teste_LDFLAGS = libchoicebox.la $(ECORE_LIBS)
