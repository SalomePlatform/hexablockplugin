// Copyright (C) 2009-2013  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

//  HEXABLOCKPlugin : C++ implementation
// File      : HEXABLOCKPlugin_Hypothesis.hxx
// Created   : Wed Apr  2 12:21:17 2008
// Author    : Lioka RAZAFINDRAZAKA (CEA)
//
#ifndef HEXABLOCKPlugin_Hypothesis_HeaderFile
#define HEXABLOCKPlugin_Hypothesis_HeaderFile

#include "HEXABLOCKPlugin_Defs.hxx"

#include <SMESH_Hypothesis.hxx>

#include "HexDocument.hxx"

#include <utilities.h>

#include <stdexcept>
#include <cstdio>

class HEXABLOCKPLUGIN_EXPORT HEXABLOCKPlugin_Hypothesis: public SMESH_Hypothesis
{
public:

  HEXABLOCKPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen * gen);

  /*!
   * Define the document to be meshed, mandatory
   */
  void SetDocument(HEXA_NS::Document* doc);
  HEXA_NS::Document* GetDocument() const;

  /*!
   * To define the hight dimension to generated: 3 = hexas, 2 = quads, 1 = segments, 0 = nodes
   */
  void SetDimension(int dim);
  int GetDimension() const;

  // Persistence
  virtual std::ostream & SaveTo(std::ostream & save);
  virtual std::istream & LoadFrom(std::istream & load);
  friend HEXABLOCKPLUGIN_EXPORT std::ostream & operator <<(std::ostream & save, HEXABLOCKPlugin_Hypothesis & hyp);
  friend HEXABLOCKPLUGIN_EXPORT std::istream & operator >>(std::istream & load, HEXABLOCKPlugin_Hypothesis & hyp);

  /*!
   * \brief Does nothing
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

  /*!
   * \brief Does nothing
   */
  virtual bool SetParametersByDefaults(const TDefaults& dflts, const SMESH_Mesh* theMesh=0);

private:
  HEXA_NS::Document* _document;
  int _dimension;
};


#endif
