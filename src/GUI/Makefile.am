# Copyright (C) 2009-2012  CEA/DEN, EDF R&D
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
salomeinclude_HEADERS = 

# Libraries targets
lib_LTLIBRARIES = libHEXABLOCKPluginGUI.la

dist_libHEXABLOCKPluginGUI_la_SOURCES = HEXABLOCKPluginGUI_HypothesisCreator.cxx 

MOC_FILES = HEXABLOCKPluginGUI_HypothesisCreator_moc.cxx
nodist_libHEXABLOCKPluginGUI_la_SOURCES= $(MOC_FILES)

libHEXABLOCKPluginGUI_la_CPPFLAGS =  \
	$(QT_INCLUDES)        \
	$(KERNEL_CXXFLAGS)    \
	$(GUI_CXXFLAGS)       \
	$(GEOM_CXXFLAGS)      \
	$(HEXABLOCK_CXXFLAGS) \
	$(MED_CXXFLAGS)       \
	$(SMESH_CXXFLAGS)     \
	$(BOOST_CPPFLAGS)     \
	$(CORBA_CXXFLAGS)     \
	$(CORBA_INCLUDES)     \
	$(CAS_CPPFLAGS)       \
        -I$(srcdir)/..        \
	-I$(top_builddir)/idl

libHEXABLOCKPluginGUI_la_LDFLAGS  = \
	../../idl/libSalomeIDLHEXABLOCKPLUGIN.la \
	$(QT_LIBS) \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace \
	$(HEXABLOCK_LDFLAGS) -lSalomeIDLHEXABLOCK \
	$(MED_LDFLAGS) -lSalomeIDLMED \
	$(SMESH_LDFLAGS) -lSMESH \
	$(GUI_LDFLAGS) -lsuit -lqtx -lSalomeApp \
	$(CAS_KERNEL)

# resources files
nodist_salomeres_DATA= \
	HEXABLOCKPlugin_images.qm \
	HEXABLOCKPlugin_msg_en.qm
