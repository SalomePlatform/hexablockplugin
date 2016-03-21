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
//  File   : SMESH_HexaBlocks.cxx
//  Author :
//  Module : SMESH
//

#include <sstream>
#include <algorithm>

// CasCade includes

#include <Precision.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <TopoDS_Wire.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>

// SMESH includes
#include "SMDS_MeshNode.hxx"
#include "SMDS_MeshVolume.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESHDS_Group.hxx"

// HEXABLOCK includes
#include "HexDocument.hxx"
#include "HexVertex.hxx"
#include "HexEdge.hxx"
#include "HexQuad.hxx"
#include "HexHexa.hxx"
#include "HexPropagation.hxx"
#include "HexGroup.hxx"
#include "hexa_base.hxx"
#include "HexAssoEdge.hxx"

// HEXABLOCKPLUGIN includes
#include "HEXABLOCKPlugin_mesh.hxx"
#include "HEXABLOCKPlugin_FromSkin_3D.hxx"

// other includes
#include "Basics_Utils.hxx"
#include "utilities.h"

#ifdef WNT
#include <process.h>
#else
#include <unistd.h>
#endif

#include <stdexcept>

#ifndef EXCEPTION
#define EXCEPTION(TYPE, MSG) {\
  std::ostringstream aStream;\
  aStream<<__FILE__<<"["<<__LINE__<<"]::"<<MSG;\
  throw TYPE(aStream.str());\
}
#endif

#ifdef _DEBUG_
static int  MYDEBUG = HEXA_NS::on_debug ();
#else
static int  MYDEBUG = 0;
#endif


static double HEXA_EPS      = 1.0e-3; //1E-3;
static double HEXA_QUAD_WAY = M_PI/4.; //3.*PI/8.;

// ============================================================ string2shape
TopoDS_Shape string2shape( const string& brep )
{
  TopoDS_Shape shape;
  istringstream streamBrep(brep);
  BRep_Builder aBuilder;
  BRepTools::Read(shape, streamBrep, aBuilder);
  return shape;
}

// ============================================================ shape2coord
bool shape2coord(const TopoDS_Shape& aShape, double& x, double& y, double& z)
{
   if ( aShape.ShapeType() == TopAbs_VERTEX ){
      TopoDS_Vertex aPoint;
       aPoint = TopoDS::Vertex( aShape );
       gp_Pnt aPnt = BRep_Tool::Pnt( aPoint );
       x = aPnt.X();
       y = aPnt.Y();
       z = aPnt.Z();
       return(1);
   } else {
       return(0);
   };
}
// ============================================================= edge_length
double edge_length(const TopoDS_Edge & E)
{
  double UMin = 0, UMax = 0;
  if (BRep_Tool::Degenerated(E))
    return 0;
  TopLoc_Location L;
  Handle(Geom_Curve) C = BRep_Tool::Curve(E, L, UMin, UMax);
  GeomAdaptor_Curve AdaptCurve(C);
  double length = GCPnts_AbscissaPoint::Length(AdaptCurve, UMin, UMax);
  return length;
}
// =============================================================== make_curve
BRepAdaptor_Curve* make_curve (gp_Pnt& pstart, gp_Pnt& pend,
                               double& length, double& u_start,
                               HEXA_NS::Edge& edge, int nro)
{
   Hex::AssoEdge*     asso       = edge.getAssociation (nro);
   BRepAdaptor_Curve* the_curve  = asso->getCurve();
   length    = asso->length ();
   if (length <= 1e-6)
       return NULL;

   const double* point1 = asso->getOrigin ();
   const double* point2 = asso->getExtrem ();
   pstart  = gp_Pnt (point1[0],  point1[1],  point1[2]);
   pend    = gp_Pnt (point2[0],  point2[1],  point2[2]);
   u_start = asso->getUstart();
   return the_curve;
}
// ============================================================== Constructeur
// SMESH_HexaBlocks::SMESH_HexaBlocks( SMESH_Mesh* theMesh ):
SMESH_HexaBlocks::SMESH_HexaBlocks(SMESH_Mesh& theMesh):
  _total(0),
  _found(0),
  _notFound(0),
  _computeVertexOK(false),
  _computeEdgeOK(false),
  _computeQuadOK(false),
  _theMesh(&theMesh),  //groups creation
  _theMeshDS(theMesh.GetMeshDS()) //meshing
{
}


SMESH_HexaBlocks::~SMESH_HexaBlocks()
{
}


// --------------------------------------------------------------
//                      PUBLIC METHODS
// --------------------------------------------------------------

// --------------------------------------------------------------
//                     Vertex computing
// --------------------------------------------------------------
//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
// ============================================================== computeVertex
bool SMESH_HexaBlocks::computeVertex(HEXA_NS::Vertex& vx)
{
  double           px, py, pz;
  vx.getAssoCoord (px, py, pz);

  SMDS_MeshNode* new_node = _theMeshDS->AddNode (px, py, pz);
  _vertex [new_node] = &vx;
  _node   [&vx]      = new_node;    //needed in computeEdge()
  _computeVertexOK   = true;
  return true;
}
// --------------------------------------------------------------
//                      Edge computing
// --------------------------------------------------------------
bool SMESH_HexaBlocks::computeEdge(HEXA_NS::Edge& edge, HEXA_NS::Law& law)
{
  bool ok = false;

  ok = computeEdgeByAssoc( edge, law);
  if ( ok == false ){
    ok = computeEdgeByShortestWire( edge, law);
  }
  if ( ok == false ){
    ok = computeEdgeByPlanWire( edge, law);
  }
  if ( ok == false ){
    ok = computeEdgeByIsoWire( edge, law);
  }
  if ( ok == false ){
    ok = computeEdgeBySegment( edge, law);
  }
  if (ok == true){
    _computeEdgeOK = true;
  }
  return ok;
}



bool SMESH_HexaBlocks::computeEdgeByAssoc( HEXA_NS::Edge& edge, HEXA_NS::Law& law )
{
  if(MYDEBUG) MESSAGE("computeEdgeByAssoc(edgeID = "<<edge.getId()<<"): begin   <<<<<<");
  ASSERT( _computeVertexOK );
  bool ok = true;

  if (NOT edge.isAssociated())
     return false;

  //vertex from edge
  HEXA_NS::Vertex* vx0 = NULL;
  HEXA_NS::Vertex* vx1 = NULL;

  // way of discretization
  if (edge.getWay() == true){
    vx0 = edge.getVertex(0);
    vx1 = edge.getVertex(1);
  } else {
    vx0 = edge.getVertex(1);
    vx1 = edge.getVertex(0);
  }
  // nodes on mesh
  SMDS_MeshNode* FIRST_NODE = _node[vx0];
  SMDS_MeshNode* LAST_NODE  = _node[vx1];


  // A) Build myCurve_list
  std::list< BRepAdaptor_Curve* >        myCurve_list;
  double                                 myCurve_tot_len;
  std::map< BRepAdaptor_Curve*, double>  myCurve_lengths;
  std::map< BRepAdaptor_Curve*, bool>    myCurve_ways;
  std::map< BRepAdaptor_Curve*, double>  myCurve_starts;

  gp_Pnt myCurve_pt_start(FIRST_NODE->X(), FIRST_NODE->Y(), FIRST_NODE->Z());
  gp_Pnt myCurve_pt_end  (LAST_NODE->X(),  LAST_NODE->Y(),  LAST_NODE->Z());

  _buildMyCurve(
      myCurve_pt_start,
      myCurve_pt_end,
      myCurve_list,
      myCurve_tot_len,
      myCurve_lengths,
      myCurve_ways,
      myCurve_starts,
      edge
  );


  // B) Build nodes and edges on mesh from myCurve_list
  SMDS_MeshNode* node_a  = NULL;
  SMDS_MeshNode* node_b  = NULL;
  SMDS_MeshEdge* edge_ab = NULL;
  SMESHNodes     nodesOnEdge;
  SMESHEdges     edgesOnEdge; //backup for group creation
//   Xx             nodesXxOnEdge;

  node_a = FIRST_NODE;
  nodesOnEdge.push_back(FIRST_NODE);
//   nodesXxOnEdge.push_back(0.);
  // _nodeXx[FIRST_NODE] = 0.;

  gp_Pnt ptOnMyCurve;
  double u, myCurve_u;
  double myCurve_start_u = 0.;
  int nbNodes = law.getNodes(); //law of discretization
  if (myCurve_list.size()==0)
     {
     PutData (edge.getName());
     nbNodes = 0;
     }
  if (MYDEBUG) MESSAGE("nbNodes -> "<<nbNodes);
  for (int i = 0; i < nbNodes; ++i){
      u = _Xx(i, law, nbNodes); //u between [0,1]
      myCurve_u = u*myCurve_tot_len;
      if (MYDEBUG) {
        MESSAGE("u -> "<<u);
        MESSAGE("myCurve_u  -> "<<myCurve_u);
        MESSAGE("myCurve_tot_len -> "<<myCurve_tot_len);
      }
      ptOnMyCurve = _getPtOnMyCurve( myCurve_u,
                                     myCurve_ways,
                                     myCurve_lengths,
                                     myCurve_starts,
                                     myCurve_list,
                                     myCurve_start_u
                                     );

      node_b = _theMeshDS->AddNode( ptOnMyCurve.X(), ptOnMyCurve.Y(), ptOnMyCurve.Z() );
      edge_ab     = _theMeshDS->AddEdge( node_a, node_b );
      nodesOnEdge.push_back( node_b );
      edgesOnEdge.push_back( edge_ab );
      if  (_nodeXx.count(node_b) >= 1 ) ASSERT(false);
      _nodeXx[node_b] = u;
      node_a = node_b;
  }
  edge_ab      = _theMeshDS->AddEdge( node_a, LAST_NODE );
  nodesOnEdge.push_back( LAST_NODE );
  edgesOnEdge.push_back( edge_ab );
  _nodesOnEdge[&edge] = nodesOnEdge;
  _edgesOnEdge[&edge] = edgesOnEdge;


  if(MYDEBUG) MESSAGE("computeEdgeByAssoc() : end  >>>>>>>>");
  return ok;
}




