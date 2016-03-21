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

// File      : HEXABLOCKPlugin_Hypothesis_i.cxx
// Created   : Wed Apr  2 13:53:01 2008
// Author    : Lioka RAZAFINDRAZAKA (CEA)
//
#include <SMESH_Gen.hxx>
#include <SMESH_PythonDump.hxx>
#include <SMESH_Mesh_i.hxx>

// #include "HEXABLOCK.hxx"
#include "HEXABLOCKPlugin_Hypothesis_i.hxx"
// #include "HexDocument_impl.hxx"    // Perime
#include "HexDocument.hxx"

#include <Utils_CorbaException.hxx>
#include <utilities.h>

#ifdef _DEBUG_
static int MYDEBUG = HEXA_NS::on_debug ();
#else
static int MYDEBUG = 0;
#endif

//=======================================================================
//function : HEXABLOCKPlugin_Hypothesis_i
//=======================================================================

HEXABLOCKPlugin_Hypothesis_i::HEXABLOCKPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                                  int             theStudyId,
                                  ::SMESH_Gen*    theGenImpl)
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  if(MYDEBUG) MESSAGE( "HEXABLOCKPlugin_Hypothesis_i::HEXABLOCKPlugin_Hypothesis_i" );
  myBaseImpl = new ::HEXABLOCKPlugin_Hypothesis (theGenImpl->GetANewId(),
                                              theStudyId,
                                              theGenImpl);
  _poa = PortableServer::POA::_duplicate(thePOA);
}

//=======================================================================
//function : ~HEXABLOCKPlugin_Hypothesis_i
//=======================================================================

HEXABLOCKPlugin_Hypothesis_i::~HEXABLOCKPlugin_Hypothesis_i()
{
  if(MYDEBUG) MESSAGE( "HEXABLOCKPlugin_Hypothesis_i::~HEXABLOCKPlugin_Hypothesis_i" );
}

//=============================================================================
/*!
 *  Get implementation
 */
//=============================================================================

::HEXABLOCKPlugin_Hypothesis* HEXABLOCKPlugin_Hypothesis_i::GetImpl()
{
  return (::HEXABLOCKPlugin_Hypothesis*)myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
 */
//================================================================================  

CORBA::Boolean HEXABLOCKPlugin_Hypothesis_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}

//================================================================================
/*!
 * Define the document to be meshed, mandatory
 */
//================================================================================
// ================================================================= GetDocument
char* HEXABLOCKPlugin_Hypothesis_i::GetDocument ()
{
  ASSERT (myBaseImpl);
  cpchar xml = this->GetImpl()->GetXmlFlow ();
  return CORBA::string_dup (xml);
}

// ================================================================= SetDocument
void HEXABLOCKPlugin_Hypothesis_i::SetDocument (const char* name)
{
    ASSERT (myBaseImpl);
    // this->GetImpl()->SetXmlFlow (xml);
    this->GetImpl()->SetDocument (name);
}

//================================================================================
/*!
 * To define the hight dimension to generated: 3 = hexas, 2 = quads, 1 = segments, 0 = nodes
 */
//================================================================================

CORBA::Long HEXABLOCKPlugin_Hypothesis_i::GetDimension() {
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetDimension();
}

void HEXABLOCKPlugin_Hypothesis_i::SetDimension(CORBA::Long dim) {
  ASSERT(myBaseImpl);
  this->GetImpl()->SetDimension(dim);
}
