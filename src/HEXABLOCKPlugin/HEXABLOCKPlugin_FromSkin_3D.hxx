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

// File      : SMESH_HexaFromSkin_3D.hxx
// Created   : Wed Jan 27 12:23:21 2010
// Author    : Edward AGAPOV (eap)
//
#ifndef __SMESH_HexaFromSkin_3D_HXX__
#define __SMESH_HexaFromSkin_3D_HXX__

#include "HEXABLOCKPlugin_Defs.hxx"
#include "SMESH_StdMeshers.hxx"
#include "SMESH_Algo.hxx"

// from HexaBlocks
#include "hexa_base.hxx" 
#include "HexDocument.hxx"
#include "HexVertex.hxx"
// #include "HexEdge.hxx"
// #include "HexQuad.hxx"
#include "HexHexa.hxx"
#include "HEXABLOCKPlugin_mesh.hxx"

/*!
 * \brief Alorithm generating hexahedral mesh from 2D skin of block
 */

class HEXABLOCKPLUGINENGINE_EXPORT SMESH_HexaFromSkin_3D : public SMESH_3D_Algo
{
public:
//   SMESH_HexaFromSkin_3D(int hypId, int studyId, SMESH_Gen* gen);
  SMESH_HexaFromSkin_3D(int hypId, int studyId, SMESH_Gen* gen, HEXA_NS::Document* doc);
  virtual ~SMESH_HexaFromSkin_3D();

  virtual bool Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper);
  virtual bool Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper,
      std::map<HEXA_NS::Hexa*, SMESH_HexaBlocks::SMESHVolumes>& volumesOnHexa,
      std::map<HEXA_NS::Vertex*, SMDS_MeshNode*> vertexNode );

  virtual bool CheckHypothesis(SMESH_Mesh& aMesh,
                               const TopoDS_Shape& aShape,
                               Hypothesis_Status& aStatus);

  virtual bool Compute(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape);

  virtual bool Evaluate(SMESH_Mesh &         aMesh,
                        const TopoDS_Shape & aShape,
                        MapShapeNbElems&     aResMap);

private:
  HEXA_NS::Document*  _doc;

};

#endif
