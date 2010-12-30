
noinst_PROGRAMS = test-download  test-conf  test-ipc

# set the include path found by configure
AM_CPPFLAGS = -I$(top_srcdir)/uglib -I$(top_srcdir)/tests 
AM_CFLAGS   = @LFS_CFLAGS@  @GLIB_CFLAGS@ @GIO_CFLAGS@ @GTHREAD_CFLAGS@ @GMODULE_CFLAGS@ @CURL_CFLAGS@ @LIBPWMD_CFLAGS@
AM_LDFLAGS  = @LFS_LDFLAGS@ @GLIB_LIBS@   @GIO_LIBS@   @GTHREAD_LIBS@   @GMODULE_LIBS@   @CURL_LIBS@   @LIBPWMD_LIBS@

test_download_LDADD    = $(top_builddir)/uglib/libuglib.a
test_download_SOURCES  = test-download.c

test_conf_LDADD    = $(top_builddir)/uglib/libuglib.a
test_conf_SOURCES  = test-conf.c

test_ipc_LDADD    = $(top_builddir)/uglib/libuglib.a
test_ipc_SOURCES  = test-ipc.c

