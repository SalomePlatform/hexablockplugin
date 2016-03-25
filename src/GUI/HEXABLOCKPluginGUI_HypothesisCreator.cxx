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
//  File   : HEXABLOCKPluginGUI_HypothesisCreator.cxx
//  Author : Lioka RAZAFINDRAZAKA (CEA)
//  Module : HEXABLOCKPlugin
//  $Header: 
//
#include "HEXABLOCKPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include <SUIT_Session.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_FileDlg.h>
#include <SalomeApp_Tools.h>
#include <SalomeApp_TypeFilter.h>

#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QFileInfo>

#include <QTableWidget>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>
#include <QModelIndexList>

#include <stdexcept>
#include <utilities.h>

#ifdef _DEBUG_
static int MYDEBUG = 1;
#else
static int MYDEBUG = 0;
#endif


// tabs
enum {
  STD_TAB = 0,
  ADV_TAB,
  ENF_VER_TAB
};

// Enforced vertices array columns
enum {
  ENF_VER_X_COLUMN = 0,
  ENF_VER_Y_COLUMN,
  ENF_VER_Z_COLUMN,
  ENF_VER_SIZE_COLUMN,
  ENF_VER_NB_COLUMNS
};

// Enforced vertices inputs
enum {
  ENF_VER_BTNS = 0,
  ENF_VER_X_COORD,
  ENF_VER_Y_COORD,
  ENF_VER_Z_COORD,
  ENF_VER_SIZE,
  ENF_VER_VERTEX_BTN,
  ENF_VER_SEPARATOR,
  ENF_VER_REMOVE_BTN,
};

namespace {

#ifdef WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif

  int maxAvailableMemory()
  {
#ifdef WIN32
    // See http://msdn.microsoft.com/en-us/library/aa366589.aspx
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    int err = GlobalMemoryStatusEx (&statex);
    if (err != 0) {
      int totMB = 
        statex.ullTotalPhys / 1024 / 1024 +
        statex.ullTotalPageFile / 1024 / 1024 +
        statex.ullTotalVirtual / 1024 / 1024;
      return (int) ( 0.7 * totMB );
    }
#else
    struct sysinfo si;
    int err = sysinfo( &si );
    if ( err == 0 ) {
      int totMB =
        si.totalram * si.mem_unit / 1024 / 1024 +
        si.totalswap * si.mem_unit / 1024 / 1024 ;
      return (int) ( 0.7 * totMB );
    }
#endif
    return 0;
  }
}

class QDoubleValidator;

//
// BEGIN DoubleLineEditDelegate
//

DoubleLineEditDelegate::DoubleLineEditDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *DoubleLineEditDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);
    editor->setValidator(new QDoubleValidator(parent));

    return editor;
}

void DoubleLineEditDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);
}

void DoubleLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool ok;
    double value = lineEdit->text().toDouble(&ok);

    if (ok) {
        model->setData(index, value, Qt::EditRole);
        if(MYDEBUG) MESSAGE("Value " << value << " was set at index(" << index.row() << "," << index.column() << ")");
    }
}

void DoubleLineEditDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

//
// END DoubleLineEditDelegate
//

HEXABLOCKPluginGUI_HypothesisCreator::HEXABLOCKPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType )
{
}

HEXABLOCKPluginGUI_HypothesisCreator::~HEXABLOCKPluginGUI_HypothesisCreator()
{
}

