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
// File      : HEXABLOCKPlugin_HEXABLOCK.cxx
// Created   : 
// Author    : Lioka RAZAFINDRAZAKA (CEA)
// Project   : SALOME
// $Header$
//=============================================================================
//
#include "HEXABLOCKPlugin_HEXABLOCK.hxx"
#include "HEXABLOCKPlugin_Hypothesis.hxx"

#include "TopExp_Explorer.hxx"

#include <Basics_Utils.hxx>

#include "SMESHDS_Mesh.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_Mesh.hxx"
#include "SMESH_MesherHelper.hxx"
#include "SMESH_subMesh.hxx"

#include "HEXABLOCKPlugin_mesh.hxx"
 
#include "HexQuad.hxx"
#include "HexEdge.hxx"
#include "HexVertex.hxx"
#include "HexPropagation.hxx"

#include "utilities.h"

using namespace std;

#ifdef _DEBUG_
static int MYDEBUG = HEXA_NS::on_debug ();
#else
static int MYDEBUG = 0;
#endif

//=============================================================================
/*!
 *  
 */
//=============================================================================

HEXABLOCKPlugin_HEXABLOCK::HEXABLOCKPlugin_HEXABLOCK(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::HEXABLOCKPlugin_HEXABLOCK");
  _name = "HEXABLOCK_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
  _compatibleHypothesis.push_back("HEXABLOCK_Parameters");
  _requireShape = false; // can work without shape
  _requireDiscreteBoundary = false;
  _hyp = NULL;
  _supportSubmeshes = false;
  _iShape  = 0;
  _nbShape = 0;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

HEXABLOCKPlugin_HEXABLOCK::~HEXABLOCKPlugin_HEXABLOCK()
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::~HEXABLOCKPlugin_HEXABLOCK");
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool HEXABLOCKPlugin_HEXABLOCK::CheckHypothesis ( SMESH_Mesh& aMesh,
                                          const TopoDS_Shape& aShape,
                                          Hypothesis_Status&  aStatus )
{
  aStatus = SMESH_Hypothesis::HYP_OK;

  // there is only one compatible Hypothesis so far
  _hyp = 0;
  const list <const SMESHDS_Hypothesis * >& hyps = GetUsedHypothesis(aMesh, aShape);
  if ( !hyps.empty() )
    _hyp = static_cast<const HEXABLOCKPlugin_Hypothesis*> ( hyps.front() );

  return true;
}

//=============================================================================
/*!
 *Here we are going to use the HEXABLOCK mesher
 */
//=============================================================================

bool HEXABLOCKPlugin_HEXABLOCK::Compute(SMESH_Mesh& theMesh, const TopoDS_Shape& theShape) {
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute with a shape");

  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
  if ( (_iShape == 0) && (_nbShape == 0) ) {
    TopExp_Explorer expShape ( meshDS->ShapeToMesh(), TopAbs_SOLID );
    for ( ; expShape.More(); expShape.Next() ) {
      _nbShape++;
    }
  }

  // to prevent from displaying error message after computing,
  for ( int i = 0; i < _nbShape; ++i )
    if ( SMESH_subMesh* sm = theMesh.GetSubMeshContaining( theShape ))
    {
      SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(/*includeSelf=*/true,
                                                               /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        sm = smIt->next();
        if ( !sm->IsMeshComputed() )
          sm->SetIsAlwaysComputed( true );
      }
    }


  _iShape++;

  if ( _iShape == _nbShape ) {
    _nbShape = 0;
    _iShape  = 0;

    switch (_hyp->GetDimension()) {
      case 0 : return( Compute0D(theMesh) );
      case 1 : return( Compute1D(theMesh) );
      case 2 : return( Compute2D(theMesh) );
      default: return( Compute3D(theMesh) );
    };
  }
  return false;
}

//=============================================================================
/*!
 *Here we are going to use the HEXABLOCK mesher w/o geometry
 */
//=============================================================================

bool HEXABLOCKPlugin_HEXABLOCK::Compute(SMESH_Mesh& theMesh,
                                SMESH_MesherHelper* aHelper)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute without a shape");

  switch (_hyp->GetDimension()) {
    case 0 : return( Compute0D(theMesh) );
    case 1 : return( Compute1D(theMesh) );
    case 2 : return( Compute2D(theMesh) );
    default: return( Compute3D(theMesh) );
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
bool HEXABLOCKPlugin_HEXABLOCK::Evaluate(SMESH_Mesh& aMesh,
                                 const TopoDS_Shape& aShape,
                                 MapShapeNbElems& aResMap)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Evaluate: do nothing");

  return true;
}

//=============================================================================
/*!
 *  Generate hexehedral
 */
//=============================================================================

bool HEXABLOCKPlugin_HEXABLOCK::Compute3D(SMESH_Mesh& theMesh) {
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute 3D Begin");

  SMESH_HexaBlocks hexaBuilder(theMesh);

  HEXA_NS::Document* doc = _hyp->GetDocument();
  // doc->reorderFaces ();                 // 0) Abu 06/03/2012

  hexaBuilder.computeDoc(doc);
  hexaBuilder.buildGroups(doc); 

  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute 3D End");
  return true;
}

//=============================================================================
/*!
 *  Generate quadrangles
 */
//=============================================================================

bool HEXABLOCKPlugin_HEXABLOCK::Compute2D(SMESH_Mesh& theMesh)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute 2D");

  HEXA_NS::Document* doc = _hyp->GetDocument();
  // doc->reorderFaces ();                 // 0) Abu 06/03/2012

  SMESH_HexaBlocks hexaBuilder(theMesh);

  // A) Vertex computation
  int nVertex = doc->countUsedVertex();
  HEXA_NS::Vertex* vertex = NULL;
  for ( int j=0; j <nVertex; ++j ){ //Computing each vertex of the document
    vertex = doc->getUsedVertex(j);
    hexaBuilder.computeVertex(*vertex);
  };

  // B) Edges computation
  int nbPropa = 0;
  HEXA_NS::Propagation* propa = NULL;
  HEXA_NS::Law*         law   = NULL;
  HEXA_NS::Edges        edges;

  nbPropa = doc->countPropagation();
  for (int j=0; j < nbPropa; ++j ){ //Computing each edge's propagations of the document
    propa = doc->getPropagation(j);
    edges = propa->getEdges();
    law   = propa->getLaw();
    if (law == NULL){
      law = doc->getLaw(0); // default law
    };
    for( HEXA_NS::Edges::const_iterator iter = edges.begin(); iter != edges.end(); ++iter ) {
      hexaBuilder.computeEdge(**iter, *law);
    };
  };

  // C) Quad computation
  std::map<HEXA_NS::Quad*, bool>  quadWays = hexaBuilder.computeQuadWays(doc);
  int nQuad = doc->countUsedQuad();
  HEXA_NS::Quad* quad = NULL;
  for (int j=0; j <nQuad; ++j ){ //Computing each quad of the document
    quad = doc->getUsedQuad(j);
    int id = quad->getId();
    if ( quadWays.count(quad) > 0 )
      hexaBuilder.computeQuad(*quad, quadWays[quad]);
    else
      if(MYDEBUG) MESSAGE("NO QUAD WAY ID = "<<id);
  };

  // D) build Groups
  hexaBuilder.buildGroups(doc);

    return true;
}

