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

//  HEXABLOCKPlugin GUI: GUI for plugged-in mesher HEXABLOCKPlugin
//  File   : HEXABLOCKPluginGUI_HypothesisCreator.h
//  Author : Lioka RAZAFINDRAZAKA (CEA)
//  Module : HEXABLOCKPlugin
//
#ifndef HEXABLOCKPLUGINGUI_HypothesisCreator_HeaderFile
#define HEXABLOCKPLUGINGUI_HypothesisCreator_HeaderFile

#ifdef WIN32
  #if defined HEXABLOCKPLUGINGUI_EXPORTS || defined HEXABLOCKPluginGUI_EXPORTS
    #define HEXABLOCKPLUGINGUI_EXPORT __declspec( dllexport )
  #else
    #define HEXABLOCKPLUGINGUI_EXPORT __declspec( dllimport )
  #endif
#else
  #define HEXABLOCKPLUGINGUI_EXPORT
#endif

#include <SMESHGUI_Hypotheses.h>
// #include <SalomeApp_DoubleSpinBox.h>

#include <QItemDelegate>
#include <map>
#include <vector>
#include CORBA_SERVER_HEADER(HEXABLOCKPlugin_Algorithm)

class QWidget;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QStandardItemModel;
class QTableView;
class QHeaderView;
class QDoubleSpinBox;

class LightApp_SelectionMgr;

typedef std::vector<double> HEXABLOCKEnforcedVertex;
typedef std::vector<HEXABLOCKEnforcedVertex> TEnforcedVertexValues;

typedef struct
{
  bool    myToMeshHoles,myKeepFiles,myToCreateNewNodes,myBoundaryRecovery,myFEMCorrection,myRemoveInitialCentralPoint;
  int     myMaximumMemory,myInitialMemory,myOptimizationLevel;
  QString myName,myWorkingDir,myTextOption;
  short   myVerboseLevel;
  TEnforcedVertexValues myEnforcedVertices;
} HEXABLOCKHypothesisData;

/*!
  \brief Class for creation of HEXABLOCK2D and HEXABLOCK3D hypotheses
*/
class HEXABLOCKPLUGINGUI_EXPORT HEXABLOCKPluginGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  HEXABLOCKPluginGUI_HypothesisCreator( const QString& );
  virtual ~HEXABLOCKPluginGUI_HypothesisCreator();

  virtual bool     checkParams(QString& msg) const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual QString  storeParams   () const;

  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

protected slots:
  void                onDirBtnClicked();
  void                updateWidgets();
  void                onVertexBtnClicked();
  void                onRemoveVertexBtnClicked();
  bool                checkVertexIsDefined();

signals:
  void                vertexDefined(bool);

private:
  bool                readParamsFromHypo( HEXABLOCKHypothesisData& ) const;
  bool                readParamsFromWidgets( HEXABLOCKHypothesisData& ) const;
  bool                storeParamsToHypo( const HEXABLOCKHypothesisData& ) const;
  bool                smpVertexExists(double, double, double) const;

private:
  QWidget*            myStdGroup;
  QLineEdit*          myName;
  QCheckBox*          myToMeshHolesCheck;
  QComboBox*          myOptimizationLevelCombo;

  QWidget*            myAdvGroup;
  QCheckBox*          myMaximumMemoryCheck;
  QSpinBox*           myMaximumMemorySpin;
  QCheckBox*          myInitialMemoryCheck;
  QSpinBox*           myInitialMemorySpin;
  QLineEdit*          myWorkingDir;
  QCheckBox*          myKeepFiles;
  QSpinBox*           myVerboseLevelSpin;
  QCheckBox*          myToCreateNewNodesCheck;
  QCheckBox*          myRemoveInitialCentralPointCheck;
  QCheckBox*          myBoundaryRecoveryCheck;
  QCheckBox*          myFEMCorrectionCheck;
QLineEdit*            myTextOption;
  
  QWidget*            myEnfGroup;
  QStandardItemModel* mySmpModel;
  QTableView*         myEnforcedTableView;
  QLineEdit*          myXCoord;
  QLineEdit*          myYCoord;
  QLineEdit*          myZCoord;
  QLineEdit*          mySizeValue;
  QPushButton*        addVertexButton;
  QPushButton*        removeVertexButton;
  
  LightApp_SelectionMgr*  mySelectionMgr;          /* User shape selection */
//   SVTK_Selector*          mySelector;
};

class DoubleLineEditDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DoubleLineEditDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