QFrame* HEXABLOCKPluginGUI_HypothesisCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0 );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 5 );
  lay->setSpacing( 0 );

  // tab
  QTabWidget* tab = new QTabWidget( fr );
  tab->setTabShape( QTabWidget::Rounded );
  tab->setTabPosition( QTabWidget::North );
  lay->addWidget( tab );

  // basic parameters
  myStdGroup = new QWidget();
  QGridLayout* aStdLayout = new QGridLayout( myStdGroup );
  aStdLayout->setSpacing( 6 );
  aStdLayout->setMargin( 11 );

  int row = 0;
  myName = 0;
  if( isCreation() )
  {
    aStdLayout->addWidget( new QLabel( tr( "SMESH_NAME" ), myStdGroup ), row, 0, 1, 1 );
    myName = new QLineEdit( myStdGroup );
    aStdLayout->addWidget( myName, row++, 1, 1, 1 );
  }

  myToMeshHolesCheck = new QCheckBox( tr( "HEXABLOCK_TO_MESH_HOLES" ), myStdGroup );
  aStdLayout->addWidget( myToMeshHolesCheck, row++, 0, 1, 2 );

  aStdLayout->addWidget( new QLabel( tr( "HEXABLOCK_OPTIMIZATIOL_LEVEL" ), myStdGroup ), row, 0 );
  myOptimizationLevelCombo = new QComboBox( myStdGroup );
  aStdLayout->addWidget( myOptimizationLevelCombo, row++, 1, 1, 1 );

  QStringList types;
  types << tr( "LEVEL_NONE" ) << tr( "LEVEL_LIGHT" ) << tr( "LEVEL_MEDIUM" ) << tr( "LEVEL_STANDARDPLUS" ) << tr( "LEVEL_STRONG" );
  myOptimizationLevelCombo->addItems( types );

  aStdLayout->setRowStretch( row, 5 );

  // advanced parameters
  myAdvGroup = new QWidget();
  QGridLayout* anAdvLayout = new QGridLayout( myAdvGroup );
  anAdvLayout->setSpacing( 6 );
  anAdvLayout->setMargin( 11 );
  
  myMaximumMemoryCheck = new QCheckBox( tr( "MAX_MEMORY_SIZE" ), myAdvGroup );
  myMaximumMemorySpin = new QSpinBox( myAdvGroup );
  myMaximumMemorySpin->setMinimum( 1 );
  myMaximumMemorySpin->setMaximum( maxAvailableMemory() );
  myMaximumMemorySpin->setSingleStep( 10 );
  QLabel* aMegabyteLabel = new QLabel( tr( "MEGABYTE" ), myAdvGroup );

  myInitialMemoryCheck = new QCheckBox( tr( "INIT_MEMORY_SIZE" ), myAdvGroup );
  myInitialMemorySpin = new QSpinBox( myAdvGroup );
  myInitialMemorySpin->setMinimum( 1 );
  myInitialMemorySpin->setMaximum( maxAvailableMemory() );
  myInitialMemorySpin->setSingleStep( 10 );
  QLabel* aMegabyteLabel2 = new QLabel( tr( "MEGABYTE" ), myAdvGroup );

  QLabel* aWorkinDirLabel = new QLabel( tr( "WORKING_DIR" ), myAdvGroup );
  myWorkingDir = new QLineEdit( myAdvGroup );
  //myWorkingDir->setReadOnly( true );
  QPushButton* dirBtn = new QPushButton( tr( "SELECT_DIR" ), myAdvGroup );
  dirBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  
  myKeepFiles = new QCheckBox( tr( "KEEP_WORKING_FILES" ), myAdvGroup );

  QLabel* aVerboseLevelLabel = new QLabel( tr( "VERBOSE_LEVEL" ), myAdvGroup );
  myVerboseLevelSpin = new QSpinBox( myAdvGroup );
  myVerboseLevelSpin->setMinimum( 0 );
  myVerboseLevelSpin->setMaximum( 10 );
  myVerboseLevelSpin->setSingleStep( 1 );

  myToCreateNewNodesCheck = new QCheckBox( tr( "TO_ADD_NODES" ), myAdvGroup );
  
  myRemoveInitialCentralPointCheck = new QCheckBox( tr( "NO_INITIAL_CENTRAL_POINT" ), myAdvGroup );
  
  myBoundaryRecoveryCheck = new QCheckBox( tr( "RECOVERY_VERSION" ), myAdvGroup );
  
  myFEMCorrectionCheck = new QCheckBox( tr( "FEM_CORRECTION" ), myAdvGroup );

  QLabel* aTextOptionLabel = new QLabel( tr( "TEXT_OPTION" ), myAdvGroup );
  myTextOption = new QLineEdit( myAdvGroup );

  anAdvLayout->addWidget( myMaximumMemoryCheck,             0, 0, 1, 1 );
  anAdvLayout->addWidget( myMaximumMemorySpin,              0, 1, 1, 1 );
  anAdvLayout->addWidget( aMegabyteLabel,                   0, 2, 1, 1 );
  anAdvLayout->addWidget( myInitialMemoryCheck,             1, 0, 1, 1 );
  anAdvLayout->addWidget( myInitialMemorySpin,              1, 1, 1, 1 );
  anAdvLayout->addWidget( aMegabyteLabel2,                  1, 2, 1, 1 );
  anAdvLayout->addWidget( aWorkinDirLabel,                  2, 0, 1, 1 );
  anAdvLayout->addWidget( myWorkingDir,                     2, 1, 1, 2 );
  anAdvLayout->addWidget( dirBtn,                           2, 3, 1, 1 );
  anAdvLayout->addWidget( myKeepFiles,                      3, 0, 1, 4 );
  anAdvLayout->addWidget( aVerboseLevelLabel,               4, 0, 1, 1 );
  anAdvLayout->addWidget( myVerboseLevelSpin,               4, 1, 1, 1 );
  anAdvLayout->addWidget( myToCreateNewNodesCheck,          5, 0, 1, 4 );
  anAdvLayout->addWidget( myRemoveInitialCentralPointCheck, 6, 0, 1, 4 );
  anAdvLayout->addWidget( myBoundaryRecoveryCheck,          7, 0, 1, 4 );
  anAdvLayout->addWidget( myFEMCorrectionCheck,             8, 0, 1, 4 );
  anAdvLayout->addWidget( aTextOptionLabel,                 9, 0, 1, 1 );
  anAdvLayout->addWidget( myTextOption,                     9, 1, 1, 2 );

  // Size Maps parameters
  myEnfGroup = new QWidget();
  QGridLayout* anSmpLayout = new QGridLayout(myEnfGroup);
  
  mySmpModel = new QStandardItemModel(0, ENF_VER_NB_COLUMNS);
  myEnforcedTableView = new QTableView(myEnfGroup);
  myEnforcedTableView->setModel(mySmpModel);
  myEnforcedTableView->setSortingEnabled(true);
  myEnforcedTableView->setItemDelegateForColumn(ENF_VER_SIZE_COLUMN,new DoubleLineEditDelegate(this));
  anSmpLayout->addWidget(myEnforcedTableView, 1, 0, 9, 1);
  QStringList enforcedHeaders;
  enforcedHeaders << tr( "HEXABLOCK_ENF_VER_X_COLUMN" )<< tr( "HEXABLOCK_ENF_VER_Y_COLUMN" ) << tr( "HEXABLOCK_ENF_VER_Z_COLUMN" ) << tr( "HEXABLOCK_ENF_VER_SIZE_COLUMN" ); 
  mySmpModel->setHorizontalHeaderLabels(enforcedHeaders);
  myEnforcedTableView->setAlternatingRowColors(true);
  myEnforcedTableView->verticalHeader()->hide();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myEnforcedTableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#else
  myEnforcedTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#endif
  QLabel* myXCoordLabel = new QLabel( tr( "HEXABLOCK_ENF_VER_X_LABEL" ), myEnfGroup );
  anSmpLayout->addWidget(myXCoordLabel, ENF_VER_X_COORD, 1, 1, 1);
  myXCoord = new QLineEdit(myEnfGroup);
  myXCoord->setValidator(new QDoubleValidator(myEnfGroup));
  anSmpLayout->addWidget(myXCoord, ENF_VER_X_COORD, 2, 1, 1);
  QLabel* myYCoordLabel = new QLabel( tr( "HEXABLOCK_ENF_VER_Y_LABEL" ), myEnfGroup );
  anSmpLayout->addWidget(myYCoordLabel, ENF_VER_Y_COORD, 1, 1, 1);
  myYCoord = new QLineEdit(myEnfGroup);
  myYCoord->setValidator(new QDoubleValidator(myEnfGroup));
  anSmpLayout->addWidget(myYCoord, ENF_VER_Y_COORD, 2, 1, 1);
  QLabel* myZCoordLabel = new QLabel( tr( "HEXABLOCK_ENF_VER_Z_LABEL" ), myEnfGroup );
  anSmpLayout->addWidget(myZCoordLabel, ENF_VER_Z_COORD, 1, 1, 1);
  myZCoord = new QLineEdit(myEnfGroup);
  myZCoord->setValidator(new QDoubleValidator(myEnfGroup));
  anSmpLayout->addWidget(myZCoord, ENF_VER_Z_COORD, 2, 1, 1);
  QLabel* mySizeLabel = new QLabel( tr( "HEXABLOCK_ENF_VER_SIZE_LABEL" ), myEnfGroup );
  anSmpLayout->addWidget(mySizeLabel, ENF_VER_SIZE, 1, 1, 1);
  mySizeValue = new QLineEdit(myEnfGroup);
  mySizeValue->setValidator(new QDoubleValidator(myEnfGroup));
  anSmpLayout->addWidget(mySizeValue, ENF_VER_SIZE, 2, 1, 1);

  addVertexButton = new QPushButton(tr("HEXABLOCK_ENF_VER_VERTEX"),myEnfGroup);
  anSmpLayout->addWidget(addVertexButton, ENF_VER_VERTEX_BTN, 1, 1, 2);
  addVertexButton->setEnabled(false);

  QFrame *line = new QFrame(myEnfGroup);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  anSmpLayout->addWidget(line, ENF_VER_SEPARATOR, 1, 1, 2);

  removeVertexButton = new QPushButton(tr("HEXABLOCK_ENF_VER_REMOVE"),myEnfGroup);
  anSmpLayout->addWidget(removeVertexButton, ENF_VER_REMOVE_BTN, 1, 1, 2);
          
  // add tabs
  tab->insertTab( STD_TAB, myStdGroup, tr( "SMESH_ARGUMENTS" ) );
  tab->insertTab( ADV_TAB, myAdvGroup, tr( "HEXABLOCK_ADV_ARGS" ) );
  tab->insertTab( ENF_VER_TAB, myEnfGroup, tr( "HEXABLOCK_ENFORCED_VERTICES" ) );
  tab->setCurrentIndex( STD_TAB );

  // connections
  connect( myMaximumMemoryCheck,    SIGNAL( toggled( bool ) ), this, SLOT( updateWidgets() ) );
  connect( myInitialMemoryCheck,    SIGNAL( toggled( bool ) ), this, SLOT( updateWidgets() ) );
  connect( myBoundaryRecoveryCheck, SIGNAL( toggled( bool ) ), this, SLOT( updateWidgets() ) );
  connect( dirBtn,                  SIGNAL( clicked() ),       this, SLOT( onDirBtnClicked() ) );
  connect( myXCoord,                SIGNAL( textChanged(const QString&) ),   this, SLOT( checkVertexIsDefined() ) );
  connect( myYCoord,                SIGNAL( textChanged(const QString&) ),   this, SLOT( checkVertexIsDefined() ) );
  connect( myZCoord,                SIGNAL( textChanged(const QString&) ),   this, SLOT( checkVertexIsDefined() ) );
  connect( mySizeValue,             SIGNAL( textChanged(const QString&) ),   this, SLOT( checkVertexIsDefined() ) );
  connect( this,                    SIGNAL( vertexDefined(bool) ), addVertexButton, SLOT( setEnabled(bool) ) );
  connect( addVertexButton,         SIGNAL( clicked() ),       this, SLOT( onVertexBtnClicked() ) );
  connect( removeVertexButton,      SIGNAL( clicked() ),       this, SLOT( onRemoveVertexBtnClicked() ) );
  

  
  return fr;
}