bool SMESH_HexaBlocks::computeEdgeByShortestWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law)
{
  if(MYDEBUG) MESSAGE("computeEdgeByShortestWire() not implemented");
  return false;
}

bool SMESH_HexaBlocks::computeEdgeByPlanWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law)
{
  if(MYDEBUG) MESSAGE("computeEdgeByPlanWire() not implemented");
  return false;
}

bool SMESH_HexaBlocks::computeEdgeByIsoWire( HEXA_NS::Edge& edge, HEXA_NS::Law& law)
{
  if(MYDEBUG) MESSAGE("computeEdgeByIsoWire() not implemented");
  return false;
}


bool SMESH_HexaBlocks::computeEdgeBySegment(HEXA_NS::Edge& edge, HEXA_NS::Law& law)
{
  if(MYDEBUG) MESSAGE("computeEdgeBySegment() : : begin   <<<<<<");
  ASSERT( _computeVertexOK );
  bool ok = true;

  //vertex from edge
  HEXA_NS::Vertex* vx0 = NULL;
  HEXA_NS::Vertex* vx1 = NULL;

  // way of discretization
  if (edge.getWay() == true){
    vx0 = edge.getVertex(0);
    vx1 = edge.getVertex(1);
  } else {
    vx0 = edge.getVertex(1);
    vx1 = edge.getVertex(0);
  }

  // nodes on mesh
  SMDS_MeshNode* FIRST_NODE = _node[vx0];
  SMDS_MeshNode* LAST_NODE  = _node[vx1];
  SMDS_MeshNode* node_a = NULL; //new node (to be added)
  SMDS_MeshNode* node_b = NULL; //new node (to be added)

  // node and edge creation
  SMESHNodes nodesOnEdge;
  SMESHEdges edgesOnEdge;

  double u; //
  double newNodeX, newNodeY, newNodeZ;
  SMDS_MeshEdge* newEdge = NULL;

  node_a = FIRST_NODE;
  nodesOnEdge.push_back(FIRST_NODE);

  //law of discretization
  int nbNodes = law.getNodes();
  if (MYDEBUG) MESSAGE("nbNodes -> "<<nbNodes);
  for (int i = 0; i < nbNodes; ++i){
    u = _Xx(i, law, nbNodes);
    newNodeX = FIRST_NODE->X() + u * ( LAST_NODE->X() - FIRST_NODE->X() );
    newNodeY = FIRST_NODE->Y() + u * ( LAST_NODE->Y() - FIRST_NODE->Y() );
    newNodeZ = FIRST_NODE->Z() + u * ( LAST_NODE->Z() - FIRST_NODE->Z() );
    node_b = _theMeshDS->AddNode(newNodeX, newNodeY, newNodeZ);
    newEdge = _theMeshDS->AddEdge(node_a, node_b);
    edgesOnEdge.push_back(newEdge);
    nodesOnEdge.push_back(node_b);
    if  (_nodeXx.count(node_b) >= 1 ) ASSERT(false);
    _nodeXx[ node_b ] = u;
    if(MYDEBUG) MESSAGE("_nodeXx <-"<<u);
    node_a = node_b;
  }
  newEdge = _theMeshDS->AddEdge(node_a, LAST_NODE);
  nodesOnEdge.push_back(LAST_NODE);
  edgesOnEdge.push_back(newEdge);

  _nodesOnEdge[&edge] = nodesOnEdge;
  _edgesOnEdge[&edge] = edgesOnEdge;

  if(MYDEBUG) MESSAGE("computeEdgeBySegment() : end  >>>>>>>>");
  return ok;
}


// --------------------------------------------------------------
//                        Quad computing
// --------------------------------------------------------------
std::map<HEXA_NS::Quad*, bool>  SMESH_HexaBlocks::computeQuadWays( HEXA_NS::Document* doc )
{
  std::map<HEXA_NS::Quad*, bool>  quadWays;
  std::map<HEXA_NS::Edge*, std::pair<HEXA_NS::Vertex*, HEXA_NS::Vertex*> > edgeWays;
  std::list<HEXA_NS::Quad*>       skinQuad;
  std::list<HEXA_NS::Quad*>       workingQuad;
  HEXA_NS::Quad* first_q = NULL;
  HEXA_NS::Quad* q = NULL;
  HEXA_NS::Edge* e = NULL;
  HEXA_NS::Vertex *e_0, *e_1 = NULL;

  // FIRST STEP: eliminate free quad + internal quad
  int nTotalQuad = doc->countUsedQuad();
  for (int i=0; i < nTotalQuad; ++i ){
    q = doc->getUsedQuad(i);
    switch ( q->getNbrParents() ){ // parent == hexaedron
      case 0: case 2: quadWays[q] = true; break;
      case 1: skinQuad.push_back(q); break;
      default: if ( q->getNbrParents() > 2 ) ASSERT(false);
    }
  }

  // SECOND STEP: setting edges ways
  while ( skinQuad.size()>0 ){
    if(MYDEBUG) MESSAGE("SEARCHING INITIAL QUAD ..." );
    for ( std::list<HEXA_NS::Quad*>::iterator it = skinQuad.begin(); it != skinQuad.end(); it++ ){
        _searchInitialQuadWay( *it, e_0, e_1 );
        if ( e_0 != NULL && e_1 != NULL ){
          q = first_q = *it;
          break;
        }
    }
    if ( e_0 == NULL && e_1 == NULL ) ASSERT(false);// should never happened,
    if(MYDEBUG) MESSAGE("INITIAL QUAD FOUND!" );
    for ( int j=0 ; j < 4 ; ++j ){
      e = q->getEdge(j);
      if  (    ((e_0 == e->getVertex(0)) && (e_1 == e->getVertex(1)))
            || ((e_0 == e->getVertex(1)) && (e_1 == e->getVertex(0))) ){
        break;
      }
    }
    if(MYDEBUG) MESSAGE("INITIAL EDGE WAY FOUND!" );

    edgeWays[e] = std::make_pair( e_0, e_1 );
    workingQuad.push_back(q);

    while ( workingQuad.size() > 0 ){
        if(MYDEBUG) MESSAGE("COMPUTE QUAD WAY ... ID ="<< q->getId());
        HEXA_NS::Vertex *lastVertex=NULL, *firstVertex = NULL;
        int i = 0;
        std::map<HEXA_NS::Edge*, std::pair<HEXA_NS::Vertex*, HEXA_NS::Vertex*> > localEdgeWays;
        while ( localEdgeWays.size() != 4 ){
            HEXA_NS::Edge* e = q->getEdge(i%4);
            if ( lastVertex == NULL ){
                if ( edgeWays.count(e) == 1 ){
                  if ( q == first_q ){
                    localEdgeWays[e] = std::make_pair( edgeWays[e].first, edgeWays[e].second );
                  } else {
                    localEdgeWays[e] = std::make_pair( edgeWays[e].second, edgeWays[e].first);
                  }
                  firstVertex = localEdgeWays[e].first;
                  lastVertex  = localEdgeWays[e].second;
                }
            } else {
              HEXA_NS::Vertex* e_0 = e->getVertex(0);
              HEXA_NS::Vertex* e_1 = e->getVertex(1);

              if ( lastVertex == e_0 ){
                firstVertex = e_0; lastVertex = e_1;
              } else if ( lastVertex == e_1 ){
                firstVertex = e_1; lastVertex = e_0;
              } else if ( firstVertex == e_0 ) {
                firstVertex = e_1; lastVertex = e_0;
              } else if ( firstVertex == e_1 ) {
                firstVertex = e_0; lastVertex = e_1;
              } else {
                ASSERT(false);
              }
              localEdgeWays[e] = std::make_pair( firstVertex, lastVertex );
              if ( edgeWays.count(e) == 0 ){ // keep current value if present otherwise add it
                edgeWays[e] = localEdgeWays[e];
              }
            }
            ++i;
        }


        //add new working quad
        for ( int i=0 ; i < 4; ++i ){
            HEXA_NS::Quad* next_q = NULL;
            HEXA_NS::Edge* e = q->getEdge(i);
            for ( int j=0 ; j < e->getNbrParents(); ++j ){
                next_q = e->getParent(j);
                if ( next_q == q ){
                  next_q = NULL;
                }
                int fromSkin = std::count( skinQuad.begin(), skinQuad.end(), next_q );
                if (fromSkin != 0){
                  int fromWorkingQuad = std::count( workingQuad.begin(), workingQuad.end(), next_q );
                    if ( fromWorkingQuad == 0 ){
                        workingQuad.push_front( next_q );
                    }
                }
            }
        }

        // setting quad way
        HEXA_NS::Edge* e0 = q->getEdge(0);
        HEXA_NS::Vertex* e0_0 = e0->getVertex(0);

        if (  e0_0 == localEdgeWays[ e0 ].first ){
            quadWays[q] = true;
        } else if ( e0_0 == localEdgeWays[ e0 ].second ){
            quadWays[q] = false;
        } else {
          ASSERT(false);
        }
        workingQuad.remove( q );
        skinQuad.remove( q );
        q = workingQuad.back();
    }
  }

  return quadWays;
}





