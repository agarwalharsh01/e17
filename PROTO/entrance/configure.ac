AC_INIT([entrance], [0.0.4], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.59])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.6 dist-bzip2])
AM_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# m4_ifdef([AM_GNU_GETTEXT_VERSION], [
# AM_GNU_GETTEXT_VERSION([0.12.1])
# ])

# m4_ifdef([AM_GNU_GETTEXT], [
# AM_GNU_GETTEXT([external])
# po_makefile_in=po/Makefile.in
# AM_CONDITIONAL([HAVE_PO], [true])
# ],[
# AM_CONDITIONAL([HAVE_PO], [false])
# ])
# AC_SUBST(LTLIBINTL)


### Needed information

# Set the localedir var for gettext
AC_DEFINE([LOCALEDIR], ["/usr/share/locale/"], ["locales"])


### Additional options to configure

# Pam
AC_ARG_ENABLE([pam],
   [AC_HELP_STRING([--disable-pam], [disable pam support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_pam="yes"
    else
       enable_pam="no"
    fi
   ],
   [enable_pam="yes"])
AC_MSG_CHECKING([whether to support pam])
AC_MSG_RESULT([${enable_pam}])

# Grub2
AC_ARG_ENABLE([grub2],
   [AC_HELP_STRING([--enable-gru2b], [enable grub2 support. @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_grub2="yes"
    else
       enable_grub2="no"
    fi
   ],
   [enable_grub2="no"])
AC_MSG_CHECKING([whether to support grub2 reboot features])
AC_MSG_RESULT([${enable_grub2}])

if test "x${enable_grub2}" = "xyes" ; then
   AC_DEFINE([HAVE_GRUB2], [1], [Set to 1 if grub is enabled.])
fi

# consolekit
AC_ARG_ENABLE([consolekit],
   [AC_HELP_STRING([--disable-consolekit], [disable consolekit support. @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       enable_consolekit="yes"
    else
       enable_consolekit="no"
    fi
   ],
   [enable_consolekit="yes"])
AC_MSG_CHECKING([whether to support consolekit login])
AC_MSG_RESULT([${enable_consolekit}])


# Set edje_cc path
AC_ARG_WITH([edje-cc],
   [AC_HELP_STRING([ --with-edje-cc=PATH], [specify a specific path to edje_cc])],
   [
    v=$withval;
    edje_cc=$v
    echo "  Enlightenment edje_cc explicitly set to "$edje_cc;
   ],
   [edje_cc=$(pkg-config --variable=prefix edje)/bin/edje_cc])
AC_SUBST(edje_cc)

### Checks for programs
AC_PROG_CC

### Checks for compiler characteristics
AC_USE_SYSTEM_EXTENSIONS
AC_ISC_POSIX
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_C_CONST
AC_C___ATTRIBUTE__

### Checks for libraries

PKG_CHECK_MODULES([ENTRANCE], [efreet >= 1.0.0 eina >= 1.0.0 eet >= 1.4.0 ecore-con >= 1.0.0 ecore >= 1.0.0 ecore-file >= 1.0.0 xcb >= 1.0.0])
PKG_CHECK_MODULES([ENTRANCE_CLIENT], [eet >= 1.4.0 edje >= 1.0.0 elementary >= 0.5.0.0 ecore-evas >= 1.0.0 evas >= 1.0.0 ecore-con >= 1.0.0 ecore >= 1.0.0 ecore-x >= 1.0.0 eina >= 1.0.0])

# Consolekit
if test "x${enable_consolekit}" = "xyes" ; then
   PKG_CHECK_MODULES([ENTRANCE_CK], [dbus-1 ck-connector], [enable_consolekit="yes"], [enable_consolekit="no"])
fi

AM_CONDITIONAL(HAVE_CONSOLEKIT, [test "x${enable_consolekit}" = "xyes"])
if test "x${enable_consolekit}" = "xyes" ; then
   AC_DEFINE([HAVE_CONSOLEKIT], [1], [Set to 1 if consolekit is enabled.])
fi

# Pam
if test "x${enable_pam}" = "xyes" ; then
   AC_CHECK_LIB([pam], [pam_start],
      [have_pam="yes"],
      [have_pam="no"])
fi


### Checks for header files

# Pam
if test "x${have_pam}" = "xyes" ; then
   AC_CHECK_HEADER([security/pam_appl.h],
      [AC_DEFINE([HAVE_PAM], [1], [Set to 1 if pam is installed.])],
      [have_pam="no"])
else
   have_pam="no"
fi

AM_CONDITIONAL(HAVE_PAM, [test "x${have_pam}" = "xyes"])

### Checks for types


### Checks for structures
EFL_COMPILER_FLAG([-Wall])
EFL_COMPILER_FLAG([-W])
EFL_COMPILER_FLAG([-Wshadow])
EFL_COMPILER_FLAG([-Wmissing-declarations])


### Checks for linker characteristics


### Checks for library functions
AC_FUNC_MALLOC
AC_CHECK_FUNCS(strndup)


AC_OUTPUT([
Makefile
src/Makefile
src/bin/Makefile
src/daemon/Makefile
data/Makefile
data/themes/Makefile
data/themes/old/Makefile
data/themes/beaumonts/Makefile
])


#####################################################################
## Info

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  pam..................: ${have_pam}"
echo "  grub2................: ${enable_grub2}"
echo "  consolekit...........: ${enable_consolekit}"
echo
echo "  edje_cc..............: ${edje_cc}"
echo
echo "Compilation............: make (or gmake)"
echo "  CPPFLAGS.............: $CPPFLAGS"
echo "  CFLAGS...............: $CFLAGS"
echo "  LDFLAGS..............: $LDFLAGS"
echo
echo "Installation...........: make install (as root if needed, with 'su' or 'sudo')"
echo "  prefix...............: $prefix"
echo