bool HEXABLOCKPluginGUI_HypothesisCreator::smpVertexExists(double x, double y, double z) const
{
    const int rowCount = mySmpModel->rowCount();
    for (int i=0 ; i < rowCount ; i++) {
      double myX = mySmpModel->data(mySmpModel->index(i, ENF_VER_X_COLUMN)).toDouble();
      if (myX == x) {
//         MESSAGE("Found x value " << x << " at row " << i);
        double myY = mySmpModel->data(mySmpModel->index(i, ENF_VER_Y_COLUMN)).toDouble();
        if (myY == y) {
//           MESSAGE("Found y value " << y << " at row " << i);
          double myZ = mySmpModel->data(mySmpModel->index(i, ENF_VER_Z_COLUMN)).toDouble();
          if (myZ == z) {
            if (MYDEBUG){
              MESSAGE("Found x value " << x << " at row " << i);
              MESSAGE("Found y value " << y << " at row " << i);
              MESSAGE("Found z value " << z << " at row " << i);
            }
            return true;
          }
        }
      }
    }
//     MESSAGE("Not found x,y,z values: " << x << " " << y << " " << z);
    return false;
}

bool HEXABLOCKPluginGUI_HypothesisCreator::checkVertexIsDefined()
{
  bool val = (!myXCoord->text().isEmpty())&&(!myYCoord->text().isEmpty())&&(!myZCoord->text().isEmpty())&&(!mySizeValue->text().isEmpty());
  bool isDefined = val;
  if (val)
    isDefined = ! smpVertexExists(myXCoord->text().toDouble(),myYCoord->text().toDouble(),myZCoord->text().toDouble());

  emit vertexDefined(isDefined);
  return isDefined;
}