// std::map<HEXA_NS::Quad*, bool>  SMESH_HexaBlocks::computeQuadWays( HEXA_NS::Document& doc, std::map<HEXA_NS::Quad*, bool>  initQuads )
// {
//   std::map<HEXA_NS::Quad*, bool>  quadWays;
// //   std::map<HEXA_NS::Edge*, bool>  edgeWays;
// //   std::map<HEXA_NS::Edge*, std::pair<HEXA_NS::Vertex*, HEXA_NS::Vertex*> > edgeWays;
//   std::map<HEXA_NS::Quad*, std::pair<HEXA_NS::Vertex*, HEXA_NS::Vertex*> > workingQuads;
//
//   std::list<HEXA_NS::Quad*>       skinQuad;
//   std::list<HEXA_NS::Quad*>       notSkinQuad;
// //   std::list<HEXA_NS::Quad*>       workingQuad;
//   HEXA_NS::Quad* first_q = NULL;
//   HEXA_NS::Quad* q = NULL;
//   HEXA_NS::Edge* e = NULL;
//   HEXA_NS::Vertex *e_0, *e_1 = NULL;
//
//   // FIRST STEP: eliminate free quad + internal quad
//   int nTotalQuad = doc.countQuad();
//   for (int i=0; i < nTotalQuad; ++i ){
//     q = doc.getQuad(i);
//     switch ( q->getNbrParents() ){ // parent == hexaedron
//       case 0: case 2: quadWays[q] = true; break;
// //       case 0: case 2: notSkinQuad.push_back(q); break; //CS_TEST
//       case 1: skinQuad.push_back(q); break;
//       default: if ( q->getNbrParents() > 2 ) ASSERT(false);
//     }
//   }
//
//
//   // SECOND STEP
//   q = first_q = skinQuad.front();
//   e = q->getUsedEdge(0);
//   e_0 = e->getVertex(0);
//   e_1 = e->getVertex(1);
//
//   workingQuads[q] = std::make_pair( e_0, e_1 );
//
//   while ( workingQuads.size() > 0 ){
//       MESSAGE("CURRENT QUAD ------>"<< q->getId());
//       HEXA_NS::Vertex *lastVertex=NULL, *firstVertex = NULL;
//       int i = 0;
//
//       std::map<HEXA_NS::Edge*, std::pair<HEXA_NS::Vertex*, HEXA_NS::Vertex*> > localEdgeWays;
//       while ( localEdgeWays.size() != 4 ){
//           HEXA_NS::Edge* e = q->getUsedEdge(i%4);
//           if ( lastVertex == NULL ){
//               HEXA_NS::Vertex* e_0 = e->getVertex(0);
//               HEXA_NS::Vertex* e_1 = e->getVertex(1);
//
//               if ( (workingQuads[q].first == e_0 and workingQuads[q].second == e_1)
//                     or (workingQuads[q].first == e_1 and workingQuads[q].second == e_0) ){
//                 if ( q == first_q ){
//                   localEdgeWays[e] = std::make_pair( workingQuads[q].first, workingQuads[q].second );
//                   MESSAGE("FIRST QUAD ");
//                 } else {
//                   localEdgeWays[e] = std::make_pair( workingQuads[q].second, workingQuads[q].first);
//                   MESSAGE("NOT FIRST QUAD ");
//                 }
//                 firstVertex = localEdgeWays[e].first;
//                 lastVertex  = localEdgeWays[e].second;
//               }
//           } else {
//             HEXA_NS::Vertex* e_0 = e->getVertex(0);
//             HEXA_NS::Vertex* e_1 = e->getVertex(1);
//             if ( lastVertex == e_0 ){
//               localEdgeWays[e] = std::make_pair( e_0, e_1 );
//               firstVertex = e_0;
//               lastVertex = e_1;
//             } else if ( lastVertex == e_1 ){
//               localEdgeWays[e] = std::make_pair( e_1, e_0 );
//               firstVertex = e_1;
//               lastVertex = e_0;
//             } else if ( firstVertex == e_0 ) {
//               localEdgeWays[e] = std::make_pair( e_1, e_0 );
//               firstVertex = e_1;
//               lastVertex = e_0;
//             } else if ( firstVertex == e_1 ) {
//               localEdgeWays[e] = std::make_pair( e_0, e_1 );
//               firstVertex = e_0;
//               lastVertex = e_1;
//             } else {
//               ASSERT(false);
//             }
//           }
//           ++i;
//       }
//
//
//       //add new working quad
//       for ( int i=0 ; i < 4; ++i ){
//           HEXA_NS::Quad* next_q = NULL;
//           HEXA_NS::Edge* e = q->getUsedEdge(i);
//           MESSAGE("NB PARENTS ="<< e->getNbrParents() );
//           for ( int j=0 ; j < e->getNbrParents(); ++j ){
//               next_q = e->getParent(j);
//               if ( next_q == q ){
//                 next_q = NULL;
//               }
//               int fromSkin = std::count( skinQuad.begin(), skinQuad.end(), next_q );
//               if (fromSkin != 0){
// //                 int fromWorkingQuad = std::count( workingQuads.begin(), workingQuads.end(), next_q );
//                 int fromWorkingQuad = workingQuads.count( next_q );
// //             MESSAGE("CHECK QUAD:"<< newWorkingQuad->getId());
//                   if ( fromWorkingQuad == 0 ){
// //                       workingQuads.push_front( next_q );
//                       workingQuads[ next_q ] = localEdgeWays[e];
// //                   MESSAGE("EDGE :"<<e->getId()<<"ADD QUAD :"<< newWorkingQuad->getId());
//                   }
//               }
//           }
//       }
//
//       //setting quad way
//       HEXA_NS::Edge* e0 = q->getUsedEdge(0);
//       HEXA_NS::Vertex* e0_0 = e0->getVertex(0);
//
//       if (  e0_0 == localEdgeWays[ e0 ].first ){
//           quadWays[q] = true;
//       } else if ( e0_0 == localEdgeWays[ e0 ].second ){
//           quadWays[q] = false;
//       } else {
//         ASSERT(false);
//       }
//       MESSAGE("quadWays ID ="<< q->getId() << ", WAY = " << quadWays[q] );
//
// //       workingQuad.remove( q );
//       workingQuads.erase( q );
//       skinQuad.remove( q );
//       *workingQuads.begin();
//       q = (*workingQuads.begin()).first;
//   }
//   return quadWays;
// }


bool SMESH_HexaBlocks::computeQuad( HEXA_NS::Quad& quad, bool way )
{
  bool ok = false;

  ok = computeQuadByAssoc(quad, way);
  if ( ok == false ){
    ok = computeQuadByFindingGeom(quad, way);
  }
  if ( ok == false ){
    ok = computeQuadByLinearApproximation(quad, way);
  }
  if (ok == true){
    _computeQuadOK = true;
  }
  return ok;
}


