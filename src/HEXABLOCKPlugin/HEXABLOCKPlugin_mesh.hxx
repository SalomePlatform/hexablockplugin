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
//  File   : SMESH_HexaBlocks.hxx
//  Author :
//  Module : SMESH
//
#ifndef _SMESH_HexaBlocks_HeaderFile
#define _SMESH_HexaBlocks_HeaderFile

#include <SALOMEconfig.h>
#include "HEXABLOCKPlugin_Defs.hxx"

#include "SMESHDS_Mesh.hxx"
#include "SMESH_Group.hxx"
#include "SMESH_Mesh.hxx"
#include "hexa_base.hxx" // from HexaBlocks
#include "HexFaceShape.hxx" // from HexaBlocks

#ifndef _Standard_Real_HeaderFile
#include <Standard_Real.hxx>
#endif

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>
#include <BRepAdaptor_Curve.hxx>

//=====================================================================
// SMESH_HexaBlocks : class definition
//=====================================================================
class HEXABLOCKPLUGINENGINE_EXPORT SMESH_HexaBlocks
{
public:

//   typedef std::vector<double>            Xx;
  typedef std::vector<SMDS_MeshVolume*>  SMESHVolumes;
  typedef std::vector<SMDS_MeshNode*>    SMESHNodes;
  typedef std::vector<SMDS_MeshFace*>    SMESHFaces;
  typedef std::vector<SMDS_MeshEdge*>    SMESHEdges;

  typedef std::vector< SMESHNodes >      ArrayOfSMESHNodes;

  struct Coord{
    double x;
    double y;
    double z;
  };


   SMESH_HexaBlocks(SMESH_Mesh& theMesh);
  ~SMESH_HexaBlocks();

  // --------------------------------------------------------------
  //  Vertex computing
  // --------------------------------------------------------------
  bool computeVertex( HEXA_NS::Vertex& vertex );

  // --------------------------------------------------------------
  //  Edge computing
  // --------------------------------------------------------------
  bool computeEdge( HEXA_NS::Edge& edge, HEXA_NS::Law& law);
  // Association solving
  bool computeEdgeByAssoc( HEXA_NS::Edge& edge, HEXA_NS::Law& law);

  // Automatic solving
  bool computeEdgeByShortestWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law);
  bool computeEdgeByPlanWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law);
  bool computeEdgeByIsoWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law);
  bool computeEdgeBySegment( HEXA_NS::Edge& edge, HEXA_NS::Law& law);

  // --------------------------------------------------------------
  //  Quad computing
  // --------------------------------------------------------------
  std::map<HEXA_NS::Quad*, bool>  computeQuadWays( HEXA_NS::Document* doc );
  bool computeQuad( HEXA_NS::Quad& quad, bool way );
  // Association solving
  bool computeQuadByAssoc( HEXA_NS::Quad& quad, bool way );
  // Automatic solving
  bool computeQuadByFindingGeom( HEXA_NS::Quad& quad, bool way );
  bool computeQuadByLinearApproximation( HEXA_NS::Quad& quad, bool way );

  // --------------------------------------------------------------
  //  Hexa computing
  // --------------------------------------------------------------
  bool computeHexa( HEXA_NS::Document* doc );

  // --------------------------------------------------------------
  //  Document computing: Vertex, Edge, Quad and Hexa computing
  // --------------------------------------------------------------
  bool computeDoc( HEXA_NS::Document* doc );


  // --------------------------------------------------------------
  //  Build groups
  // --------------------------------------------------------------
  void buildGroups(HEXA_NS::Document* doc);


