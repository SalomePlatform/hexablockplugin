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

//  HEXABLOCKPlugin : C++ implementation
// File      : HEXABLOCKPlugin_Hypothesis_i.hxx
// Date      : 2010/11/08
// Project   : SALOME
//
#ifndef _HEXABLOCKPlugin_Hypothesis_i_HXX_
#define _HEXABLOCKPlugin_Hypothesis_i_HXX_

#include "HEXABLOCKPlugin_Defs.hxx"

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HEXABLOCKPlugin_Algorithm)
//   #include CORBA_CLIENT_HEADER(Document)

#include "SMESH_Hypothesis_i.hxx"
#include "HEXABLOCKPlugin_Hypothesis.hxx"

class SMESH_Gen;

// HEXABLOCKPlugin parameters hypothesis

class HEXABLOCKPLUGINENGINE_EXPORT HEXABLOCKPlugin_Hypothesis_i:
  public virtual POA_HEXABLOCKPlugin::HEXABLOCKPlugin_Hypothesis,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  HEXABLOCKPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                            int                     theStudyId,
                            ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~HEXABLOCKPlugin_Hypothesis_i();

    /*!
     * Define the document to be meshed, mandatory
     */
    char* GetDocument ();
    void  SetDocument (const char* doc);

    /*!
     * To define the hight dimension to generated: 3 = hexas, 2 = quads, 1 = segments, 0 = nodes
     */
    CORBA::Long GetDimension();
    void SetDimension(CORBA::Long dim);

    // Get implementation
    ::HEXABLOCKPlugin_Hypothesis* GetImpl();
  
    // Verify whether hypothesis supports given entity type 
    CORBA::Boolean IsDimSupported( SMESH::Dimension type );

 private:
    PortableServer::POA_var _poa;// POA reference

};

#endif