bool SMESH_HexaBlocks::computeQuadByAssoc( HEXA_NS::Quad& quad, bool way  )
{
//   int id = quad.getId();
//   if ( id != 11 )  return false; //7
  if (MYDEBUG){
    MESSAGE("computeQuadByLinearApproximation() : : begin   <<<<<<");
    MESSAGE("quadID = "<<quad.getId());
  }
  ASSERT( _computeEdgeOK );
  bool ok = true;

  ArrayOfSMESHNodes nodesOnQuad; // nodes in this quad ( to be added on the mesh )
  SMESHFaces      facesOnQuad;
  SMDS_MeshFace*  newFace = NULL;
  std::vector<double> xx, yy;

  // Elements for quad computation
  SMDS_MeshNode *S1, *S2, *S4, *S3;

//   bool initOk = _computeQuadInit( quad, eh, eb, eg, ed, S1, S2, S3, S4 );
  bool initOk = _computeQuadInit( quad, nodesOnQuad, xx, yy );
  if ( initOk == false ){
    return false;
  }

  int nbass  = quad.countAssociation ();
  if (nbass==0)
     {
     if(MYDEBUG) MESSAGE("computeQuadByAssoc() : end  >>>>>>>>");
     return false;
     }

  TopoDS_Shape shapeOrCompound = getFaceShapes ( quad );


  std::map<SMDS_MeshNode*, gp_Pnt> interpolatedPoints;
  int iSize = nodesOnQuad.size();
  int jSize = nodesOnQuad[0].size();

  S1 = nodesOnQuad[0][0];
  S2 = nodesOnQuad[iSize-1][0];
  S4 = nodesOnQuad[0][jSize-1];
  S3 = nodesOnQuad[iSize-1][jSize-1];


  for (int j = 1; j < jSize; ++j){
    for (int i = 1; i < iSize; ++i){
        SMDS_MeshNode* n1 = nodesOnQuad[i-1][j];
        SMDS_MeshNode* n2 = nodesOnQuad[i-1][j-1];
        SMDS_MeshNode* n3 = nodesOnQuad[i][j-1];
        SMDS_MeshNode* n4 = nodesOnQuad[i][j];

        if ( n4 == NULL ){
            double newNodeX, newNodeY, newNodeZ;
            SMDS_MeshNode* Ph = nodesOnQuad[i][jSize-1];   //dNodes[h_i];
            SMDS_MeshNode* Pb = nodesOnQuad[i][0];   //bNodes[b_i];
            SMDS_MeshNode* Pg = nodesOnQuad[0][j];   //gNodes[g_j];
            SMDS_MeshNode* Pd = nodesOnQuad[iSize-1][j];  //dNodes[d_j];
            double u = xx[i];
            double v = yy[j];

            _nodeInterpolationUV(u, v, Pg, Pd, Ph, Pb, S1, S2, S3, S4, newNodeX, newNodeY, newNodeZ);
              gp_Pnt newPt = gp_Pnt( newNodeX, newNodeY, newNodeZ );//interpolated point
              gp_Pnt pt1;
              gp_Pnt pt3;
              if ( interpolatedPoints.count(n1) > 0 ){
                      pt1 = interpolatedPoints[n1];
              } else {
                      pt1 = gp_Pnt( n1->X(), n1->Y(), n1->Z() );
              }
              if ( interpolatedPoints.count(n3) > 0 ){
                      pt3 = interpolatedPoints[n3];
              } else {
                      pt3 = gp_Pnt( n3->X(), n3->Y(), n3->Z() );
              }
              gp_Vec vec1( newPt, pt1 );
              gp_Vec vec2( newPt, pt3 );

              gp_Pnt ptOnShape = _intersect(newPt, vec1, vec2, shapeOrCompound);
              newNodeX = ptOnShape.X();
              newNodeY = ptOnShape.Y();
              newNodeZ = ptOnShape.Z();
              n4 = _theMeshDS->AddNode( newNodeX, newNodeY, newNodeZ );
              nodesOnQuad[i][j] = n4;
              interpolatedPoints[ n4 ] = newPt;

              if (MYDEBUG) {
                  MESSAGE("u parameter is "<<u);
                  MESSAGE("v parameter is "<<v);
                  MESSAGE("point interpolated ("<<newPt.X()<<","<<newPt.Y()<<","<<newPt.Z()<<" )");
                  MESSAGE("point on shape     ("<<newNodeX<<","<<newNodeY<<","<<newNodeZ<<" )");
              }
        }

        if (MYDEBUG){
          MESSAGE("n1 (" << n1->X() << "," << n1->Y() << "," << n1->Z() << ")");
          MESSAGE("n2 (" << n2->X() << "," << n2->Y() << "," << n2->Z() << ")");
          MESSAGE("n4 (" << n4->X() << "," << n4->Y() << "," << n4->Z() << ")");
          MESSAGE("n3 (" << n3->X() << "," << n3->Y() << "," << n3->Z() << ")");
        }

        if ( way == true ){
            if (MYDEBUG) MESSAGE("AddFace( n1, n2, n3, n4 )");
            newFace = _theMeshDS->AddFace( n1, n2, n3, n4 );
        } else {
            if (MYDEBUG) MESSAGE("AddFace( n4, n3, n2, n1 )");
            newFace = _theMeshDS->AddFace( n4, n3, n2, n1 );
        }
        facesOnQuad.push_back(newFace);
      }
  }
  _quadNodes[ &quad ] = nodesOnQuad;
  _facesOnQuad[&quad] = facesOnQuad;

  if(MYDEBUG) MESSAGE("computeQuadByLinearApproximation() : end  >>>>>>>>");
  return ok;
}


bool SMESH_HexaBlocks::computeQuadByFindingGeom( HEXA_NS::Quad& quad, bool way )
{
  if(MYDEBUG) MESSAGE("computeQuadByFindingGeom() not implemented");
  return false;
}

bool SMESH_HexaBlocks::_computeQuadInit(
  HEXA_NS::Quad& quad,
  ArrayOfSMESHNodes& nodesOnQuad,
  std::vector<double>& xx, std::vector<double>& yy)
{
  if(MYDEBUG) MESSAGE("_computeQuadInit() : begin ---------------");
  bool ok = true;

  SMDS_MeshNode *S1, *S2, *S4, *S3;
  HEXA_NS::Edge *eh, *eb, *eg, *ed;
  HEXA_NS::Edge *e1, *e2, *e3, *e4;
  HEXA_NS::Vertex *e1_0, *e1_1, *e2_0, *e2_1, *e3_0, *e3_1, *e4_0, *e4_1;

  e1 = quad.getEdge(0);
  e2 = quad.getEdge(1);
  e3 = quad.getEdge(2);
  e4 = quad.getEdge(3);

  e1_0 = e1->getVertex(0); e1_1 = e1->getVertex(1);
  e2_0 = e2->getVertex(0); e2_1 = e2->getVertex(1);
  e3_0 = e3->getVertex(0); e3_1 = e3->getVertex(1);
  e4_0 = e4->getVertex(0); e4_1 = e4->getVertex(1);

  //S1, S2
  S1 = _node[e1_0]; S2 = _node[e1_1];
  eb = e1; eh = e3;
  //S4
  if ( e1_0 == e2_0 ){
    S4 = _node[e2_1];
    eg = e2; ed = e4;
  } else if ( e1_0 == e2_1 ){
    S4 = _node[e2_0];
    eg = e2; ed = e4;
  } else if ( e1_0 == e4_0 ){
    S4 = _node[e4_1];
    eg = e4; ed = e2;
  } else if ( e1_0 == e4_1 ){
    S4 = _node[e4_0];
    eg = e4; ed = e2;
  } else {
    ASSERT(false);
  }
  //S3
  if ( S4 == _node[e3_0] ){
    S3 = _node[e3_1];
  } else if ( S4 == _node[e3_1] ){
    S3 = _node[e3_0];
  } else {
    ASSERT(false);
  }

  SMESHNodes hNodes = _nodesOnEdge[eh];
  SMESHNodes bNodes = _nodesOnEdge[eb];
  SMESHNodes gNodes = _nodesOnEdge[eg];
  SMESHNodes dNodes = _nodesOnEdge[ed];
  nodesOnQuad.resize( bNodes.size(), SMESHNodes(gNodes.size(), static_cast<SMDS_MeshNode*>(NULL)) );


  int i, j, _i, _j;
//   int &b_i = i, &h_i = i, &g_j = j, &d_j = j;
  int *b_i = &i, *h_i = &i, *g_j = &j, *d_j = &j;
  bool uWay = true, vWay = true;

  if ( bNodes[0] != S1 ){
    b_i = &_i;
    uWay = false;
    ASSERT( bNodes[bNodes.size()-1] == S1 );
  } else {
    ASSERT( bNodes[0] == S1);
  }
  if ( hNodes[0] != S4 ){
    h_i = &_i;
  } else {
    ASSERT( hNodes[0] == S4 );
  }
  if ( gNodes[0] != S1 ){
    g_j = &_j;
    vWay = false;
  } else {
    ASSERT( gNodes[0] == S1 );
  }
  if ( dNodes[0] != S2 ){
    d_j = &_j;
  } else {
    ASSERT( dNodes[0] == S2 );
  }

  //bNodes, hNodes
  double u;
  for (i = 0, _i = bNodes.size()-1; i < bNodes.size(); ++i, --_i){
    nodesOnQuad[i][0]                = bNodes[*b_i];
    nodesOnQuad[i][gNodes.size()-1 ] = hNodes[*h_i];

    u = _nodeXx[ bNodes[*b_i] ];
    if ( uWay == true ){
      xx.push_back(u);
    } else {
      xx.push_back(1.-u);
    }
  }
  if ( S1 != nodesOnQuad[0][0] ){
    if(MYDEBUG) MESSAGE("ZZZZZZZZZZZZZZZZ quadID = "<<quad.getId());
  }
//   ASSERT( S1 == nodesOnQuad[0][0] );

  //gNodes, dNodes
  double v;
  for (j = 0, _j = gNodes.size()-1; j < gNodes.size(); ++j, --_j){
    nodesOnQuad[0][j] = gNodes[*g_j];
    if ( S1 != nodesOnQuad[0][0] ){
      if(MYDEBUG) MESSAGE("XXXXXXXXXXXXXXXX quadID = "<<quad.getId());
    }
//     ASSERT( S1 == nodesOnQuad[0][0] );
    nodesOnQuad[bNodes.size()-1][j] = dNodes[*d_j];
    v = _nodeXx[ gNodes[*g_j] ];
    if ( vWay == true ){
      yy.push_back(v);
    } else {
      yy.push_back(1.-v);
    }
  }


  int iSize = nodesOnQuad.size();
  int jSize = nodesOnQuad[0].size();
  ASSERT( iSize = bNodes.size() );
  ASSERT( jSize = gNodes.size() );

//   ASSERT( S1 == nodesOnQuad[0][0] );
//   ASSERT( S2 == nodesOnQuad[iSize-1][0]);
//   ASSERT( S4 == nodesOnQuad[0][jSize-1]);
//   ASSERT( S3 == nodesOnQuad[iSize-1][jSize-1]);

  return ok;
}


