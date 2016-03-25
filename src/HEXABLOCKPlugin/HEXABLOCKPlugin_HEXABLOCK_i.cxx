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

//  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
//  File   : HEXABLOCKPlugin_HEXABLOCK_i.cxx
//  Author : Lioka RAZAFINDRAZAKA (CEA)
//  Module : HEXABLOCKPlugin
//  $Header$
//
#include "HEXABLOCKPlugin_HEXABLOCK_i.hxx"
#include "SMESH_Gen.hxx"
#include "HEXABLOCKPlugin_HEXABLOCK.hxx"

#include "utilities.h"
#include "hexa_base.hxx"

#ifdef _DEBUG_
static int MYDEBUG = HEXA_NS::on_debug ();
#else
static int MYDEBUG = 0;
#endif

using namespace std;

//=============================================================================
/*!
 *  HEXABLOCKPlugin_HEXABLOCK_i::HEXABLOCKPlugin_HEXABLOCK_i
 *
 *  Constructor
 */
//=============================================================================

HEXABLOCKPlugin_HEXABLOCK_i::HEXABLOCKPlugin_HEXABLOCK_i (PortableServer::POA_ptr thePOA,
                                          int                     theStudyId,
                                          ::SMESH_Gen*            theGenImpl )
     : SALOME::GenericObj_i( thePOA ), 
       SMESH_Hypothesis_i( thePOA ), 
       SMESH_Algo_i( thePOA ),
       SMESH_3D_Algo_i( thePOA )
{
  if(MYDEBUG) MESSAGE( "HEXABLOCKPlugin_HEXABLOCK_i::HEXABLOCKPlugin_HEXABLOCK_i" );
  myBaseImpl = new ::HEXABLOCKPlugin_HEXABLOCK (theGenImpl->GetANewId(),
                                        theStudyId,
                                        theGenImpl );
}

//=============================================================================
/*!
 *  HEXABLOCKPlugin_HEXABLOCK_i::~HEXABLOCKPlugin_HEXABLOCK_i
 *
 *  Destructor
 */
//=============================================================================

HEXABLOCKPlugin_HEXABLOCK_i::~HEXABLOCKPlugin_HEXABLOCK_i()
{
  if(MYDEBUG) MESSAGE( "HEXABLOCKPlugin_HEXABLOCK_i::~HEXABLOCKPlugin_HEXABLOCK_i" );
}

//=============================================================================
/*!
 *  HEXABLOCKPlugin_HEXABLOCK_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================

::HEXABLOCKPlugin_HEXABLOCK* HEXABLOCKPlugin_HEXABLOCK_i::GetImpl()
{
  if(MYDEBUG) MESSAGE( "HEXABLOCKPlugin_HEXABLOCK_i::GetImpl" );
  return ( ::HEXABLOCKPlugin_HEXABLOCK* )myBaseImpl;
}

