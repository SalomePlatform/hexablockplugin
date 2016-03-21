// Copyright (C) 2009-2016  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//  File   : HEXABLOCKPlugin_HEXABLOCK_i.hxx
//  Author : Lioka RAZAFINDRAZAKA (CEA)
//  Module : HEXABLOCKPlugin
//  $Header$
//
#ifndef _HEXABLOCKPlugin_HEXABLOCK_I_HXX_
#define _HEXABLOCKPlugin_HEXABLOCK_I_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HEXABLOCKPlugin_Algorithm)
#include CORBA_SERVER_HEADER(SALOME_Exception)

#include "HEXABLOCKPlugin_Defs.hxx"
#include "SMESH_3D_Algo_i.hxx"
#include "HEXABLOCKPlugin_HEXABLOCK.hxx"

// ======================================================
// HEXABLOCK 3d algorithm
// ======================================================
class HEXABLOCKPLUGINENGINE_EXPORT HEXABLOCKPlugin_HEXABLOCK_i:
  public virtual POA_HEXABLOCKPlugin::HEXABLOCKPlugin_HEXABLOCK,
  public virtual SMESH_3D_Algo_i
{
public:
  // Constructor
  HEXABLOCKPlugin_HEXABLOCK_i (PortableServer::POA_ptr thePOA,
                       int                     theStudyId,
                       ::SMESH_Gen*            theGenImpl );
  // Destructor
  virtual ~HEXABLOCKPlugin_HEXABLOCK_i();
 
  // Get implementation
  ::HEXABLOCKPlugin_HEXABLOCK* GetImpl();
};

#endif