bool SMESH_HexaBlocks::computeQuadByLinearApproximation( HEXA_NS::Quad& quad, bool way )
{
//   int id = quad.getId();
//   if ( quad.getId() != 66 )  return false; //7, 41
  if (MYDEBUG){
    MESSAGE("computeQuadByLinearApproximation() : : begin   <<<<<<");
    MESSAGE("quadID = "<<quad.getId());
  }
  ASSERT( _computeEdgeOK );
  bool ok = true;

  ArrayOfSMESHNodes nodesOnQuad; // nodes in this quad ( to be added on the mesh )
  SMESHFaces      facesOnQuad;
  SMDS_MeshFace*  newFace = NULL;
  std::vector<double> xx, yy;

  // Elements for quad computation
  SMDS_MeshNode *S1, *S2, *S4, *S3;

//   bool initOk = _computeQuadInit( quad, eh, eb, eg, ed, S1, S2, S3, S4 );
  bool initOk = _computeQuadInit( quad, nodesOnQuad, xx, yy );
  if ( initOk == false ){
    return false;
  }

  int iSize = nodesOnQuad.size();
  int jSize = nodesOnQuad[0].size();

  S1 = nodesOnQuad[0][0];
//   S2 = nodesOnQuad[bNodes.size()-1][0];
  S2 = nodesOnQuad[iSize-1][0];
  S4 = nodesOnQuad[0][jSize-1];
  S3 = nodesOnQuad[iSize-1][jSize-1];

  for (int j = 1; j < jSize; ++j){
    for (int i = 1; i < iSize; ++i){
        SMDS_MeshNode* n1 = nodesOnQuad[i-1][j];
        SMDS_MeshNode* n2 = nodesOnQuad[i-1][j-1];
        SMDS_MeshNode* n3 = nodesOnQuad[i][j-1];
        SMDS_MeshNode* n4 = nodesOnQuad[i][j];

        if ( n4 == NULL ){
            double newNodeX, newNodeY, newNodeZ;
            SMDS_MeshNode* Ph = nodesOnQuad[i][jSize-1];   //dNodes[h_i];
            SMDS_MeshNode* Pb = nodesOnQuad[i][0];   //bNodes[b_i];
            SMDS_MeshNode* Pg = nodesOnQuad[0][j];   //gNodes[g_j];
            SMDS_MeshNode* Pd = nodesOnQuad[iSize-1][j];  //dNodes[d_j];
            double u = xx[i];
            double v = yy[j];

            _nodeInterpolationUV(u, v, Pg, Pd, Ph, Pb, S1, S2, S3, S4, newNodeX, newNodeY, newNodeZ);
            n4 = _theMeshDS->AddNode( newNodeX, newNodeY, newNodeZ );
            nodesOnQuad[i][j] = n4;
        }

        if (MYDEBUG){
          MESSAGE("n1 (" << n1->X() << "," << n1->Y() << "," << n1->Z() << ")");
          MESSAGE("n2 (" << n2->X() << "," << n2->Y() << "," << n2->Z() << ")");
          MESSAGE("n4 (" << n4->X() << "," << n4->Y() << "," << n4->Z() << ")");
          MESSAGE("n3 (" << n3->X() << "," << n3->Y() << "," << n3->Z() << ")");
        }

        if ( way == true ){
            if (MYDEBUG) MESSAGE("AddFace( n1, n2, n3, n4 )");
            newFace = _theMeshDS->AddFace( n1, n2, n3, n4 );
        } else {
            if (MYDEBUG) MESSAGE("AddFace( n4, n3, n2, n1 )");
            newFace = _theMeshDS->AddFace( n4, n3, n2, n1 );
        }
        facesOnQuad.push_back(newFace);
      }
  }
  _quadNodes[ &quad ] = nodesOnQuad;
  _facesOnQuad[&quad] = facesOnQuad;

  if(MYDEBUG) MESSAGE("computeQuadByLinearApproximation() : end  >>>>>>>>");
  return ok;
}


// --------------------------------------------------------------
//                      Hexa computing
// --------------------------------------------------------------
bool SMESH_HexaBlocks::computeHexa( HEXA_NS::Document* doc )
{
  if(MYDEBUG) MESSAGE("computeHexa() : : begin   <<<<<<");
  bool ok=false;

  SMESH_MesherHelper aHelper(*_theMesh);
  TopoDS_Shape shape = _theMesh->GetShapeToMesh();
  aHelper.SetSubShape( shape );
  aHelper.SetElementsOnShape( true );

  SMESH_Gen* gen = _theMesh->GetGen();
  SMESH_HexaFromSkin_3D algo( gen->GetANewId(), 0, gen, doc );
  algo.InitComputeError();
  try {
      ok = algo.Compute( *_theMesh, &aHelper, _volumesOnHexa, _node );
  } catch(...) {
    if(MYDEBUG) MESSAGE("SMESH_HexaFromSkin_3D error!!! ");
  }
  if (MYDEBUG){
    MESSAGE("SMESH_HexaFromSkin_3D.comment = "<<algo.GetComputeError()->myComment);
    MESSAGE("computeHexa() : end  >>>>>>>>");
  }
  return ok;
}



// --------------------------------------------------------------
//                Document computing
// --------------------------------------------------------------
bool SMESH_HexaBlocks::computeDoc(  HEXA_NS::Document* doc )
{
  if(MYDEBUG) MESSAGE("computeDoc() : : begin   <<<<<<");
  bool ok = true;

  // A) Vertex computation

  doc->lockDump ();
  int nVertex = doc->countUsedVertex();
  HEXA_NS::Vertex* vertex = NULL;

  for (int j=0; j <nVertex; ++j ){ //Computing each vertex of the document
    vertex = doc->getUsedVertex(j);
    ok = computeVertex(*vertex);
  }

  // B) Edges computation
  int nbPropa = 0;
  HEXA_NS::Propagation* propa = NULL;
  HEXA_NS::Law*         law   = NULL;
  HEXA_NS::Edges edges;

  nbPropa = doc->countPropagation();
  for (int j=0; j < nbPropa; ++j ){//Computing each edge's propagations of the document
    propa = doc->getPropagation(j);
    edges = propa->getEdges();
    law   = propa->getLaw();
//     ASSERT( law );
    if (law == NULL){
      law = doc->getLaw(0); // default law
    }
    for( HEXA_NS::Edges::const_iterator iter = edges.begin();
        iter != edges.end();
        ++iter ){
        ok = computeEdge(**iter, *law);
    }
  }
  // C) Quad computation
  std::map<HEXA_NS::Quad*, bool>  quadWays = computeQuadWays(doc);
  int nQuad = doc->countUsedQuad();
  HEXA_NS::Quad* q = NULL;
  for (int j=0; j <nQuad; ++j ){ //Computing each quad of the document
    q = doc->getUsedQuad(j);
    int id = q->getId();
    if ( quadWays.count(q) > 0 )
      ok = computeQuad( *q, quadWays[q] );
    else
      if(MYDEBUG) MESSAGE("NO QUAD WAY ID = "<<id);

  }

  // D) Hexa computation: Calling HexaFromSkin algo
  ok = computeHexa(doc);

  if(MYDEBUG) MESSAGE("computeDoc() : end  >>>>>>>>");
  doc->lockDump ();
  return ok;
}


void SMESH_HexaBlocks::buildGroups(HEXA_NS::Document* doc)
{
  if (MYDEBUG){
    MESSAGE("_addGroups() : : begin   <<<<<<");
    MESSAGE("_addGroups() : : nb. hexas= " << doc->countUsedHexa());
    MESSAGE("_addGroups() : : nb. quads= " << doc->countUsedQuad());
    MESSAGE("_addGroups() : : nb. edges= " << doc->countUsedEdge());
    MESSAGE("_addGroups() : : nb. nodes= " << doc->countUsedVertex());
  }
  // Looping on each groups of the document
  for ( int i=0; i < doc->countGroup(); i++ ){
      _fillGroup( doc->getGroup(i) );
  };

  if(MYDEBUG) MESSAGE("_addGroups() : end  >>>>>>>>");
}

