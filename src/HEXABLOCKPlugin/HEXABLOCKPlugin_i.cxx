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

//  SMESH HEXABLOCKPlugin : implementaion of SMESH idl descriptions
//  File   : HEXABLOCKPlugin.cxx
//  Author : Lioka RAZAFINDRAZAKA (CEA)
//  Module : SMESH
//
#include "SMESH_Hypothesis_i.hxx"

#include "utilities.h"

#include "HEXABLOCKPlugin_HEXABLOCK_i.hxx"
#include "HEXABLOCKPlugin_Hypothesis_i.hxx"

#include "hexa_base.hxx"

#ifdef _DEBUG_
static int MYDEBUG = HEXA_NS::on_debug ();
#else
static int MYDEBUG = 0;
#endif


using namespace std;

template <class T> class HEXABLOCKPlugin_Creator_i:public HypothesisCreator_i<T>
{
  // as we have 'module HEXABLOCKPlugin' in HEXABLOCKPlugin_Algorithm.idl
  virtual std::string GetModuleName() { return "HEXABLOCKPlugin"; }
};

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{
  HEXABLOCKPLUGINENGINE_EXPORT
  GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    if(MYDEBUG) MESSAGE("GetHypothesisCreator " << aHypName);

    GenericHypothesisCreator_i* aCreator = 0;

    // Hypotheses

    // Algorithm
    if (strcmp(aHypName, "HEXABLOCK_3D") == 0)
      aCreator = new HEXABLOCKPlugin_Creator_i<HEXABLOCKPlugin_HEXABLOCK_i>;
    // Hypothesis
    else if (strcmp(aHypName, "HEXABLOCK_Parameters") == 0)
      aCreator = new HEXABLOCKPlugin_Creator_i<HEXABLOCKPlugin_Hypothesis_i>;
    else ;

    return aCreator;
  }
}