void HEXABLOCKPluginGUI_HypothesisCreator::onVertexBtnClicked()
{
    if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::onVertexBtnClicked()");
    const int row = mySmpModel->rowCount() ;
    double x = myXCoord->text().toDouble();
    double y = myYCoord->text().toDouble();
    double z = myZCoord->text().toDouble();
    double size = mySizeValue->text().toDouble();
    
    if (smpVertexExists(x,y,z)) return;
    
//     double size = 10.0;
    // ENF_VER_X_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_X_COLUMN),x);
    mySmpModel->setItem( row, ENF_VER_X_COLUMN, new QStandardItem(QString::number(x)) );
    mySmpModel->item( row, ENF_VER_X_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_Y_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_Y_COLUMN),y);
    mySmpModel->setItem( row, ENF_VER_Y_COLUMN, new QStandardItem(QString::number(y)) );
    mySmpModel->item( row, ENF_VER_Y_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_Z_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_Z_COLUMN),z);
    mySmpModel->setItem( row, ENF_VER_Z_COLUMN, new QStandardItem(QString::number(z)) );
    mySmpModel->item( row, ENF_VER_Z_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_SIZE_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_SIZE_COLUMN),size);
    mySmpModel->setItem( row, ENF_VER_SIZE_COLUMN, new QStandardItem(QString::number(size,'f')) );

    myEnforcedTableView->clearSelection();
    myEnforcedTableView->scrollTo( mySmpModel->item( row, ENF_VER_SIZE_COLUMN )->index() );
    checkVertexIsDefined();
}