private:
  //    ********     METHOD FOR MESH COMPUTATION    ********
  //  EDGE
  double _Xx( double i, HEXA_NS::Law law, double nbNodes );

  double _edgeLength(const TopoDS_Edge & E);

  void _buildMyCurve(
      const gp_Pnt&                             myCurve_start,  //IN
      const gp_Pnt&				myCurve_end,    //IN
      std::list< BRepAdaptor_Curve* >& 	        myCurve,        //INOUT
      double& 				        myCurve_length, //INOUT
      std::map< BRepAdaptor_Curve*, double>& 	myCurve_lengths,//INOUT
      std::map< BRepAdaptor_Curve*, bool>& 	myCurve_ways,   //INOUT
      std::map< BRepAdaptor_Curve*, double>&    myCurve_starts,   //INOUT
      HEXA_NS::Edge&                            edge);  // For diagnostic

  gp_Pnt _getPtOnMyCurve(
      const double&                           myCurve_u,      //IN
      std::map< BRepAdaptor_Curve*, bool>&    myCurve_ways,   //IN
      std::map< BRepAdaptor_Curve*, double>&  myCurve_lengths,//IN
      std::map< BRepAdaptor_Curve*, double>&  myCurve_starts, //IN
      std::list< BRepAdaptor_Curve* >&        myCurve,        //INOUT
      double&                                 myCurve_start); //INOUT

  // QUAD
  void _nodeInterpolationUV( double u, double v,
    SMDS_MeshNode* Pg, SMDS_MeshNode* Pd, SMDS_MeshNode* Ph, SMDS_MeshNode* Pb,
    SMDS_MeshNode* S0, SMDS_MeshNode* S1, SMDS_MeshNode* S2, SMDS_MeshNode* S3,
    double& xOut, double& yOut, double& zOut );

  // TopoDS_Shape _getShapeOrCompound( const std::vector<HEXA_NS::Shape*>& shapesIn );
  TopoDS_Shape getFaceShapes (Hex::Quad& quad);

  gp_Pnt _intersect( const gp_Pnt& Pt,
                     const gp_Vec& u, const gp_Vec& v,
                     const TopoDS_Shape& s,
                     Standard_Real tol = 0.0001 );

  bool _computeQuadInit(
    HEXA_NS::Quad& quad,
    ArrayOfSMESHNodes& nodesOnQuad,
    std::vector<double>& xx, std::vector<double>& yy);

  void _searchInitialQuadWay( HEXA_NS::Quad* quad, //IN
                              HEXA_NS::Vertex*& v0,      //INOUT
                              HEXA_NS::Vertex*& v1 );    //INOUT


  //    ********     DATA FOR MESH COMPUTATION    ********
  //    NODES
  std::map<HEXA_NS::Vertex*, SMDS_MeshNode*>        _node;    //_vertexNode;
  std::map<const SMDS_MeshNode*, HEXA_NS::Vertex*>  _vertex;  //_nodeVertex;

  //    EDGES
  std::map<HEXA_NS::Edge*, SMESHNodes>  _nodesOnEdge; //_edgeNodes;
//   std::map<HEXA_NS::Edge*, Xx>                _edgeXx;
  std::map<SMDS_MeshNode*, double>  _nodeXx; //_edgeNodes;
  std::map<HEXA_NS::Quad*, ArrayOfSMESHNodes> _quadNodes;

  bool _computeVertexOK;
  bool _computeEdgeOK;
  bool _computeQuadOK;

  SMESHDS_Mesh* _theMeshDS;
  SMESH_Mesh*   _theMesh;

  //    ********     METHOD FOR GROUPS COMPUTATION    ********
  SMESH_Group* _createGroup(HEXA_NS::Group& grHex);
  void _fillGroup(HEXA_NS::Group* grHex);

  //    ********     DATA FOR GROUPS COMPUTATION    ********
  std::map<HEXA_NS::Hexa*, SMESHVolumes> _volumesOnHexa;
  std::map<HEXA_NS::Quad*, SMESHFaces>   _facesOnQuad;
  std::map<HEXA_NS::Edge*, SMESHEdges>   _edgesOnEdge;


  // for DEBUG purpose only:
  int _total;
  int _found;
  int _notFound;
};


#endif
