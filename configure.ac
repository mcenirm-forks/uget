AC_PREREQ([2.60])
AC_INIT([uget],[1.10.3])

## Use automake (add automake to autogen.sh)
AM_INIT_AUTOMAKE

## Use config.h (add autoheader to autogen.sh)
AC_CONFIG_HEADERS([config.h])

## Enable platform related extensions
AC_USE_SYSTEM_EXTENSIONS

## Determine a C compiler to use.
AC_PROG_CC

## Check C compiler -c -o options.
AM_PROG_CC_C_O

## Use library (static library)
AC_PROG_RANLIB

## Use libtool (share and static library) (add libtoolize to autogen.sh)
# AC_PROG_LIBTOOL
# LT_PREREQ([2.2.6])
# LT_INIT


## ----------------------------------------------
## L10N  (add intltoolize to autogen.sh)
AC_PROG_INTLTOOL
## replace ALL_LINGUAS with po/LINGUAS
# ALL_LINGUAS=""
GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(LOCALEDIR)
## Use AM_GLIB_DEFINE_LOCALEDIR with AC_CONFIG_HEADERS


## ----------------------------------------------
## GTK+ related
PKG_CHECK_MODULES(GTK, gtk+-3.0 >= 3.4)
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.32)
PKG_CHECK_MODULES(GTHREAD, gthread-2.0)
PKG_CHECK_MODULES(GMODULE, gmodule-2.0)
PKG_CHECK_MODULES(GIO, gio-2.0)

## ----------------
## libnotify
AC_ARG_ENABLE(
	[notify],
	AC_HELP_STRING([--disable-notify], [Disable libnotify support.]),
	[enable_notify="$enableval"],
	[enable_notify="yes"]
)
if test "x$enable_notify" = "xyes"; then
  PKG_CHECK_MODULES(LIBNOTIFY, libnotify)
  AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define to 1 if libnotify support is required.])
  # for ArchLinux
  AC_SUBST(LIBNOTIFY_CFLAGS)
  AC_SUBST(LIBNOTIFY_LIBS)
fi

## ----------------
## appindicator
AC_ARG_ENABLE(
	[appindicator],
	AS_HELP_STRING([--enable-appindicator[=@<:@no/auto/yes@:>@]],[Build support for application indicators.]),
	[enable_appindicator=$enableval],
	[enable_appindicator="auto"]
)
if test x$enable_appindicator = xauto ; then
	PKG_CHECK_EXISTS([appindicator3-0.1],
	                 [enable_appindicator="yes"],
	                 [enable_appindicator="no"])
fi
if test x$enable_appindicator = xyes ; then
	PKG_CHECK_MODULES(APP_INDICATOR, appindicator3-0.1)
	AC_SUBST(APP_INDICATOR_CFLAGS)
	AC_SUBST(APP_INDICATOR_LIBS)
	AC_DEFINE(HAVE_APP_INDICATOR, 1, [Have AppIndicator])
fi
AM_CONDITIONAL(HAVE_APP_INDICATOR, test x"$enable_appindicator" = xyes)

## ----------------
## gstreamer
AC_ARG_ENABLE(
	[gstreamer],
	AC_HELP_STRING([--disable-gstreamer], [Disable GStreamer audio support.]),
	[enable_gstreamer="$enableval"],
	[enable_gstreamer="yes"]
)
if test "x$enable_gstreamer" = "xyes"; then
  PKG_CHECK_EXISTS([gstreamer-1.0],
                   [enable_gstreamer1="yes"],
                   [enable_gstreamer1="no"])
fi
if test "x$enable_gstreamer1" = "xyes"; then
  PKG_CHECK_MODULES(GSTREAMER, gstreamer-1.0)
  AC_DEFINE(HAVE_GSTREAMER, 1, [Define to 1 if gstreamer support is required.])
  # for ArchLinux
  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
elif test "x$enable_gstreamer" = "xyes"; then
  PKG_CHECK_MODULES(GSTREAMER, gstreamer-0.10)
  AC_DEFINE(HAVE_GSTREAMER, 1, [Define to 1 if gstreamer support is required.])
  # for ArchLinux
  AC_SUBST(GSTREAMER_CFLAGS)
  AC_SUBST(GSTREAMER_LIBS)
