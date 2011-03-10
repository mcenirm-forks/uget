noinst_PROGRAMS = uget-cmd
UGET_CMD_LIBS   = @GIO_LIBS@   @GTHREAD_LIBS@   @CURL_LIBS@   @LIBPWMD_LIBS@

# set the include path found by configure
uget_cmd_CPPFLAGS = -DDATADIR='"$(datadir)"' -I$(top_srcdir)/uglib -I$(top_srcdir)/uget-cmd
uget_cmd_CFLAGS   = @LFS_CFLAGS@  @GIO_CFLAGS@ @GTHREAD_CFLAGS@ @CURL_CFLAGS@ @LIBPWMD_CFLAGS@
uget_cmd_LDFLAGS  = @LFS_LDFLAGS@
uget_cmd_LDADD    = $(top_builddir)/uglib/libuglib.a $(UGET_CMD_LIBS)

uget_cmd_SOURCES  = \
	UgCategory-cmd.c \
	UgApp-cmd.c \
	UgApp-cmd-main.c

noinst_HEADERS    = \
	UgCategory-cmd.h \
	UgApp-cmd.h