// --------------------------------------------------------------
//                      PRIVATE METHODS
// --------------------------------------------------------------
double SMESH_HexaBlocks::_Xx( double i, HEXA_NS::Law law, double nbNodes) //, double pos0 )
{
  double result;
  double u0;

  HEXA_NS::KindLaw k = law.getKind();
  double coeff       = law.getCoefficient();
  switch (k){
    case HEXA_NS::Uniform:
        result = (i+1)/(nbNodes+1);
        if(MYDEBUG) MESSAGE( "_Xx():" << " Uniform u("<<i<< ")"<< " = " << result);
        break;
    case HEXA_NS::Arithmetic:
        u0 = 1./(nbNodes + 1.) - (coeff*nbNodes)/2.;
//         ASSERT(u0>0);
        if ( u0 <= 0 ) throw (SALOME_Exception(LOCALIZED("Arithmetic discretization : check coefficient")));
        if (i==0){
          result = u0;
        } else {
          result = (i + 1.)*u0 + coeff*i*(i+1.)/2.;
        };
        if(MYDEBUG) MESSAGE( "_Xx():" << " Arithmetic u("<<i<< ")"<< " = " << result);
        break;
    case HEXA_NS::Geometric:
        u0 = (1.-coeff)/(1.-pow(coeff, nbNodes + 1) )  ;
//         ASSERT(u0>0);
        if ( u0 <= 0 ) throw (SALOME_Exception(LOCALIZED("Geometric discretization : check coefficient")));
        if (i==0){
          result = u0;
        } else {
          result = u0 * (1.- pow(coeff, i + 1) )/(1.-coeff) ;
        };
        if(MYDEBUG) MESSAGE( "_Xx():" << " Geometric u("<<i<< ")"<< " = " << result);
        break;
  }
  return result;
}


// ============================================================= _edgeLength
double SMESH_HexaBlocks::_edgeLength(const TopoDS_Edge & E)
{
  if(MYDEBUG) MESSAGE("_edgeLength() : : begin   <<<<<<");
  double UMin = 0, UMax = 0;
  if (BRep_Tool::Degenerated(E))
    return 0;
  TopLoc_Location L;
  Handle(Geom_Curve) C = BRep_Tool::Curve(E, L, UMin, UMax);
  GeomAdaptor_Curve AdaptCurve(C);
  double length = GCPnts_AbscissaPoint::Length(AdaptCurve, UMin, UMax);
  if(MYDEBUG) MESSAGE("_edgeLength() : end  >>>>>>>>");
  return length;
}
//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8
// ============================================================== _buildMyCurve
// === construire ma courbe a moi
void SMESH_HexaBlocks::_buildMyCurve(
    const gp_Pnt&                               myCurve_pt_start,  //IN
    const gp_Pnt&				myCurve_pt_end,    //IN
    std::list< BRepAdaptor_Curve* >& 	        myCurve_list,        //INOUT
    double& 				        myCurve_tot_len, //INOUT
    std::map< BRepAdaptor_Curve*, double>& 	myCurve_lengths,//INOUT
    std::map< BRepAdaptor_Curve*, bool>& 	myCurve_ways,   //INOUT
    std::map< BRepAdaptor_Curve*, double>&      myCurve_starts,   //INOUT
    HEXA_NS::Edge& 	                        edge) // For error diagnostic
{
    if(MYDEBUG) MESSAGE("_buildMyCurve() : : begin   <<<<<<");
    bool current_way  = true;
    myCurve_tot_len    = 0.;
    BRepAdaptor_Curve* thePreviousCurve = NULL;
    BRepAdaptor_Curve* theCurve         = NULL;

    gp_Pnt  curv_start, curv_end;
    gp_Pnt  p_curv_start , p_curv_end;
    double u_start     = 0;
    double curv_length = 0;

    int nbass = edge.countAssociation();
    for (int nro = 0 ; nro < nbass ; ++nro)
        {
        theCurve = make_curve (curv_start, curv_end, curv_length, u_start,
                               edge, nro);
        if (theCurve != NULL)
           {
            bool    sens   = true;
            if ( thePreviousCurve == NULL )
               {
               // setting current_way and first curve way
               if ( myCurve_pt_start.IsEqual(curv_start, HEXA_EPS) )
                  {
                  current_way = true;
                  sens        = true;
                  }
               else if ( myCurve_pt_start.IsEqual(curv_end, HEXA_EPS) )
                  {
                  current_way = true;
                  sens        = false;
                  }
               else if ( myCurve_pt_end.IsEqual(curv_end, HEXA_EPS) )
                  {
                  current_way = false;
                  sens        = true;
                  }
               else if ( myCurve_pt_end.IsEqual(curv_start, HEXA_EPS) )
                  {
                  current_way = false;
                  sens        = false;
                  }
               else
                  {
                 if (MYDEBUG)
                     MESSAGE("SOMETHING WRONG on edge association... Bad script?");
                 edge.dumpAsso();
                 throw (SALOME_Exception (LOCALIZED("Edge association : check association parameters ( first, last ) between HEXA model and CAO")));
                   }

               }
                // it is not the first or last curve.
                // ways are calculated between previous and new one.
            else
               {
               if (    p_curv_end.IsEqual( curv_end, HEXA_EPS  )
                    || p_curv_start.IsEqual( curv_start, HEXA_EPS ) )
                  {
                  sens  = NOT myCurve_ways[thePreviousCurve];// opposite WAY
                  }
               else if (   p_curv_end.IsEqual( curv_start, HEXA_EPS )
                        || p_curv_start.IsEqual( curv_end, HEXA_EPS ) )
                  {
                  sens  = myCurve_ways[thePreviousCurve];// same WAY
                  }
               else
                  {
                  if (MYDEBUG)
                     MESSAGE("SOMETHING WRONG on edge association... bad script?");
//                     ASSERT(false);
                  edge.dumpAsso();
                  throw (SALOME_Exception(LOCALIZED("Edge association : Check association parameters ( first, last ) between HEXA model and CAO")));
                }
            }

            myCurve_list.push_back( theCurve );
            myCurve_ways   [theCurve] = sens;
            myCurve_starts [theCurve] = u_start;
            myCurve_lengths[theCurve] = curv_length;
            myCurve_tot_len          += curv_length;

            thePreviousCurve  = theCurve;
            p_curv_start = curv_start;
            p_curv_end   = curv_end;
           }//if ( theCurveLength > 0 )
    }// for


    if ( NOT current_way ){
        std::list< BRepAdaptor_Curve* > tmp( myCurve_list.size() );
        std::copy( myCurve_list.rbegin(), myCurve_list.rend(), tmp.begin() );
        myCurve_list = tmp;
    }

    if (MYDEBUG) {
      MESSAGE("current_way  was :" << current_way);
      MESSAGE("_buildMyCurve() : end  >>>>>>>>");
    }
}




gp_Pnt SMESH_HexaBlocks::_getPtOnMyCurve(
    const double&                             myCurve_u,      //IN
    std::map< BRepAdaptor_Curve*, bool>&      myCurve_ways,   //IN
    std::map< BRepAdaptor_Curve*, double>&    myCurve_lengths,//IN
    std::map< BRepAdaptor_Curve*, double>&    myCurve_starts, //IN
    std::list< BRepAdaptor_Curve* >&          myCurve_list,        //INOUT
    double&                                   myCurve_start ) //INOUT
//     std::map< BRepAdaptor_Curve*, double>&  myCurve_firsts,
//     std::map< BRepAdaptor_Curve*, double>&  myCurve_lasts,
{
  if(MYDEBUG) MESSAGE("_getPtOnMyCurve() : : begin   <<<<<<");
  gp_Pnt ptOnMyCurve;

  // looking for curve which contains parameter myCurve_u
  BRepAdaptor_Curve* curve      = myCurve_list.front();
  double            curve_start = myCurve_start;
  double            curve_end   = curve_start + myCurve_lengths[curve];
  double            curve_u;
  GCPnts_AbscissaPoint discret;

  if (MYDEBUG){
    MESSAGE("looking for curve               = "<<(long) curve);
    MESSAGE("looking for curve: curve_u      = "<<myCurve_u);
    MESSAGE("looking for curve: curve_start  = "<<curve_start);
    MESSAGE("looking for curve: curve_end    = "<<curve_end);
    MESSAGE("looking for curve: curve_lenght = "<<myCurve_lengths[curve]);
    MESSAGE("looking for curve: curve.size _lenght= "<<myCurve_list.size());
  }
  while ( !( (myCurve_u >= curve_start) &&  (myCurve_u <= curve_end) ) ) {

    ASSERT( myCurve_list.size() != 0 );
    myCurve_list.pop_front();
    curve       = myCurve_list.front();
    curve_start = curve_end;
    curve_end   = curve_start + myCurve_lengths[curve];
    if (MYDEBUG){
      MESSAGE("go next curve: curve_lenght = "<<myCurve_lengths[curve]);
      MESSAGE("go next curve: curve_start  = "<<curve_start);
      MESSAGE("go next curve: curve_end    = "<<curve_end);
      MESSAGE("go next curve: myCurve_u    = "<<myCurve_u);
    }
  }
  myCurve_start = curve_start;

  // compute point
  if ( myCurve_ways[curve] ){
    discret = GCPnts_AbscissaPoint( *curve, (myCurve_u - curve_start), myCurve_starts[curve] );
  } else {
    discret = GCPnts_AbscissaPoint( *curve, myCurve_lengths[curve]- (myCurve_u - curve_start), myCurve_starts[curve] );
  }
  // PutData (discret);
  ASSERT(discret.IsDone());
  curve_u = discret.Parameter();
  ptOnMyCurve = curve->Value( curve_u );

  if (MYDEBUG){
    MESSAGE("curve found!");
    MESSAGE("curve_u = "<< curve_u);
    MESSAGE("curve way = "<< myCurve_ways[curve]);
    MESSAGE("_getPtOnMyCurve() : end  >>>>>>>>");
  }
  return ptOnMyCurve;
}




