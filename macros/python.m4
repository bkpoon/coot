## ------------------------                                 -*- Autoconf -*-
## Python file handling
## From Andrew Dalke
## Updated by James Henstridge
## ------------------------
# Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
# Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# AM_PATH_PYTHON([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------------------------------------------------------------------
# Adds support for distributing Python modules and packages.  To
# install modules, copy them to $(pythondir), using the python_PYTHON
# automake variable.  To install a package with the same name as the
# automake package, install to $(pkgpythondir), or use the
# pkgpython_PYTHON automake variable.
#
# The variables $(pyexecdir) and $(pkgpyexecdir) are provided as
# locations to install python extension modules (shared libraries).
# Another macro is required to find the appropriate flags to compile
# extension modules.
#
# If your package is configured with a different prefix to python,
# users will have to add the install directory to the PYTHONPATH
# environment variable, or create a .pth file (see the python
# documentation for details).
#
# If the MINIMUM-VERSION argument is passed, AM_PATH_PYTHON will
# cause an error if the version of python installed on the system
# doesn't meet the requirement.  MINIMUM-VERSION should consist of
# numbers and dots only.
AC_DEFUN([AM_PATH_PYTHON],
 [
  dnl Find a Python interpreter.  Python versions prior to 2.0 are not
  dnl supported. (2.0 was released on October 16, 2000).
  m4_define_default([_AM_PYTHON_INTERPRETER_LIST], [python3 python])

  m4_if([$1],[],[
    dnl No version check is needed.
    # Find any Python interpreter.
    if test -z "$PYTHON"; then
      AC_PATH_PROGS([PYTHON], _AM_PYTHON_INTERPRETER_LIST, :)
    fi
    am_display_PYTHON=python
  ], [
    dnl A version check is needed.
    if test -n "$PYTHON"; then
      # If the user set $PYTHON, use it and don't search something else.
      AC_MSG_CHECKING([whether $PYTHON version >= $1])
      AM_PYTHON_CHECK_VERSION([$PYTHON], [$1],
			      [AC_MSG_RESULT(yes)],
			      [AC_MSG_ERROR(too old)])
      am_display_PYTHON=$PYTHON
    else
      # Otherwise, try each interpreter until we find one that satisfies
      # VERSION.
      AC_CACHE_CHECK([for a Python interpreter with version >= $1],
	[am_cv_pathless_PYTHON],[
	for am_cv_pathless_PYTHON in _AM_PYTHON_INTERPRETER_LIST none; do
	  test "$am_cv_pathless_PYTHON" = none && break
	  AM_PYTHON_CHECK_VERSION([$am_cv_pathless_PYTHON], [$1], [break])
	done])
      # Set $PYTHON to the absolute path of $am_cv_pathless_PYTHON.
      if test "$am_cv_pathless_PYTHON" = none; then
	PYTHON=:
      else
        AC_PATH_PROG([PYTHON], [$am_cv_pathless_PYTHON])
      fi
      am_display_PYTHON=$am_cv_pathless_PYTHON
    fi
  ])

  if test "$PYTHON" = :; then
  dnl Run any user-specified action, or abort.
    m4_default([$3], [AC_MSG_ERROR([no suitable Python interpreter found])])
  else

  dnl Query Python for its version number.  Getting [:3] seems to be
  dnl the best way to do this; it's what "site.py" does in the standard
  dnl library.

  AC_CACHE_CHECK([for $am_display_PYTHON version], [am_cv_python_version],
    [am_cv_python_version=`$PYTHON -c "import sys; sys.stdout.write(str(sys.version_info.major) + '.' + str(sys.version_info.minor))"`])
  AC_SUBST([PYTHON_VERSION], [$am_cv_python_version])

  dnl Use the values of $prefix and $exec_prefix for the corresponding
  dnl values of PYTHON_PREFIX and PYTHON_EXEC_PREFIX.  These are made
  dnl distinct variables so they can be overridden if need be.  However,
  dnl general consensus is that you shouldn't need this ability.

  AC_SUBST([PYTHON_PREFIX], ['${prefix}'])
  AC_SUBST([PYTHON_EXEC_PREFIX], ['${exec_prefix}'])

  dnl BL says:: let them be defined by python in Windows (or in general?)
  dnl seems to be an odd way this...
  if test $have_windows_mingw = yes ; then
     PYTHON_PREFIX=`$PYTHON -c "import sys, os; from distutils import sysconfig; path=sysconfig.PREFIX.replace(':','').split(os.sep); sys.stdout.write('/'+'/'.join(path))"`
     PYTHON_EXEC_PREFIX=`$PYTHON -c "import sys, os; from distutils import sysconfig; path=sysconfig.EXEC_PREFIX.replace(':','').split(os.sep); sys.stdout.write('/'+'/'.join(path))"`
  fi

  dnl At times (like when building shared libraries) you may want
  dnl to know which OS platform Python thinks this is.

  AC_CACHE_CHECK([for $am_display_PYTHON platform], [am_cv_python_platform],
    [am_cv_python_platform=`$PYTHON -c "import sys; sys.stdout.write(sys.platform)"`])
  AC_SUBST([PYTHON_PLATFORM], [$am_cv_python_platform])


  dnl Set up 4 directories:

  dnl pythondir -- where to install python scripts.  This is the
  dnl   site-packages directory, not the python standard library
  dnl   directory like in previous automake betas.  This behavior
  dnl   is more consistent with lispdir.m4 for example.
  dnl Query distutils for this directory.  distutils does not exist in
  dnl Python 1.5, so we fall back to the hardcoded directory if it
  dnl doesn't work.
  AC_CACHE_CHECK([for $am_display_PYTHON script directory],
    [am_cv_python_pythondir],
    [if test "x$prefix" = xNONE
     then
       am_py_prefix=$ac_default_prefix
     else
       am_py_prefix=$prefix
     fi
     am_cv_python_pythondir=`$PYTHON -c "import sys; from distutils import sysconfig; sys.stdout.write(sysconfig.get_python_lib(0,0,prefix='$am_py_prefix'))" 2>/dev/null ||
     echo "$PYTHON_PREFIX/lib/python$PYTHON_VERSION/site-packages"`
     case $am_cv_python_pythondir in
     $am_py_prefix*)
       am__strip_prefix=`echo "$am_py_prefix" | sed 's|.|.|g'`
       am_cv_python_pythondir=`echo "$am_cv_python_pythondir" | sed "s,^$am__strip_prefix,$PYTHON_PREFIX,"`
       ;;
     *)
       case $am_py_prefix in
         /usr|/System*) ;;
         *)
	  am_cv_python_pythondir=$PYTHON_PREFIX/lib/python$PYTHON_VERSION/site-packages
	  ;;
       esac
       ;;
     esac
     if test $have_windows_mingw = yes ; then
       dnl BL says:: just let python find its way
       am_cv_python_pythondir=`$PYTHON -c "import os; from distutils import sysconfig; path=sysconfig.get_python_lib(0,0,prefix=sysconfig.PREFIX).replace(':','').split(os.sep); print('/'+'/'.join(path))"`
     fi
    ])
  AC_SUBST([pythondir], [$am_cv_python_pythondir])

  dnl pkgpythondir -- $PACKAGE directory under pythondir.  Was
  dnl   PYTHON_SITE_PACKAGE in previous betas, but this naming is
  dnl   more consistent with the rest of automake.

  AC_SUBST([pkgpythondir], [\${pythondir}/$PACKAGE])

  dnl pyexecdir -- directory for installing python extension modules
  dnl   (shared libraries)
  dnl Query distutils for this directory.  distutils does not exist in
  dnl Python 1.5, so we fall back to the hardcoded directory if it
  dnl doesn't work.
  AC_CACHE_CHECK([for $am_display_PYTHON extension module directory],
    [am_cv_python_pyexecdir],
    [if test "x$exec_prefix" = xNONE
     then
       am_py_exec_prefix=$am_py_prefix
     else
       am_py_exec_prefix=$exec_prefix
     fi
     am_cv_python_pyexecdir=`$PYTHON -c "import sys; from distutils import sysconfig; sys.stdout.write(sysconfig.get_python_lib(1,0,prefix='$am_py_exec_prefix'))" 2>/dev/null ||
     echo "$PYTHON_EXEC_PREFIX/lib/python$PYTHON_VERSION/site-packages"`
     case $am_cv_python_pyexecdir in
     $am_py_exec_prefix*)
       am__strip_prefix=`echo "$am_py_exec_prefix" | sed 's|.|.|g'`
       am_cv_python_pyexecdir=`echo "$am_cv_python_pyexecdir" | sed "s,^$am__strip_prefix,$PYTHON_EXEC_PREFIX,"`
       ;;
     *)
       case $am_py_exec_prefix in
         /usr|/System*) ;;
         *)
	   am_cv_python_pyexecdir=$PYTHON_EXEC_PREFIX/lib/python$PYTHON_VERSION/site-packages
	   ;;
       esac
       ;;
     esac
     if test $have_windows_mingw = yes ; then
       dnl BL says:: just let python find its way
       am_cv_python_pyexecdir=`$PYTHON -c "import os; from distutils import sysconfig; path=sysconfig.get_python_lib(0,0,prefix=sysconfig.EXEC_PREFIX).replace(':','').split(os.sep); print('/'+'/'.join(path))"`
     fi
    ])
  AC_SUBST([pyexecdir], [$am_cv_python_pyexecdir])

  dnl pkgpyexecdir -- $(pyexecdir)/$(PACKAGE)

  AC_SUBST([pkgpyexecdir], [\${pyexecdir}/$PACKAGE])

  dnl Run any user-specified action.
  $2
  fi

])


# AM_PYTHON_CHECK_VERSION(PROG, VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ---------------------------------------------------------------------------
# Run ACTION-IF-TRUE if the Python interpreter PROG has version >= VERSION.
# Run ACTION-IF-FALSE otherwise.
# This test uses sys.hexversion instead of the string equivalent (first
# word of sys.version), in order to cope with versions such as 2.2c1.
# This supports Python 2.0 or higher. (2.0 was released on October 16, 2000).
AC_DEFUN([AM_PYTHON_CHECK_VERSION],
 [prog="import sys
# split strings by '.' and convert to numeric.  Append some zeros
# because we need at least 4 digits for the hex conversion.
# map returns an iterator in Python 3.0 and a list in 2.x
minver = list(map(int, '$2'.split('.'))) + [[0, 0, 0]]
minverhex = 0
# xrange is not present in Python 3.0 and range returns an iterator
for i in list(range(0, 4)): minverhex = (minverhex << 8) + minver[[i]]
sys.exit(sys.hexversion < minverhex)"
  AS_IF([AM_RUN_LOG([$1 -c "$prog"])], [$3], [$4])])
