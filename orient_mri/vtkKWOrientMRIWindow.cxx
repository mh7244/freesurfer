#include "vtkKWOrientMRIWindow.h"
#include "vtkObjectFactory.h"
#include "vtkKWToolbar.h"
#include "vtkKWToolbarSet.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWIcon.h"
#include "vtkKWMenu.h"
#include "vtkKWPushButton.h"

using namespace std;

vtkStandardNewMacro( vtkKWOrientMRIWindow );
vtkCxxRevisionMacro( vtkKWOrientMRIWindow, "$Revision: 1.1 $" );

vtkKWOrientMRIWindow::vtkKWOrientMRIWindow () :
  vtkKWWindow(),
  mView( NULL )
{

}

vtkKWOrientMRIWindow::~vtkKWOrientMRIWindow () {

}

void
vtkKWOrientMRIWindow::Create () {

  this->Superclass::Create();

  SetPanelLayoutToSecondaryBelowView();
  SecondaryPanelVisibilityOff();
  MainPanelVisibilityOff();

  // Create our interior view.
  mView = vtkKWOrientMRIView::New();
  mView->SetParent( GetViewFrame() );
  mView->Create();
  this->Script( "pack %s -expand yes -fill both -anchor c",
		mView->GetWidgetName() );


  // Make a toolbar.
  vtkKWToolbar* toolbar = vtkKWToolbar::New();
  toolbar->SetName( "Main" );
  toolbar->SetParent( GetMainToolbarSet()->GetToolbarsFrame() );
  toolbar->Create();
  GetMainToolbarSet()->AddToolbar( toolbar );

  // Make our commands. For each one, we make a toolbar button,
  // associate the command with the button, and then make a menu
  // entry.
  int nFilePos = GetFileMenuInsertPosition();
  int nEditPos = 0;
  int nViewPos = GetViewMenuInsertPosition();

  bzero( maPushButtons, sizeof(vtkKWPushButton*) * kcCommands );
  bzero( maMenuItems, sizeof(MenuItem) * kcCommands );
  bzero( maCommandEnabled, sizeof(bool) * kcCommands );

  // Init our toolbar buttons with common settings.
  for( int nCmd = 0; nCmd < kcCommands; nCmd++ ) {
    maPushButtons[(Command)nCmd] = vtkKWPushButton::New();
    maPushButtons[(Command)nCmd]->SetParent( toolbar->GetFrame() );
    maPushButtons[(Command)nCmd]->Create();
    maPushButtons[(Command)nCmd]->SetWidth( 30 );
    maPushButtons[(Command)nCmd]->SetHeight( 30 );
    maPushButtons[(Command)nCmd]->
      SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen ); // Placeholder.
    toolbar->AddWidget( maPushButtons[(Command)nCmd] );
    maPushButtons[(Command)nCmd]->Delete();
  }

  // Load Volume
  maPushButtons[CmdLoadVolume]->SetText( "Load Volume" );
  maPushButtons[CmdLoadVolume]->SetBalloonHelpString( "Load Volume" );
  maPushButtons[CmdLoadVolume]->
    SetImageToPredefinedIcon( vtkKWIcon::IconFileOpen );
  maPushButtons[CmdLoadVolume]->SetCommand( this, "LoadVolumeFromDlog" );

  // Save Volume
  maPushButtons[CmdSaveVolume]->SetText( "Save Volume" );
  maPushButtons[CmdSaveVolume]->SetBalloonHelpString( "Save Volume" );
  maPushButtons[CmdSaveVolume]->
    SetImageToPredefinedIcon( vtkKWIcon::IconFloppy );
  maPushButtons[CmdSaveVolume]->SetCommand( this, "SaveVolumeWithConfirm" );

  // Transform Volume
  maPushButtons[CmdTransformVolume]->SetText( "Transform Volume" );
  maPushButtons[CmdTransformVolume]->SetBalloonHelpString( "Transform Volume");
  maPushButtons[CmdTransformVolume]->
    SetImageToPredefinedIcon( vtkKWIcon::IconGridLinear );
  maPushButtons[CmdTransformVolume]->SetCommand( this, "TransformVolume" );
  
  // Revert.
  maPushButtons[CmdRevertVolume]->SetText( "Revert Volume" );
  maPushButtons[CmdRevertVolume]->SetBalloonHelpString( "Revert Volume" );
  maPushButtons[CmdRevertVolume]->
    SetImageToPredefinedIcon( vtkKWIcon::IconBrowserBack );
  maPushButtons[CmdRevertVolume]->SetCommand( this, "RevertToSavedTransform" );

  // Restore View
  maPushButtons[CmdRestoreView]->SetText( "Restore View" );
  maPushButtons[CmdRestoreView]->SetBalloonHelpString( "Restore View" );
  maPushButtons[CmdRestoreView]->
    SetImageToPredefinedIcon( vtkKWIcon::IconReload );
  maPushButtons[CmdRestoreView]->SetCommand( this, "RestoreView" );

  // Zoom Out
  maPushButtons[CmdZoomOut]->SetText( "Zoom Out" );
  maPushButtons[CmdZoomOut]->SetBalloonHelpString( "Zoom Out" );
  maPushButtons[CmdZoomOut]->
    SetImageToPredefinedIcon( vtkKWIcon::IconMagGlass );
  maPushButtons[CmdZoomOut]->SetCommand( this, "ZoomOut" );

  // Zoom In
  maPushButtons[CmdZoomIn]->SetText( "Zoom In" );
  maPushButtons[CmdZoomIn]->SetBalloonHelpString( "Zoom In" );
  maPushButtons[CmdZoomIn]->
    SetImageToPredefinedIcon( vtkKWIcon::IconMagGlass );
  maPushButtons[CmdZoomIn]->SetCommand( this, "ZoomIn" );

  
  // Build the menus. File menu.
  // Load Volume.
  GetFileMenu()->
    InsertCommand( nFilePos, "L&oad Volume...", this, "LoadVolumeFromDlog" );
  GetFileMenu()->
    SetItemImageToPredefinedIcon( nFilePos, vtkKWIcon::IconFileOpen );
  GetFileMenu()->SetItemCompoundModeToLeft( nFilePos );
  GetFileMenu()->SetItemAccelerator( nFilePos, "Ctrl+O" );
  maMenuItems[CmdLoadVolume].menu = GetFileMenu();
  maMenuItems[CmdLoadVolume].nItem = nFilePos;
  nFilePos++;

  GetFileMenu()->InsertSeparator( nFilePos++ );

  // Save Volume.
  GetFileMenu()->
    InsertCommand( nFilePos, "&Save Volume", this, "SaveVolumeWithConfirm" );
  GetFileMenu()->
    SetItemImageToPredefinedIcon( nFilePos, vtkKWIcon::IconFloppy );
  GetFileMenu()->SetItemCompoundModeToLeft( nFilePos );
  GetFileMenu()->SetItemAccelerator( nFilePos, "Ctrl+S" );
  maMenuItems[CmdSaveVolume].menu = GetFileMenu();
  maMenuItems[CmdSaveVolume].nItem = nFilePos;
  nFilePos++;

  GetFileMenu()->InsertSeparator( nFilePos++ );

  InsertRecentFilesMenu( nFilePos++, this );
  
  GetFileMenu()->InsertSeparator( nFilePos++ );

  // Edit menu.
  // Transform Volume.
  GetEditMenu()->
    InsertCommand( nEditPos, "Trans&form Volume", this, "TransformVolume" );
  GetEditMenu()->
    SetItemImageToPredefinedIcon( nEditPos, vtkKWIcon::IconGridLinear );
  GetEditMenu()->SetItemCompoundModeToLeft( nEditPos );
  GetEditMenu()->SetItemAccelerator( nEditPos, "Ctrl+F" );
  maMenuItems[CmdTransformVolume].menu = GetEditMenu();
  maMenuItems[CmdTransformVolume].nItem = nEditPos;
  nEditPos++;

  GetEditMenu()->InsertSeparator( nEditPos++ );

  // Revert.
  GetEditMenu()->
    InsertCommand( nEditPos, "&Revert Volume", this, "RevertToSavedTransform");
  GetEditMenu()->
    SetItemImageToPredefinedIcon( nEditPos, vtkKWIcon::IconBrowserBack );
  GetEditMenu()->SetItemCompoundModeToLeft( nEditPos );
  GetEditMenu()->SetItemAccelerator( nEditPos, "Ctrl+R" );
  maMenuItems[CmdRevertVolume].menu = GetEditMenu();
  maMenuItems[CmdRevertVolume].nItem = nEditPos;
  nEditPos++;

  // View menu.
  // Restore view.
  GetViewMenu()->
    InsertCommand( nViewPos, "Restore &View", this, "RestoreView");
  GetViewMenu()->
    SetItemImageToPredefinedIcon( nViewPos, vtkKWIcon::IconReload );
  GetViewMenu()->SetItemCompoundModeToLeft( nViewPos );
  GetViewMenu()->SetItemAccelerator( nViewPos, "Ctrl+V" );
  maMenuItems[CmdRestoreView].menu = GetViewMenu();
  maMenuItems[CmdRestoreView].nItem = nViewPos;
  nViewPos++;

  GetViewMenu()->InsertSeparator( nViewPos++ );

  // Zoom Out.
  GetViewMenu()->
    InsertCommand( nViewPos, "Zoom Out", this, "ZoomOut");
  GetViewMenu()->SetItemCompoundModeToLeft( nViewPos );
  //  GetViewMenu()->SetItemAccelerator( nViewPos, "Ctrl+Minus" );
  maMenuItems[CmdZoomOut].menu = GetViewMenu();
  maMenuItems[CmdZoomOut].nItem = nViewPos;
  nViewPos++;

  // Zoom In.
  GetViewMenu()->
    InsertCommand( nViewPos, "Zoom In", this, "ZoomIn");
  GetViewMenu()->SetItemCompoundModeToLeft( nViewPos );
  //  GetViewMenu()->SetItemAccelerator( nViewPos, "Ctrl+Plus" );
  maMenuItems[CmdZoomIn].menu = GetViewMenu();
  maMenuItems[CmdZoomIn].nItem = nViewPos;
  nViewPos++;

  // Update our menu and buttons.
  UpdateCommandStatus();
}

