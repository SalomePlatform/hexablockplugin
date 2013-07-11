# Copyright (C) 2009-2013  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# -* Makefile *- 
# Author : Lioka RAZAFINDRAZAKA (CEA)
# Modified by : Alexander BORODIN (OCN) - autotools usage
# Module : HEXABLOCKPLUGIN
# Date : 2010/11/08
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
       HEXABLOCKPlugin_Defs.hxx \
       HEXABLOCKPlugin_HEXABLOCK.hxx \
       HEXABLOCKPlugin_HEXABLOCK_i.hxx \
       HEXABLOCKPlugin_Hypothesis.hxx \
       HEXABLOCKPlugin_Hypothesis_i.hxx \
       HEXABLOCKPlugin_mesh.hxx \
       HEXABLOCKPlugin_FromSkin_3D.hxx

# Libraries targets
lib_LTLIBRARIES = libHexaBlockEngine.la

dist_libHexaBlockEngine_la_SOURCES = \
	HEXABLOCKPlugin_HEXABLOCK.cxx \
	HEXABLOCKPlugin_HEXABLOCK_i.cxx \
	HEXABLOCKPlugin_i.cxx \
	HEXABLOCKPlugin_Hypothesis.cxx \
	HEXABLOCKPlugin_Hypothesis_i.cxx \
        HEXABLOCKPlugin_mesh.cxx \
        HEXABLOCKPlugin_FromSkin_3D.cxx

libHexaBlockEngine_la_CPPFLAGS =  \
	$(KERNEL_CXXFLAGS)    \
	$(CAS_CPPFLAGS)       \
        $(VTK_INCLUDES)       \
	$(GEOM_CXXFLAGS)      \
	$(HEXABLOCK_CXXFLAGS) \
	$(SMESH_CXXFLAGS)     \
	$(BOOST_CPPFLAGS)     \
	$(CORBA_CXXFLAGS)     \
	$(CORBA_INCLUDES)     \
	-I$(top_builddir)/idl

libHexaBlockEngine_la_LDFLAGS  = \
	../../idl/libSalomeIDLHEXABLOCKPLUGIN.la \
	$(CAS_KERNEL) -lTKBRep -lTKG2d -lTKG3d -lTKTopAlgo -lTKGeomBase -lTKGeomAlgo \
	$(HEXABLOCK_LDFLAGS) -lHEXABLOCKEngine \
	$(SMESH_LDFLAGS) -lSMESHimpl -lSMESHEngine -lSMESHDS -lSMDS \
	$(KERNEL_LDFLAGS) -lSalomeGenericObj -lSALOMELocalTrace -lSALOMEBasics