fi

## ----------------
## LFS
AC_PATH_PROG(GETCONF, getconf)
if test "x$GETCONF" != "x" ; then
	LFS_CFLAGS=`$GETCONF LFS_CFLAGS`
	LFS_LDFLAGS=`$GETCONF LFS_LDFLAGS`
fi
AC_SUBST(LFS_CFLAGS)
AC_SUBST(LFS_LDFLAGS)

## ----------------
## cURL
AC_PATH_PROG(CURL_CONFIG, curl-config)
if test "x$CURL_CONFIG" = "x" ; then
	AC_MSG_ERROR(Unable to find curl-config, please install libcurl)
fi
CURL_CFLAGS=`$CURL_CONFIG --cflags`
CURL_LIBS=`$CURL_CONFIG --libs`
let CURL_VERNUM=0x0`$CURL_CONFIG --vernum`
let CURL_VERMIN=0x071301		# 7.19.1
if test $CURL_VERNUM -lt $CURL_VERMIN; then
	AC_MSG_ERROR(Requires libcurl version >= 7.19.1)
fi
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

## ----------------
## cURL plug-in
AC_ARG_ENABLE(
	[plugin-curl],
	AC_HELP_STRING([--disable-plugin-curl], [Disable cURL plug-in.]),
	[plugin_curl="$enableval"],
	[plugin_curl="yes"]
)

if test "x$plugin_curl" = "xyes"; then
    AC_DEFINE(HAVE_PLUGIN_CURL, 1, [Define to 1 to enable cURL plug-in.])
fi

## ----------------
## GnuTLS
AC_ARG_ENABLE(
	[gnutls],
	AC_HELP_STRING([--enable-gnutls], [Enable GnuTLS support.]),
	[enable_gnutls="$enableval"],
	[enable_gnutls="no"]
)
if test "x$enable_gnutls" = "xyes"; then
  PKG_CHECK_MODULES(GNUTLS, gnutls)
  AC_DEFINE(HAVE_GNUTLS, 1, [Define to 1 if gnutls support is required.])
  # for ArchLinux
  AC_SUBST(GNUTLS_CFLAGS)
  AC_SUBST(GNUTLS_LIBS)
fi

## -----------------
## aria2 plug-in
AC_ARG_ENABLE(
	[plugin-aria2],
	AC_HELP_STRING([--disable-plugin-aria2], [Disable aria2 plug-in.]),
	[plugin_aria2="$enableval"],
	[plugin_aria2="yes"]
)
if test "x$plugin_aria2" = "xyes"; then
    AC_DEFINE(HAVE_PLUGIN_ARIA2, 1, [Define to 1 to enable aria2 plug-in.])
fi

## -----------------
## libpwmd
AC_ARG_ENABLE(
	[pwmd],
	AC_HELP_STRING([--enable-pwmd], [Enable pwmd support.]),
	[enable_pwmd="$enableval"],
	[enable_pwmd="no"]
)
if test "x$enable_pwmd" = "xyes"; then
  PKG_CHECK_MODULES(LIBPWMD, [libpwmd-7.0 >= 7.0.0])
  AC_DEFINE(HAVE_LIBPWMD, 1, [Define to 1 if libpwmd support is required.])
fi
AM_CONDITIONAL([WITH_LIBPWMD],  [test "x$enable_pwmd" = "xyes"])

## -----------------
## HIDE_TEMPORARY_FILE
AC_ARG_ENABLE(
	[hidden],
	AC_HELP_STRING([--enable-hidden], [Make temporary files hidden.]),
	[hide_tempfiles="$enableval"],
	[hide_tempfiles="no"]
)
if test "x$hide_tempfiles" = "xyes"; then
    AC_DEFINE(HIDE_TEMPORARY_FILE, 1, [Define to 1 to make temporary download files hidden.])
fi

## ----------------------------------------------
## output
AC_CONFIG_FILES([
  Makefile
  uglib/Makefile
  uget-gtk/Makefile
  uget-cmd/Makefile
  tests/Makefile
  pixmaps/Makefile
  sounds/Makefile
  po/Makefile.in
  Windows/Makefile
])

AC_OUTPUT

