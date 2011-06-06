#  Copyright (C) 2009-2011  CEA/DEN, EDF R&D
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# -* Makefile *- 
# Author : Lioka RAZAFINDRAZAKA (CEA)
# Date : 2010/11/08
#  Modified by : Alexander BORODIN (OCN) - autotools usage
# $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

dist_salomeres_DATA =		\
	HEXABLOCKPlugin.xml		\
	SalomeApp.xml

if HEXABLOCKPLUGIN_ENABLE_GUI
  dist_salomeres_DATA +=	\
	mesh_hypo_HEXABLOCK.png	\
	mesh_tree_hypo_HEXABLOCK.png
endif