void SMESH_HexaBlocks::_nodeInterpolationUV(double u, double v,
    SMDS_MeshNode* Pg, SMDS_MeshNode* Pd, SMDS_MeshNode* Ph, SMDS_MeshNode* Pb,
    SMDS_MeshNode* S1, SMDS_MeshNode* S2, SMDS_MeshNode* S3, SMDS_MeshNode* S4,
    double& xOut, double& yOut, double& zOut )
{
  if (MYDEBUG){
    MESSAGE("_nodeInterpolationUV() IN:");
    MESSAGE("u ( "<< u <<" )");
    MESSAGE("v ( "<< v <<" )");

    MESSAGE("S1 (" << S1->X() << "," << S1->Y() << "," << S1->Z() << ")");
    MESSAGE("S2 (" << S2->X() << "," << S2->Y() << "," << S2->Z() << ")");
    MESSAGE("S4 (" << S4->X() << "," << S4->Y() << "," << S4->Z() << ")");
    MESSAGE("S3 (" << S3->X() << "," << S3->Y() << "," << S3->Z() << ")");

    MESSAGE("Pg (" << Pg->X() << "," << Pg->Y() << "," << Pg->Z() << ")");
    MESSAGE("Pd (" << Pd->X() << "," << Pd->Y() << "," << Pd->Z() << ")");
    MESSAGE("Ph (" << Ph->X() << "," << Ph->Y() << "," << Ph->Z() << ")");
    MESSAGE("Pb (" << Pb->X() << "," << Pb->Y() << "," << Pb->Z() << ")");
  }

  xOut = ((1.-u)*Pg->X() + v*Ph->X() + u*Pd->X() + (1.-v)*Pb->X()) - (1.-u)*(1.-v)*S1->X() - u*(1.-v)*S2->X() - u*v*S3->X() - (1.-u)*v*S4->X();
  yOut = ((1.-u)*Pg->Y() + v*Ph->Y() + u*Pd->Y() + (1.-v)*Pb->Y()) - (1.-u)*(1.-v)*S1->Y() - u*(1.-v)*S2->Y() - u*v*S3->Y() - (1.-u)*v*S4->Y();
  zOut = ((1.-u)*Pg->Z() + v*Ph->Z() + u*Pd->Z() + (1.-v)*Pb->Z()) - (1.-u)*(1.-v)*S1->Z() - u*(1.-v)*S2->Z() - u*v*S3->Z() - (1.-u)*v*S4->Z();

  if (MYDEBUG){
    MESSAGE("_nodeInterpolationUV() OUT("<<xOut<<","<<yOut<<","<<zOut<<" )");
  }
}

// =========================================================== getFaceShapes
TopoDS_Shape SMESH_HexaBlocks::getFaceShapes (Hex::Quad& quad)
{
   int             nbass = quad.countAssociation ();
   Hex::FaceShape* face  = quad.getAssociation (0);
   if (nbass==1)
       return face->getShape ();

   TopoDS_Compound compound;
   BRep_Builder    builder;
   builder.MakeCompound (compound);

   for (int nro=0 ; nro < nbass ; nro++)
       {
       face = quad.getAssociation (nro);
       TopoDS_Shape shape = face->getShape ();
       builder.Add (compound, shape);
       }

   return compound;
}


// ================================================== carre
inline double carre (double val)
{
  return val*val;
}
// ================================================== dist2
inline double dist2 (const gp_Pnt& pt1, const gp_Pnt& pt2)
{
   double dist = carre (pt2.X()-pt1.X()) + carre (pt2.Y()-pt1.Y())
                                         + carre (pt2.Z()-pt1.Z());
   return dist;
}
// ================================================== _intersect
gp_Pnt SMESH_HexaBlocks::_intersect( const gp_Pnt& Pt,
                                     const gp_Vec& u, const gp_Vec& v,
                                     const TopoDS_Shape& shape,
                                     Standard_Real tol )
{
  gp_Pnt result;

  gp_Vec normale = u^v;
  gp_Dir dir(normale);
  gp_Lin li( Pt, dir );

  Standard_Real s = -Precision::Infinite();
  Standard_Real e = +Precision::Infinite();

  IntCurvesFace_ShapeIntersector inter;
  inter.Load(shape, tol);
//   inter.Load(S, tol);
  inter.Perform(li, s, e);//inter.PerformNearest(li, s, e);

/***********************************************  Abu 2011-11-04 */
  if ( inter.IsDone() )
     {
     result = inter.Pnt(1);//first
     int nbrpts = inter.NbPnt();
     if (nbrpts>1)
        {
        double d0 = dist2 (result, Pt);
        for (int i=2; i <= inter.NbPnt(); ++i )
            {
            double d1 = dist2 (Pt, inter.Pnt(i));
            if (d1<d0)
               {
               d0 = d1;
               result = inter.Pnt (i);
               }
            }
        }
/**********************************************  Abu 2011-11-04 (getEnd()) */
    if (MYDEBUG){
      MESSAGE("_intersect() : OK");
      for ( int i=1; i <= inter.NbPnt(); ++i ){
        gp_Pnt tmp = inter.Pnt(i);
        MESSAGE("_intersect() : pnt("<<i<<") = ("<<tmp.X()<<","<<tmp.Y()<<","<<tmp.Z()<<" )");
      }
    }
    _found +=1;
  } else {
    if(MYDEBUG) MESSAGE("_intersect() : KO");
    result = Pt;
    _notFound +=1;
  }
  _total+=1;

  return result;
}

// parameters q : IN,  v0: INOUT, v1: INOUT
void SMESH_HexaBlocks::_searchInitialQuadWay( HEXA_NS::Quad* q, HEXA_NS::Vertex*& v0, HEXA_NS::Vertex*& v1 )
{
  if(MYDEBUG) MESSAGE("_searchInitialQuadWay() : begin");
  v0 = NULL; v1 = NULL;
  if ( q->getNbrParents() != 1 ) return; // q must be a skin quad

  HEXA_NS::Vertex* qA = q->getVertex(0);
  HEXA_NS::Vertex* qB = q->getVertex(1);
  HEXA_NS::Vertex* qC = q->getVertex(2);
  HEXA_NS::Vertex* qD = q->getVertex(3);

  // searching for vertex on opposed quad
  HEXA_NS::Vertex *qAA = NULL, *qBB = NULL, *qCC = NULL, *qDD = NULL;
  HEXA_NS::Hexa* h = q->getParent(0);
  for( int i=0; i < h->countEdge(); ++i  ){
    HEXA_NS::Edge* e = h->getEdge(i);
    HEXA_NS::Vertex* e0 = e->getVertex(0);
    HEXA_NS::Vertex* e1 = e->getVertex(1);

    if ( e0 == qA && e1 != qB && e1 != qC && e1 != qD ){
      qAA = e1;
    } else if ( e1 == qA && e0 != qB && e0 != qC && e0 != qD ){
      qAA = e0;
    } else if ( e0 == qB && e1 != qA && e1 != qC && e1 != qD ){
      qBB = e1;
    } else if ( e1 == qB && e0 != qA && e0 != qC && e0 != qD ){
      qBB = e0;
    } else if ( e0 == qC && e1 != qA && e1 != qB && e1 != qD ){
      qCC = e1;
    } else if ( e1 == qC && e0 != qA && e0 != qB && e0 != qD ){
      qCC = e0;
    } else if ( e0 == qD && e1 != qA && e1 != qB && e1 != qC ){
      qDD = e1;
    } else if ( e1 == qD && e0 != qA && e0 != qB && e0 != qC ){
      qDD = e0;
    }
  }

  // working on final value ( point on CAO ), not on model
  SMDS_MeshNode *nA = _node[qA], *nAA = _node[qAA];
  SMDS_MeshNode *nB = _node[qB], *nBB = _node[qBB];
  SMDS_MeshNode *nC = _node[qC], *nCC = _node[qCC];
  SMDS_MeshNode *nD = _node[qD], *nDD = _node[qDD];

  gp_Pnt pA( nA->X(), nA->Y(), nA->Z() );
  gp_Pnt pB( nB->X(), nB->Y(), nB->Z() );
  gp_Pnt pC( nC->X(), nC->Y(), nC->Z() );
  gp_Pnt pD( nD->X(), nD->Y(), nD->Z() );

  gp_Pnt pAA( nAA->X(), nAA->Y(), nAA->Z() );
  gp_Pnt pBB( nBB->X(), nBB->Y(), nBB->Z() );
  gp_Pnt pCC( nCC->X(), nCC->Y(), nCC->Z() );
  gp_Pnt pDD( nDD->X(), nDD->Y(), nDD->Z() );

  gp_Vec AB( pA, pB );
  gp_Vec AC( pA, pC );
  gp_Vec normP = AB^AC;
  gp_Dir dirP( normP );

  // building plane for point projection
  gp_Pln plnP( gp_Pnt(nA->X(), nA->Y(), nA->Z()), dirP);
  TopoDS_Shape sPlnP = BRepBuilderAPI_MakeFace(plnP).Face();

  // PAAA is the result of PAA projection
  gp_Pnt pAAA = _intersect( pAA, AB, AC, sPlnP );
  gp_Pnt pBBB = _intersect( pBB, AB, AC, sPlnP );
  gp_Pnt pCCC = _intersect( pCC, AB, AC, sPlnP );
  gp_Pnt pDDD = _intersect( pDD, AB, AC, sPlnP );

  gp_Dir AA( gp_Vec(pAA, pAAA) );
  gp_Dir BB( gp_Vec(pBB, pBBB) );
  gp_Dir CC( gp_Vec(pCC, pCCC) );
  gp_Dir DD( gp_Vec(pDD, pDDD) );

  // eventually, we are able to know if the input quad is a good client!
  // exit the fonction otherwise
  if ( AA.IsOpposite(BB, HEXA_QUAD_WAY) ) return;
  if ( BB.IsOpposite(CC, HEXA_QUAD_WAY) ) return;
  if ( CC.IsOpposite(DD, HEXA_QUAD_WAY) ) return;

  // ok, give the input quad the good orientation by
  // setting 2 vertex
  if ( !dirP.IsOpposite(AA, HEXA_QUAD_WAY) ) { //OK
      v0 = qA; v1 = qB;
  } else {
      v0 = qB; v1 = qA;
  }

  if(MYDEBUG) MESSAGE("_searchInitialQuadWay() : end");
}