void HEXABLOCKPluginGUI_HypothesisCreator::onRemoveVertexBtnClicked()
{
    QList<int> selectedRows;
    QList<QModelIndex> selectedIndex = myEnforcedTableView->selectionModel()->selectedIndexes();
    int row;
    QModelIndex index;
    foreach( index, selectedIndex ) {
        row = index.row();
        if ( !selectedRows.contains( row ) ) 
        selectedRows.append( row );
    }
    qSort( selectedRows );
    QListIterator<int> it( selectedRows );
    it.toBack();
    while ( it.hasPrevious() ) {
        row = it.previous();
        if (MYDEBUG) MESSAGE("delete row #"<< row);
        mySmpModel->removeRow(row );
    }
    myEnforcedTableView->clearSelection();
}
void HEXABLOCKPluginGUI_HypothesisCreator::onDirBtnClicked()
{
  QString dir = SUIT_FileDlg::getExistingDirectory( dlg(), myWorkingDir->text(), QString() );
  if ( !dir.isEmpty() )
    myWorkingDir->setText( dir );
}

void HEXABLOCKPluginGUI_HypothesisCreator::updateWidgets()
{
  myMaximumMemorySpin->setEnabled( myMaximumMemoryCheck->isChecked() );
  myInitialMemoryCheck->setEnabled( !myBoundaryRecoveryCheck->isChecked() );
  myInitialMemorySpin->setEnabled( myInitialMemoryCheck->isChecked() && !myBoundaryRecoveryCheck->isChecked() );
  myOptimizationLevelCombo->setEnabled( !myBoundaryRecoveryCheck->isChecked() );
}

bool HEXABLOCKPluginGUI_HypothesisCreator::checkParams(QString& msg) const
{
  if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::checkParams");

  if ( !QFileInfo( myWorkingDir->text().trimmed() ).isWritable() ) {
    SUIT_MessageBox::warning( dlg(),
                              tr( "SMESH_WRN_WARNING" ),
                              tr( "HEXABLOCK_PERMISSION_DENIED" ) );
    return false;
  }

  return true;
}

void HEXABLOCKPluginGUI_HypothesisCreator::retrieveParams() const
{
  if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::retrieveParams");
  HEXABLOCKHypothesisData data;
  readParamsFromHypo( data );

  if ( myName )
    myName->setText( data.myName );
  
  myToMeshHolesCheck               ->setChecked    ( data.myToMeshHoles );
  myOptimizationLevelCombo         ->setCurrentIndex( data.myOptimizationLevel );
  myMaximumMemoryCheck             ->setChecked    ( data.myMaximumMemory > 0 );
  myMaximumMemorySpin              ->setValue      ( qMax( data.myMaximumMemory,
                                                           myMaximumMemorySpin->minimum() ));
  myInitialMemoryCheck             ->setChecked    ( data.myInitialMemory > 0 );
  myInitialMemorySpin              ->setValue      ( qMax( data.myInitialMemory,
                                                           myInitialMemorySpin->minimum() ));
  myWorkingDir                     ->setText       ( data.myWorkingDir );
  myKeepFiles                      ->setChecked    ( data.myKeepFiles );
  myVerboseLevelSpin               ->setValue      ( data.myVerboseLevel );
  myToCreateNewNodesCheck          ->setChecked    ( data.myToCreateNewNodes );
  myRemoveInitialCentralPointCheck ->setChecked    ( data.myRemoveInitialCentralPoint );
  myBoundaryRecoveryCheck          ->setChecked    ( data.myBoundaryRecovery );
  myFEMCorrectionCheck             ->setChecked    ( data.myFEMCorrection );
  myTextOption                     ->setText       ( data.myTextOption );

  TEnforcedVertexValues::const_iterator it;
  int row = 0;
  for(it = data.myEnforcedVertices.begin() ; it != data.myEnforcedVertices.end(); it++ )
  {
    double x = it->at(0);
    double y = it->at(1);
    double z = it->at(2);
    double size = it->at(3);
    // ENF_VER_X_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_X_COLUMN),x);
    mySmpModel->setItem( row, ENF_VER_X_COLUMN, new QStandardItem(QString::number(x)) );
    mySmpModel->item( row, ENF_VER_X_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_Y_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_Y_COLUMN),y);
    mySmpModel->setItem( row, ENF_VER_Y_COLUMN, new QStandardItem(QString::number(y)) );
    mySmpModel->item( row, ENF_VER_Y_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_Z_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_Z_COLUMN),z);
    mySmpModel->setItem( row, ENF_VER_Z_COLUMN, new QStandardItem(QString::number(z)) );
    mySmpModel->item( row, ENF_VER_Z_COLUMN )->setFlags( Qt::ItemIsSelectable );
    // ENF_VER_SIZE_COLUMN
    mySmpModel->setData(mySmpModel->index(row, ENF_VER_SIZE_COLUMN),size);
    mySmpModel->setItem( row, ENF_VER_SIZE_COLUMN, new QStandardItem(QString::number(size)) );

    if (MYDEBUG) MESSAGE("Row " << row << ": (" << x << ","<< y << ","<< z << ") ="<< size);
    row++;
  }
  
  HEXABLOCKPluginGUI_HypothesisCreator* that = (HEXABLOCKPluginGUI_HypothesisCreator*)this;
  that->updateWidgets();
}