// SMESH::SMESH_Mesh_ptr SMESH_Gen_i::HexaBlocksQuad( HEXABLOCK_ORB::Document_ptr docIn, ::CORBA::Long quadID )
// {
//   try {
//     SMESH::SMESH_Mesh_var aNewMesh = CreateEmptyMesh();
//     if ( !aNewMesh->_is_nil() ) {
//       SMESH_Mesh_i* aNewImpl    = dynamic_cast<SMESH_Mesh_i*>( GetServant( aNewMesh ).in() );
//       Document_impl* docServant = dynamic_cast<Document_impl*>( GetServant( docIn ).in() );
//       ASSERT( aNewImpl );
//       ASSERT( docServant );

//       HEXA_NS::Document* doc = docServant->GetImpl();
//       SMESH_HexaBlocks hexaBuilder(aNewImpl);

//       // A) Vertex computation
//       int nVertex = doc->countVertex();
//       HEXA_NS::Vertex* vertex = NULL;
//       for ( int j=0; j <nVertex; ++j ){ //Computing each vertex of the document
//         vertex = doc->getUsedVertex(j);
//         hexaBuilder.computeVertex(*vertex);
//       }

//       // B) Edges computation
//       int nbPropa = 0;
//       HEXA_NS::Propagation* propa = NULL;
//       HEXA_NS::Law*         law   = NULL;
//       HEXA_NS::Edges        edges;