SMESH_Group* SMESH_HexaBlocks::_createGroup(HEXA_NS::Group& grHex)
{
  if(MYDEBUG) MESSAGE("_createGroup() : : begin   <<<<<<");

  std::string aGrName           = grHex.getName();
  HEXA_NS::EnumGroup grHexKind  = grHex.getKind();

  if(MYDEBUG) MESSAGE("aGrName"<<aGrName);

  SMDSAbs_ElementType aGrType;
  switch ( grHexKind ){
    case HEXA_NS::HexaCell   : aGrType = SMDSAbs_Volume; break;
    case HEXA_NS::QuadCell   : aGrType = SMDSAbs_Face  ; break;
    case HEXA_NS::EdgeCell   : aGrType = SMDSAbs_Edge  ; break;
    case HEXA_NS::HexaNode   : aGrType = SMDSAbs_Node  ; break;
    case HEXA_NS::QuadNode   : aGrType = SMDSAbs_Node  ; break;
    case HEXA_NS::EdgeNode   : aGrType = SMDSAbs_Node  ; break;
    case HEXA_NS::VertexNode : aGrType = SMDSAbs_Node  ; break;
    default : ASSERT(false);
  }

  int aId;
  SMESH_Group* aGr = _theMesh->AddGroup(aGrType, aGrName.c_str(), aId);

  if(MYDEBUG) MESSAGE("_createGroup() : end  >>>>>>>>");
  return aGr;
}

void SMESH_HexaBlocks::_fillGroup(HEXA_NS::Group* grHex )
{
  if(MYDEBUG) MESSAGE("_fillGroup() : : begin   <<<<<<");

  SMESH_Group* aGr = _createGroup( *grHex );
  HEXA_NS::EltBase*  grHexElt   = NULL;
  HEXA_NS::EnumGroup grHexKind  = grHex->getKind();
  int                grHexNbElt = grHex->countElement();

  if(MYDEBUG) MESSAGE("_fillGroup() : kind = " << grHexKind);
  if(MYDEBUG) MESSAGE("_fillGroup() : count= " << grHexNbElt);

  // A)Looking for elements ID
  std::vector<const SMDS_MeshElement*> aGrEltIDs;

  for ( int n=0; n<grHexNbElt; ++n ){
      grHexElt = grHex->getElement(n);

      switch ( grHexKind ){
        case HEXA_NS::HexaCell:
        {
            HEXA_NS::Hexa* h = reinterpret_cast<HEXA_NS::Hexa*>(grHexElt);
            if ( _volumesOnHexa.count(h)>0 ){
              SMESHVolumes volumes = _volumesOnHexa[h];
              for ( SMESHVolumes::iterator aVolume = volumes.begin(); aVolume != volumes.end(); ++aVolume ){
                  aGrEltIDs.push_back(*aVolume);
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF VOLUME: volume for hexa (id = "<<h->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::QuadCell:
        {
            HEXA_NS::Quad* q = reinterpret_cast<HEXA_NS::Quad*>(grHexElt);
            if ( _facesOnQuad.count(q)>0 ){
              SMESHFaces faces = _facesOnQuad[q];
              for ( SMESHFaces::iterator aFace = faces.begin(); aFace != faces.end(); ++aFace ){
                  aGrEltIDs.push_back(*aFace);
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF FACE: face for quad (id = "<<q->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::EdgeCell:
        {
            HEXA_NS::Edge* e = reinterpret_cast<HEXA_NS::Edge*>(grHexElt);
            if ( _edgesOnEdge.count(e)>0 ){
              SMESHEdges edges = _edgesOnEdge[e];
              for ( SMESHEdges::iterator anEdge = edges.begin(); anEdge != edges.end(); ++anEdge ){
                  aGrEltIDs.push_back(*anEdge);
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF Edge: edge for edge (id = "<<e->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::HexaNode:
        {
            HEXA_NS::Hexa* h = reinterpret_cast<HEXA_NS::Hexa*>(grHexElt);
            if ( _volumesOnHexa.count(h)>0 ){
              SMESHVolumes volumes = _volumesOnHexa[h];
              for ( SMESHVolumes::iterator aVolume = volumes.begin(); aVolume != volumes.end(); ++aVolume ){
                SMDS_ElemIteratorPtr aNodeIter = (*aVolume)->nodesIterator();
                while( aNodeIter->more() ){
                  const SMDS_MeshNode* aNode =
                    dynamic_cast<const SMDS_MeshNode*>( aNodeIter->next() );
                  if ( aNode ){
                      aGrEltIDs.push_back(aNode);
                  }
                }
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF HEXA NODES: nodes on hexa  (id = "<<h->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::QuadNode:
        {
            HEXA_NS::Quad* q = reinterpret_cast<HEXA_NS::Quad*>(grHexElt);
            if ( _quadNodes.count(q)>0 ){
              ArrayOfSMESHNodes nodesOnQuad = _quadNodes[q];
              for ( ArrayOfSMESHNodes::iterator nodes = nodesOnQuad.begin(); nodes != nodesOnQuad.end(); ++nodes){
                for ( SMESHNodes::iterator aNode = nodes->begin(); aNode != nodes->end(); ++aNode){
                  aGrEltIDs.push_back(*aNode);
                }
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF QUAD NODES: nodes on quad (id = "<<q->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::EdgeNode:
        {
            HEXA_NS::Edge* e = reinterpret_cast<HEXA_NS::Edge*>(grHexElt);
            if ( _nodesOnEdge.count(e)>0 ){
              SMESHNodes nodes = _nodesOnEdge[e];
              for ( SMESHNodes::iterator aNode = nodes.begin(); aNode != nodes.end(); ++aNode){
                aGrEltIDs.push_back(*aNode);
              }
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF EDGE NODES: nodes on edge (id = "<<e->getId()<<") not found");
            }
        }
        break;
        case HEXA_NS::VertexNode:
        {
          HEXA_NS::Vertex* v = reinterpret_cast<HEXA_NS::Vertex*>(grHexElt);
            if ( _node.count(v)>0 ){
              aGrEltIDs.push_back(_node[v]);
            } else {
              if(MYDEBUG) MESSAGE("GROUP OF VERTEX NODES: nodes for vertex (id = "<<v->getId()<<") not found");
            }
        }
        break;
        default : ASSERT(false);
      }
  }

  // B)Filling the group on SMESH
  SMESHDS_Group* aGroupDS = dynamic_cast<SMESHDS_Group*>( aGr->GetGroupDS() );

  for ( int i=0; i < aGrEltIDs.size(); i++ ) {
    aGroupDS->SMDSGroup().Add( aGrEltIDs[i] );
  };

  if(MYDEBUG) MESSAGE("_fillGroup() : end  >>>>>>>>");
}
