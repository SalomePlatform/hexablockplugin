dnl Copyright (C) 2009-2016  CEA/DEN, EDF R&D
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl

#  Check availability of HEXABLOCKPLUGIN binary distribution
#
#  Author : Lioka RAZAFINDRAZAKA (CEA)
#------------------------------------------------------------

AC_DEFUN([CHECK_HEXABLOCKPLUGIN],[

HEXABLOCKPLUGIN_LDFLAGS=""
HEXABLOCKPLUGIN_CXXFLAGS=""

AC_CHECKING(for GHS3dPlugin)

HEXABLOCKPLUGIN_ok=no

AC_ARG_WITH(ghs,
	    --with-HEXABLOCKPlugin=DIR  root directory path of HEXABLOCKPLUGIN build or installation,
	    HEXABLOCKPLUGIN_DIR="$withval",HEXABLOCKPLUGIN_DIR="")

if test "x$HEXABLOCKPLUGIN_DIR" = "x" ; then

# no --with-gui-dir option used

  if test "x$HEXABLOCKPLUGIN_ROOT_DIR" != "x" ; then

    # SALOME_ROOT_DIR environment variable defined
    HEXABLOCKPLUGIN_DIR=$HEXABLOCKPLUGIN_ROOT_DIR

  else

    # search Salome binaries in PATH variable
    AC_PATH_PROG(TEMP, libHexaBlockPluginEngine.so)
    if test "x$TEMP" != "x" ; then
      HEXABLOCKPLUGIN_DIR=`dirname $TEMP`
    fi

  fi

fi

if test -f ${HEXABLOCKPLUGIN_DIR}/lib/salome/libHexaBlockPluginEngine.so  ; then
  HEXABLOCKPLUGIN_ok=yes
  AC_MSG_RESULT(Using HEXABLOCKPLUGIN module distribution in ${HEXABLOCKPLUGIN_DIR})

  if test "x$HEXABLOCKPLUGIN_ROOT_DIR" == "x" ; then
    HEXABLOCKPLUGIN_ROOT_DIR=${HEXABLOCKPLUGIN_DIR}
  fi
  HEXABLOCKPLUGIN_CXXFLAGS+=-I${HEXABLOCKPLUGIN_ROOT_DIR}/include/salome
  HEXABLOCKPLUGIN_LDFLAGS+=-L${HEXABLOCKPLUGIN_ROOT_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
  AC_SUBST(HEXABLOCKPLUGIN_ROOT_DIR)
  AC_SUBST(HEXABLOCKPLUGIN_LDFLAGS)
  AC_SUBST(HEXABLOCKPLUGIN_CXXFLAGS)
else
  AC_MSG_WARN("Cannot find compiled HEXABLOCKPLUGIN module distribution")
fi
  
AC_MSG_RESULT(for HEXABLOCKPLUGIN: $HEXABLOCKPLUGIN_ok)
 
])dnl
 