QString HEXABLOCKPluginGUI_HypothesisCreator::storeParams() const
{
    if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::storeParams");
    HEXABLOCKHypothesisData data;
    readParamsFromWidgets( data );
    storeParamsToHypo( data );
    
    QString valStr = "";
    
    if ( !data.myBoundaryRecovery )
        valStr = "-c " + QString::number( !data.myToMeshHoles );
    
    if ( data.myOptimizationLevel >= 0 && data.myOptimizationLevel < 5 && !data.myBoundaryRecovery) {
        const char* level[] = { "none" , "light" , "standard" , "standard+" , "strong" };
        valStr += " -o ";
        valStr += level[ data.myOptimizationLevel ];
    }
    if ( data.myMaximumMemory > 0 ) {
        valStr += " -m ";
        valStr += QString::number( data.myMaximumMemory );
    }
    if ( data.myInitialMemory > 0 && !data.myBoundaryRecovery ) {
        valStr += " -M ";
        valStr += QString::number( data.myInitialMemory );
    }
    valStr += " -v ";
    valStr += QString::number( data.myVerboseLevel );
    
    if ( !data.myToCreateNewNodes )
        valStr += " -p0";
    
    if ( data.myRemoveInitialCentralPoint )
        valStr += " -no_initial_central_point";
    
    if ( data.myBoundaryRecovery )
        valStr += " -C";
    
    if ( data.myFEMCorrection )
        valStr += " -FEM";
    
    valStr += " ";
    valStr += data.myTextOption;
    
    valStr += " #BEGIN ENFORCED VERTICES#";
    // Add size map parameters storage
    for (int i=0 ; i<mySmpModel->rowCount() ; i++) {
        valStr += " (";
        double x = mySmpModel->data(mySmpModel->index(i,ENF_VER_X_COLUMN)).toDouble();
        double y = mySmpModel->data(mySmpModel->index(i,ENF_VER_Y_COLUMN)).toDouble();
        double z = mySmpModel->data(mySmpModel->index(i,ENF_VER_Z_COLUMN)).toDouble();
        double size = mySmpModel->data(mySmpModel->index(i,ENF_VER_SIZE_COLUMN)).toDouble();
        valStr += QString::number( x );
        valStr += ",";
        valStr += QString::number( y );
        valStr += ",";
        valStr += QString::number( z );
        valStr += ")=";
        valStr += QString::number( size );
        if (i!=mySmpModel->rowCount()-1)
            valStr += ";";
    }
    valStr += " #END ENFORCED VERTICES#";
    if (MYDEBUG) MESSAGE(valStr.toStdString());
  return valStr;
}

