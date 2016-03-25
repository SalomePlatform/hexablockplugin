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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${HEXABLOCK_INCLUDE_DIRS}
  ${SMESH_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${CAS_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${QT_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${QT_LIBRARIES}
  ${GUI_SalomeApp}
  ${GUI_qtx}
  ${GUI_suit}
  ${SMESH_SMESH}
  ${KERNEL_SALOMELocalTrace}
  ${CAS_KERNEL}
  SalomeIDLHEXABLOCKPLUGIN
)

# --- headers ---

# header files / to be processed by moc
SET(HEXABLOCKPluginGUI_HEADERS
  HEXABLOCKPluginGUI_HypothesisCreator.h
)

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${HEXABLOCKPluginGUI_HEADERS})

# sources / static
SET(_other_SOURCES
  HEXABLOCKPluginGUI_HypothesisCreator.cxx
)

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  HEXABLOCKPlugin_images.ts
  HEXABLOCKPlugin_msg_en.ts
  HEXABLOCKPlugin_msg_ja.ts
) 

# sources / to compile
SET(HEXABLOCKPluginGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- rules ---

ADD_LIBRARY(HEXABLOCKPluginGUI ${HEXABLOCKPluginGUI_SOURCES})
TARGET_LINK_LIBRARIES(HEXABLOCKPluginGUI ${_link_LIBRARIES} )
INSTALL(TARGETS HEXABLOCKPluginGUI EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_HEXABLOCKPLUGIN_INSTALL_RES_DATA}")
