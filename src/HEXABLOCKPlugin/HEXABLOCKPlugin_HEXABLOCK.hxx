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
// File      : HEXABLOCKPlugin_HEXABLOCK.hxx
// Author    : Lioka RAZAFINDRAZAKA (CEA)
// Project   : SALOME
// $Header$
//=============================================================================
//
#ifndef _HEXABLOCKPlugin_HEXABLOCK_HXX_
#define _HEXABLOCKPlugin_HEXABLOCK_HXX_

#include "HEXABLOCKPlugin_Defs.hxx"
#include "SMESH_Algo.hxx"
#include "SMESH_Mesh.hxx"

class SMESH_Mesh;
class HEXABLOCKPlugin_Hypothesis;

class HEXABLOCKPLUGINENGINE_EXPORT HEXABLOCKPlugin_HEXABLOCK: public SMESH_3D_Algo
{
public:
  HEXABLOCKPlugin_HEXABLOCK(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~HEXABLOCKPlugin_HEXABLOCK();

  virtual bool CheckHypothesis(SMESH_Mesh&         aMesh,
                               const TopoDS_Shape& aShape,
                               Hypothesis_Status&  aStatus);

  virtual bool Compute(SMESH_Mesh&         aMesh,
                       const TopoDS_Shape& aShape);

  virtual bool Evaluate(SMESH_Mesh& aMesh, const TopoDS_Shape& aShape,
                        MapShapeNbElems& aResMap);

  virtual bool Compute(SMESH_Mesh&         theMesh,
                       SMESH_MesherHelper* aHelper);

  bool Compute3D(SMESH_Mesh& aMesh);
  bool Compute2D(SMESH_Mesh& aMesh);
  bool Compute1D(SMESH_Mesh& aMesh);
  bool Compute0D(SMESH_Mesh& aMesh);

private:
  const HEXABLOCKPlugin_Hypothesis* _hyp;
  int  _iShape;
  int  _nbShape;
};

#endif