bool HEXABLOCKPluginGUI_HypothesisCreator::readParamsFromHypo( HEXABLOCKHypothesisData& h_data ) const
{
  if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::readParamsFromHypo");
  HEXABLOCKPlugin::HEXABLOCKPlugin_Hypothesis_var h =
    HEXABLOCKPlugin::HEXABLOCKPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName = isCreation() && data ? hypName() : "";

  /* fkl to update:
  h_data.myToMeshHoles                = h->GetToMeshHoles();
  h_data.myMaximumMemory              = h->GetMaximumMemory();
  h_data.myInitialMemory              = h->GetInitialMemory();
  h_data.myInitialMemory              = h->GetInitialMemory();
  h_data.myOptimizationLevel          = h->GetOptimizationLevel();
  h_data.myKeepFiles                  = h->GetKeepFiles();
  h_data.myWorkingDir                 = h->GetWorkingDirectory();
  h_data.myVerboseLevel               = h->GetVerboseLevel();
  h_data.myToCreateNewNodes           = h->GetToCreateNewNodes();
  h_data.myRemoveInitialCentralPoint  = h->GetToRemoveCentralPoint();
  h_data.myBoundaryRecovery           = h->GetToUseBoundaryRecoveryVersion();
  h_data.myFEMCorrection              = h->GetFEMCorrection();
  h_data.myTextOption                 = h->GetTextOption();

  HEXABLOCKPlugin::HEXABLOCKEnforcedVertexList_var vertices = h->GetEnforcedVertices();
  MESSAGE("vertices->length(): " << vertices->length());
  h_data.myEnforcedVertices.clear();
  for (int i=0 ; i<vertices->length() ; i++) {
    HEXABLOCKEnforcedVertex myVertex;
    myVertex.push_back(vertices[i].x);
    myVertex.push_back(vertices[i].y);
    myVertex.push_back(vertices[i].z);
    myVertex.push_back(vertices[i].size);
    MESSAGE("Add enforced vertex ("<< myVertex[0] << ","<< myVertex[1] << ","<< myVertex[2] << ") ="<< myVertex[3]);
    h_data.myEnforcedVertices.push_back(myVertex);
  }
  */

  return true;
}

bool HEXABLOCKPluginGUI_HypothesisCreator::storeParamsToHypo( const HEXABLOCKHypothesisData& h_data ) const
{
  if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::storeParamsToHypo");
  HEXABLOCKPlugin::HEXABLOCKPlugin_Hypothesis_var h =
    HEXABLOCKPlugin::HEXABLOCKPlugin_Hypothesis::_narrow( hypothesis() );

  bool ok = true;
  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().constData() );

    /* fkl to update:
    if ( h->GetToMeshHoles() != h_data.myToMeshHoles ) // avoid duplication of DumpPython commands
      h->SetToMeshHoles      ( h_data.myToMeshHoles       );
    if ( h->GetMaximumMemory() != h_data.myMaximumMemory )
      h->SetMaximumMemory    ( h_data.myMaximumMemory     );
    if ( h->GetInitialMemory() != h_data.myInitialMemory )
      h->SetInitialMemory    ( h_data.myInitialMemory     );
    if ( h->GetInitialMemory() != h_data.myInitialMemory )
      h->SetInitialMemory    ( h_data.myInitialMemory     );
    if ( h->GetOptimizationLevel() != h_data.myOptimizationLevel )
      h->SetOptimizationLevel( h_data.myOptimizationLevel );
    if ( h->GetKeepFiles() != h_data.myKeepFiles )
      h->SetKeepFiles        ( h_data.myKeepFiles         );
    if ( h->GetWorkingDirectory() != h_data.myWorkingDir )
      h->SetWorkingDirectory ( h_data.myWorkingDir.toLatin1().constData() );
    if ( h->GetVerboseLevel() != h_data.myVerboseLevel )
      h->SetVerboseLevel     ( h_data.myVerboseLevel );
    if ( h->GetToCreateNewNodes() != h_data.myToCreateNewNodes )
      h->SetToCreateNewNodes( h_data.myToCreateNewNodes );
    if ( h->GetToRemoveCentralPoint() != h_data.myRemoveInitialCentralPoint )
      h->SetToRemoveCentralPoint( h_data.myRemoveInitialCentralPoint );
    if ( h->GetToUseBoundaryRecoveryVersion() != h_data.myBoundaryRecovery )
      h->SetToUseBoundaryRecoveryVersion( h_data.myBoundaryRecovery );
    if ( h->GetFEMCorrection() != h_data.myFEMCorrection )
      h->SetFEMCorrection( h_data.myFEMCorrection );
    if ( h->GetTextOption() != h_data.myTextOption )
      h->SetTextOption       ( h_data.myTextOption.toLatin1().constData() );
    
    int nbVertex = (int) h_data.myEnforcedVertices.size();
    HEXABLOCKPlugin::HEXABLOCKEnforcedVertexList_var vertexHyp = h->GetEnforcedVertices();
    int nbVertexHyp = vertexHyp->length();
    
    MESSAGE("Store params for size maps: " << nbVertex << " enforced vertices");
    MESSAGE("h->GetEnforcedVertices()->length(): " << nbVertexHyp);
    
    // Some vertices were removed
    if (nbVertex < nbVertexHyp) {
//        if (nbVertex == 0)
//            h->ClearEnforcedVertices();
//        else {
            // iterate over vertices of hypo
            for(int i = 0 ; i <nbVertexHyp ; i++) {
                double x = vertexHyp[i].x;
                double y = vertexHyp[i].y;
                double z = vertexHyp[i].z;
                // vertex is removed
                if (!smpVertexExists(x,y,z))
                    h->RemoveEnforcedVertex(x,y,z);
            }
//        }
    }
    
    TEnforcedVertexValues::const_iterator it;
    for(it = h_data.myEnforcedVertices.begin() ; it != h_data.myEnforcedVertices.end(); it++ ) {
      double x = it->at(0);
      double y = it->at(1);
      double z = it->at(2);
      double size = it->at(3);
      MESSAGE("(" << x   << ", "
                       << y   << ", "
                       << z   << ") = "
                       << size  );
      double mySize;
      try {
        mySize = h->GetEnforcedVertex(x,y,z);
        MESSAGE("Old size: " << mySize);
        if (mySize != size) {
          MESSAGE("Setting new size: " << size);
          h->SetEnforcedVertex(x,y,z,size);
        }
      }
      catch (...) {
        MESSAGE("Setting new size: " << size);
        h->SetEnforcedVertex(x,y,z,size);
      }
    }
    */
  }
  catch ( const SALOME::SALOME_Exception& ex )
  {
    SalomeApp_Tools::QtCatchCorbaException( ex );
    ok = false;
  }
  return ok;
}

