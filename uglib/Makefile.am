# To enable LFS (Large File Support) in 32bit platform
# add `getconf LFS_CFLAGS`  to CFLAGS
# add `getconf LFS_LDFLAGS` to LDFLAGS

# static library ---

#  lib_LIBRARIES = libuglib.a
noinst_LIBRARIES = libuglib.a

libuglib_a_CPPFLAGS = -I$(top_srcdir)/uglib
libuglib_a_CFLAGS   = @LFS_CFLAGS@ @GLIB_CFLAGS@ @GIO_CFLAGS@ @GTHREAD_CFLAGS@ @CURL_CFLAGS@ @LIBPWMD_CFLAGS@

libuglib_a_SOURCES  = uglib.c  \
	UgOption.c  UgMarkup.c   UgIpc.c  UgRegistry.c  \
	UgStdio.c   UgString.c   UgUri.c  UgUtils.c  UgHtml.c  UgXmlrpc.c  \
	UgData.c    UgDataset.c  UgData-download.c  UgCategory.c  UgMessage.c  \
	UgRunning.c UgPlugin.c   UgPlugin-aria2.c  UgPlugin-curl.c pwmd.c

noinst_HEADERS      = uglib.h  \
	UgOption.h  UgMarkup.h   UgIpc.h  UgRegistry.h  \
	UgStdio.h   UgString.h   UgUri.h  UgUtils.h  UgHtml.h  UgXmlrpc.h  \
	UgData.h    UgDataset.h  UgData-download.h  UgCategory.h  UgMessage.h  \
	UgRunning.h UgPlugin.h   UgPlugin-aria2.h  UgPlugin-curl.h pwmd.h

