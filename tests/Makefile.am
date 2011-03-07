
noinst_PROGRAMS = test-download  test-conf  test-ipc  test-xmlrpc
TESTS_LIBS      = @GLIB_LIBS@   @GIO_LIBS@   @GTHREAD_LIBS@   @GMODULE_LIBS@   @CURL_LIBS@   @LIBPWMD_LIBS@

# set the include path found by configure
AM_CPPFLAGS = -I$(top_srcdir)/uglib -I$(top_srcdir)/tests 
AM_CFLAGS   = @LFS_CFLAGS@  @GLIB_CFLAGS@ @GIO_CFLAGS@ @GTHREAD_CFLAGS@ @GMODULE_CFLAGS@ @CURL_CFLAGS@ @LIBPWMD_CFLAGS@
AM_LDFLAGS  = @LFS_LDFLAGS@

test_download_LDADD    = $(top_builddir)/uglib/libuglib.a $(TESTS_LIBS)
test_download_SOURCES  = test-download.c

test_conf_LDADD    = $(top_builddir)/uglib/libuglib.a $(TESTS_LIBS)
test_conf_SOURCES  = test-conf.c

test_ipc_LDADD    = $(top_builddir)/uglib/libuglib.a $(TESTS_LIBS)
test_ipc_SOURCES  = test-ipc.c

test_xmlrpc_LDADD    = $(top_builddir)/uglib/libuglib.a $(TESTS_LIBS)
test_xmlrpc_SOURCES  = test-xmlrpc.c