bool HEXABLOCKPluginGUI_HypothesisCreator::readParamsFromWidgets( HEXABLOCKHypothesisData& h_data ) const
{
  if (MYDEBUG) MESSAGE("HEXABLOCKPluginGUI_HypothesisCreator::readParamsFromWidgets");
  h_data.myName                       = myName ? myName->text() : "";
  h_data.myToMeshHoles                = myToMeshHolesCheck->isChecked();
  h_data.myMaximumMemory              = myMaximumMemoryCheck->isChecked() ? myMaximumMemorySpin->value() : -1;
  h_data.myInitialMemory              = myInitialMemoryCheck->isChecked() ? myInitialMemorySpin->value() : -1;
  h_data.myOptimizationLevel          = myOptimizationLevelCombo->currentIndex();
  h_data.myKeepFiles                  = myKeepFiles->isChecked();
  h_data.myWorkingDir                 = myWorkingDir->text().trimmed();
  h_data.myVerboseLevel               = myVerboseLevelSpin->value();
  h_data.myToCreateNewNodes           = myToCreateNewNodesCheck->isChecked();
  h_data.myRemoveInitialCentralPoint  = myRemoveInitialCentralPointCheck->isChecked();
  h_data.myBoundaryRecovery           = myBoundaryRecoveryCheck->isChecked();
  h_data.myFEMCorrection              = myFEMCorrectionCheck->isChecked();
  h_data.myTextOption                 = myTextOption->text();
  h_data.myEnforcedVertices.clear();

  for (int i=0 ; i<mySmpModel->rowCount() ; i++) {
    HEXABLOCKEnforcedVertex myVertex;
    myVertex.push_back(mySmpModel->data(mySmpModel->index(i,ENF_VER_X_COLUMN)).toDouble());
    myVertex.push_back(mySmpModel->data(mySmpModel->index(i,ENF_VER_Y_COLUMN)).toDouble());
    myVertex.push_back(mySmpModel->data(mySmpModel->index(i,ENF_VER_Z_COLUMN)).toDouble());
    myVertex.push_back(mySmpModel->data(mySmpModel->index(i,ENF_VER_SIZE_COLUMN)).toDouble());
    if (MYDEBUG) MESSAGE("Add new enforced vertex (" << myVertex[0] << ", "
                                             << myVertex[1] << ", "
                                             << myVertex[2] << ") = "
                                             << myVertex[3]);
    h_data.myEnforcedVertices.push_back(myVertex);
  }

  return true;
}

QString HEXABLOCKPluginGUI_HypothesisCreator::caption() const
{
  return tr( "HEXABLOCK_TITLE" );
}

QPixmap HEXABLOCKPluginGUI_HypothesisCreator::icon() const
{
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "HEXABLOCKPlugin", tr( "ICON_DLG_HEXABLOCK_PARAMETERS" ) );
}

QString HEXABLOCKPluginGUI_HypothesisCreator::type() const
{
  return tr( "HEXABLOCK_HYPOTHESIS" );
}

QString HEXABLOCKPluginGUI_HypothesisCreator::helpPage() const
{
  return "HEXABLOCK_hypo_page.html";
}

//=============================================================================
/*! GetHypothesisCreator
 *
 */
//=============================================================================
extern "C"
{
  HEXABLOCKPLUGINGUI_EXPORT
  SMESHGUI_GenericHypothesisCreator* GetHypothesisCreator( const QString& aHypType )
  {
    if ( aHypType == "HEXABLOCK_Parameters" )
      return new HEXABLOCKPluginGUI_HypothesisCreator( aHypType );
    return 0;
  }
}
