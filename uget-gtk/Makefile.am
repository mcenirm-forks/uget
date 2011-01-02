bin_PROGRAMS = uget-gtk

FOR_GTK3_CFLAGS = -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE -DGTK_DISABLE_SINGLE_INCLUDES

# set the include path found by configure
uget_gtk_CPPFLAGS = -DDATADIR='"$(datadir)"' -I$(top_srcdir)/uglib -I$(top_srcdir)/uget-gtk $(FOR_GTK3_CFLAGS)
uget_gtk_CFLAGS   = @LFS_CFLAGS@  @GTK_CFLAGS@ @GIO_CFLAGS@ @GTHREAD_CFLAGS@ @CURL_CFLAGS@ @LIBNOTIFY_CFLAGS@ @GSTREAMER_CFLAGS@ @LIBPWMD_CFLAGS@
uget_gtk_LDFLAGS  = @LFS_LDFLAGS@ @GTK_LIBS@   @GIO_LIBS@   @GTHREAD_LIBS@   @CURL_LIBS@   @LIBNOTIFY_LIBS@   @GSTREAMER_LIBS@   @LIBPWMD_LIBS@
uget_gtk_LDADD    = $(top_builddir)/uglib/libuglib.a

uget_gtk_SOURCES  = \
	UgSelector.c  UgSummary.c  UgSettingForm.c  UgScheduleGrid.c \
	UgDownloadForm.c    UgCategoryForm.c    UgProxyForm.c  UgBatchForm.c \
	UgDownloadWidget.c  UgCategoryWidget.c  UgCategory-gtk.c \
	UgDownloadDialog.c  UgCategoryDialog.c  UgSettingDialog.c \
	UgetGtk.c  UgetGtk-setting.c  UgetGtk-gui.c \
	UgetGtk-callback.c  UgetGtk-timeout.c \
	main-gtk.c

noinst_HEADERS    = \
	UgSelector.h  UgSummary.h  UgSettingForm.h  UgScheduleGrid.h \
	UgDownloadForm.h    UgCategoryForm.h    UgProxyForm.h  UgBatchForm.h \
	UgDownloadWidget.h  UgCategoryWidget.h  UgCategory-gtk.h \
	UgDownloadDialog.h  UgCategoryDialog.h  UgSettingDialog.h \
	UgetGtk.h  UgetGtk-setting.h