void
vtkKWOrientMRIWindow::LoadVolumeFromDlog () {

  vtkKWLoadSaveDialog* dialog = vtkKWLoadSaveDialog::New();
  dialog->SetApplication( GetApplication() );
  dialog->Create();
  dialog->SetFileTypes( "{MGH {.mgh .mgz}} {Binary {.bshort .bfloat}} {All {*}}" );
  dialog->RetrieveLastPathFromRegistry( "LoadVolume" );
  dialog->SetDefaultExtension( ".mgz" );
  if( dialog->Invoke() ) {
    dialog->SaveLastPathToRegistry( "LoadVolume" );
    string fnVolume( dialog->GetFileName() );
    this->LoadVolume( fnVolume.c_str() );
  }
}

void
vtkKWOrientMRIWindow::LoadVolume ( const char* ifnVolume ) {

  if( mView ) {
    try {
      mView->LoadVolume( ifnVolume );
      SetStatusText( "Volume loaded." );
      
      AddRecentFile( ifnVolume, this, "LoadVolume" ); 
    }
    catch( exception& e ) {
      ErrorMessage( e.what() );
    }
  }

  // Update our menu and buttons.
  UpdateCommandStatus();
}


void
vtkKWOrientMRIWindow::SaveVolumeWithConfirm () {

  if( mView->IsVolumeDirty() ) {
    if( vtkKWMessageDialog::PopupYesNo
	( GetApplication(), this,
	  "Save Volume",
	  "Are you sure you want to save changes?" ) ) {
      try {
	mView->SaveVolume();
	SetStatusText( "Volume saved." );
      }
      catch( exception& e ) {
	ErrorMessage( e.what() );
      }
    }
  }

  // Update our menu and buttons.
  UpdateCommandStatus();
}