//       nbPropa = doc->countPropagation();
//       for (int j=0; j < nbPropa; ++j ){//Computing each edge's propagations of the document
//         propa = doc->getPropagation(j);
//         edges = propa->getEdges();
//         law   = propa->getLaw();
// //         ASSERT( law );
//         if (law == NULL){
//           law = doc->getLaw(0); // default law
//         }
//         for( HEXA_NS::Edges::const_iterator iter = edges.begin();
//             iter != edges.end();
//             ++iter ){
//             hexaBuilder.computeEdge(**iter, *law);
//         }
//       }

//       // C) Quad computation
//       std::map<HEXA_NS::Quad*, bool>  quadWays = hexaBuilder.computeQuadWays(*doc);
//       int nQuad = doc->countQuad();
//       HEXA_NS::Quad* quad = NULL;
//       for (int j=0; j <nQuad; ++j ){ //Computing each quad of the document
//         quad = doc->getQuad(j);
//         int id = quad->getId();
//         if ( quadID == id and (quadWays.count(quad) > 0) )
//           hexaBuilder.computeQuad( *quad, quadWays[quad] );

//         if ( quadWays.count(quad) ==  0 )
//             if(MYDEBUG) MESSAGE("NO QUAD WAY ID = "<<id);

//       }

//       // D) build Groups
//       hexaBuilder.buildGroups(doc);

//     }
//     return aNewMesh._retn();
//   } catch (SALOME_Exception& S_ex) {
//     THROW_SALOME_CORBA_EXCEPTION(S_ex.what(), SALOME::BAD_PARAM);
//   }
// }

bool HEXABLOCKPlugin_HEXABLOCK::Compute1D(SMESH_Mesh& theMesh)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute 1D");

  HEXA_NS::Document* doc = _hyp->GetDocument();
  // doc->reorderFaces ();                 // 0) Abu 06/03/2012

  SMESH_HexaBlocks hexaBuilder(theMesh);

  // A) Vertex computation
  int nVertex = doc->countUsedVertex();
  HEXA_NS::Vertex* vertex = NULL;
  for ( int j=0; j <nVertex; ++j ){ //Computing each vertex of the document
    vertex = doc->getUsedVertex(j);
    hexaBuilder.computeVertex(*vertex);
  };

  // B) Edges computation
  int nbPropa = 0;
  HEXA_NS::Propagation* propa = NULL;
  HEXA_NS::Law*         law   = NULL;
  HEXA_NS::Edges        edges;

  nbPropa = doc->countPropagation();
  for (int j=0; j < nbPropa; ++j ){//Computing each edge's propagations of the document
    propa = doc->getPropagation(j);
    edges = propa->getEdges();
    law   = propa->getLaw();
    //         ASSERT( law );
    if (law == NULL){
      law = doc->getLaw(0); // default law
    };
    for( HEXA_NS::Edges::const_iterator iter = edges.begin(); iter != edges.end(); ++iter ) {
      hexaBuilder.computeEdge(**iter, *law);
    };
  };

  // C) build Groups
  hexaBuilder.buildGroups(doc);

    return true;
}

bool HEXABLOCKPlugin_HEXABLOCK::Compute0D(SMESH_Mesh& theMesh)
{
  if(MYDEBUG) MESSAGE("HEXABLOCKPlugin_HEXABLOCK::Compute 0D");

  HEXA_NS::Document* doc = _hyp->GetDocument();
  // doc->reorderFaces ();                 // 0) Abu 06/03/2012

  SMESH_HexaBlocks hexaBuilder(theMesh);

  // A) Vertex computation
  int nVertex = doc->countUsedVertex();
  HEXA_NS::Vertex* vertex = NULL;
  for ( int j=0; j <nVertex; ++j ){ //Computing each vertex of the document
    vertex = doc->getUsedVertex(j);
    hexaBuilder.computeVertex(*vertex);
  };

  // B) build Groups
  hexaBuilder.buildGroups(doc);

  return true;
}
