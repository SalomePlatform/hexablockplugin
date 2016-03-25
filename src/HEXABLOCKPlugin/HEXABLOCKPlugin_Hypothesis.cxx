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

//=============================================================================
// File      : HEXABLOCKPlugin_Hypothesis.cxx
// Created   : Wed Apr  2 12:36:29 2008
// Author    : Lioka RAZAFINDRAZAKA (CEA)
//=============================================================================
//
#include "HEXABLOCKPlugin_Hypothesis.hxx"
#include "Hex.hxx"

//=======================================================================
//function : HEXABLOCKPlugin_Hypothesis
//=======================================================================

HEXABLOCKPlugin_Hypothesis::HEXABLOCKPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen * gen)
  : SMESH_Hypothesis(hypId, studyId, gen)
{
   hexa_root     = HEXA_NS::Hex::getInstance ();
   hyp_document  = NULL;
   hyp_dimension = 3;

   // PutData (hexa_root->countDocument ());

   _name = "HEXABLOCK_Parameters";
   _param_algo_dim = 3;
}

//=======================================================================
//function : GetDocument
//=======================================================================

HEXA_NS::Document* HEXABLOCKPlugin_Hypothesis::GetDocument() const
{
  return hyp_document;
}

//=======================================================================
//function : SetXmlFlow
//=======================================================================
void HEXABLOCKPlugin_Hypothesis::SetXmlFlow (cpchar xml)
{
   if (hyp_document ==NULL)
       hyp_document  = hexa_root->addDocument ("tobe_meshed");
   hyp_document->setXml (xml); 
}

//=======================================================================
//function : GetXmlFlow
//=======================================================================
cpchar HEXABLOCKPlugin_Hypothesis::GetXmlFlow () const
{
   return (hyp_document == NULL) ? NULL : hyp_document->getXml();
}

//=======================================================================
//function : SetDocument
//=======================================================================
void HEXABLOCKPlugin_Hypothesis::SetDocument (cpchar name)
{
   hyp_document = hexa_root->findDocument (name);
}

//=======================================================================
//function : GetDimension
//=======================================================================

int HEXABLOCKPlugin_Hypothesis::GetDimension() const
{
   return hyp_dimension;
}

//=======================================================================
//function : SetDimension
//=======================================================================

void HEXABLOCKPlugin_Hypothesis::SetDimension(int dim)
{
   hyp_dimension = dim;
}

//=======================================================================
//function : SaveTo
//=======================================================================

std::ostream & HEXABLOCKPlugin_Hypothesis::SaveTo(std::ostream & save)
{
//save << hyp_document->getXML() << " ";
  save << hyp_dimension           << " ";

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
//         hyp_document = xml_2_doc(str);
//     else
//         load.clear(ios::badbit | load.rdstate());
    
    isOK = static_cast<bool>(load >> i);
    if (isOK)
        hyp_dimension = i;
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