void
vtkKWOrientMRIWindow::RevertToSavedTransform () {

  if( mView ) {
    try {
      mView->RevertToSavedTransform();
    }
    catch( exception& e ) {
      
    }
  }

  // Update our menu and buttons.
  UpdateCommandStatus();
}


void
vtkKWOrientMRIWindow::TransformVolume () {

  if( mView )
    mView->TransformVolume();

  // Update our menu and buttons.
  UpdateCommandStatus();
}


void
vtkKWOrientMRIWindow::RestoreView () {

  if( mView )
    mView->RestoreView();

  // Update our menu and buttons.
  UpdateCommandStatus();
}


void
vtkKWOrientMRIWindow::ZoomBy ( float iFactor ) {

  if( mView )
    mView->ZoomBy( iFactor );
  
  // Update our menu and buttons.
  UpdateCommandStatus();
}

void
vtkKWOrientMRIWindow::ZoomIn () {

  ZoomBy( 2.0 );
}

void
vtkKWOrientMRIWindow::ZoomOut () {

  ZoomBy( 0.5 );
}

void
vtkKWOrientMRIWindow::UpdateCommandStatus () {
  
  maCommandEnabled[CmdLoadVolume] = true;
  maCommandEnabled[CmdSaveVolume] = (mView != NULL && mView->IsVolumeDirty());
  maCommandEnabled[CmdTransformVolume] = (mView != NULL && mView->IsVolumeLoaded());
  maCommandEnabled[CmdRevertVolume] = (mView != NULL && mView->IsVolumeDirty());
  maCommandEnabled[CmdRestoreView] = true;
  maCommandEnabled[CmdZoomIn] = true;
  maCommandEnabled[CmdZoomOut] = true;
  
  for( int nCmd = 0; nCmd < kcCommands; nCmd++ ) {
    if( maCommandEnabled[(Command)nCmd] ) {
      if( NULL != maPushButtons[(Command)nCmd] )
	maPushButtons[(Command)nCmd]->SetStateToNormal();
      if( NULL != maMenuItems[(Command)nCmd].menu )
	maMenuItems[(Command)nCmd].menu->
	  SetItemStateToNormal( maMenuItems[(Command)nCmd].nItem );
    } else {
      if( NULL != maPushButtons[(Command)nCmd] )
	maPushButtons[(Command)nCmd]->SetStateToDisabled();
      if( NULL != maMenuItems[(Command)nCmd].menu )
	maMenuItems[(Command)nCmd].menu->
	  SetItemStateToDisabled( maMenuItems[(Command)nCmd].nItem );
    }
  }
}
