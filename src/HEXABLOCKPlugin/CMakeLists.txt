# Copyright (C) 2012-2016  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${HEXABLOCK_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(  
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKBRep}
  ${CAS_TKG2d}
  ${CAS_TKG3d}
  ${CAS_TKTopAlgo}
  ${CAS_TKGeomBase}
  ${CAS_TKGeomAlgo}
  ${HEXABLOCK_HEXABLOCKEngine}
  ${SMESH_SMESHimpl}
  ${SMESH_SMESHEngine}
  ${SMESH_SMESHDS}
  ${SMESH_SMDS}
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  SalomeIDLHEXABLOCKPLUGIN
)

# --- headers ---

# header files
SET(HexaBlockPluginEngine_HEADERS
  HEXABLOCKPlugin_Defs.hxx
  HEXABLOCKPlugin_HEXABLOCK.hxx
  HEXABLOCKPlugin_HEXABLOCK_i.hxx
  HEXABLOCKPlugin_Hypothesis.hxx
  HEXABLOCKPlugin_Hypothesis_i.hxx
  HEXABLOCKPlugin_mesh.hxx
  HEXABLOCKPlugin_FromSkin_3D.hxx
)

# --- sources ---

# sources / static
SET(HexaBlockPluginEngine_SOURCES
  HEXABLOCKPlugin_HEXABLOCK.cxx
  HEXABLOCKPlugin_HEXABLOCK_i.cxx
  HEXABLOCKPlugin_i.cxx
  HEXABLOCKPlugin_Hypothesis.cxx
  HEXABLOCKPlugin_Hypothesis_i.cxx
  HEXABLOCKPlugin_mesh.cxx
  HEXABLOCKPlugin_FromSkin_3D.cxx
)

# --- rules ---

ADD_LIBRARY(HexaBlockPluginEngine ${HexaBlockPluginEngine_SOURCES})
TARGET_LINK_LIBRARIES(HexaBlockPluginEngine ${_link_LIBRARIES} )
INSTALL(TARGETS HexaBlockPluginEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${HexaBlockPluginEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
