//  Copyright (C) 2009-2011  CEA/DEN, EDF R&D
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

//=============================================================================
// File      : HEXABLOCKPlugin_Hypothesis.cxx
// Created   : Wed Apr  2 12:36:29 2008
// Author    : Lioka RAZAFINDRAZAKA (CEA)
//=============================================================================
//
#include "HEXABLOCKPlugin_Hypothesis.hxx"

//=======================================================================
//function : HEXABLOCKPlugin_Hypothesis
//=======================================================================

HEXABLOCKPlugin_Hypothesis::HEXABLOCKPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen),
  _document(NULL),
  _dimension(3)
{
  _name = "HEXABLOCK_Parameters";
  _param_algo_dim = 3;
}

//=======================================================================
//function : GetDocument
//=======================================================================

HEXA_NS::Document* HEXABLOCKPlugin_Hypothesis::GetDocument() const
{
  return(_document);
}

//=======================================================================
//function : SetDocument
//=======================================================================

void HEXABLOCKPlugin_Hypothesis::SetDocument(HEXA_NS::Document* doc)
{
  _document = doc;
}

//=======================================================================
//function : GetDimension
//=======================================================================

int HEXABLOCKPlugin_Hypothesis::GetDimension() const
{
  return(_dimension);
}

//=======================================================================
//function : SetDimension
//=======================================================================

void HEXABLOCKPlugin_Hypothesis::SetDimension(int dim)
{
  _dimension = dim;
}

//=======================================================================
//function : SaveTo
//=======================================================================

std::ostream & HEXABLOCKPlugin_Hypothesis::SaveTo(std::ostream & save)
{
//save << _document->getXML() << " ";
  save <<_dimension           << " ";

  return save;
}

//=======================================================================
//function : LoadFrom
//=======================================================================

std::istream & HEXABLOCKPlugin_Hypothesis::LoadFrom(std::istream & load)
{
    bool isOK = true;
    int i;

//     char* str;
//     isOK = (load >> str);
//     if (isOK)
//         _document = xml_2_doc(str);
//     else
//         load.clear(ios::badbit | load.rdstate());
    
    isOK = (load >> i);
    if (isOK)
        _dimension = i;
    else
        load.clear(ios::badbit | load.rdstate());
    
  return load;
}

//=======================================================================
//function : SetParametersByMesh
//=======================================================================

bool HEXABLOCKPlugin_Hypothesis::SetParametersByMesh(const SMESH_Mesh* ,const TopoDS_Shape&)
{
  return false;
}


//================================================================================
/*!
 * \brief Return false
 */
//================================================================================

bool HEXABLOCKPlugin_Hypothesis::SetParametersByDefaults(const TDefaults&  /*dflts*/,
                                                     const SMESH_Mesh* /*theMesh*/)
{
  return false;
}
