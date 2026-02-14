// @(#)root/gui:$Id: b4c21444ab4f787f65b2b44199fc0440c3c2ce81 $
// Author: Fons Rademakers   15/01/98

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


/** \class TRootCanvas
    \ingroup guiwidgets

This class creates a main window with menubar, scrollbars and a
drawing area. The widgets used are the new native ROOT GUI widgets.

*/


#include "RConfigure.h"

#include "TRootCanvas.h"
#include "TRootApplication.h"
#include "TRootHelpDialog.h"
#include "TGClient.h"
#include "TGCanvas.h"
#include "TGMenu.h"
#include "TGWidget.h"
#include "TGFileBrowser.h"
#include "TGFileDialog.h"
#include "TGStatusBar.h"
#include "TGTextEditDialogs.h"
#include "TROOT.h"
#include "TClass.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TBrowser.h"
#include "TClassTree.h"
#include "TMarker.h"
#include "TStyle.h"
#include "TColorWheel.h"
#include "TVirtualX.h"
#include "TApplication.h"
#include "TFile.h"
#include "TInterpreter.h"
#include "TEnv.h"
#include "TMath.h"
#include <iostream>
#include "TGDockableFrame.h"

#include "TG3DLine.h"
#include "TGToolBar.h"
#include "TGToolTip.h"
#include "TVirtualPadEditor.h"
#include "TRootControlBar.h"
#include "TGuiBuilder.h"
#include "TImage.h"
#include "TError.h"
#include "TGDNDManager.h"
#include "TBufferFile.h"
#include "TRootBrowser.h"
#include "TGTab.h"
#include "TGedEditor.h"

#include "TPluginManager.h"
#include "TVirtualGL.h"

#ifdef WIN32
#include "TWin32SplashThread.h"
#endif

#include "HelpText.h"


// Canvas menu command ids
// NOTE: With CMake unity builds enabled, multiple sources can end up in the same
// translation unit. Prefix these ids to avoid collisions with other GUI sources
// that define similarly-named enums.
enum ERootCanvasCommands {
   kCanvasFileNewCanvas,
   kCanvasFileOpen,
   kCanvasFileSaveAs,
   kCanvasFileSaveAsRoot,
   kCanvasFileSaveAsC,
   kCanvasFileSaveAsPS,
   kCanvasFileSaveAsEPS,
   kCanvasFileSaveAsPDF,
   kCanvasFileSaveAsGIF,
   kCanvasFileSaveAsJPG,
   kCanvasFileSaveAsPNG,
   kCanvasFileSaveAsTEX,
   kCanvasFilePrint,
   kCanvasFileCloseCanvas,
   kCanvasFileQuit,

   kCanvasEditStyle,
   kCanvasEditCut,
   kCanvasEditCopy,
   kCanvasEditPaste,
   kCanvasEditClearPad,
   kCanvasEditClearCanvas,
   kCanvasEditUndo,
   kCanvasEditRedo,

   kCanvasViewEditor,
   kCanvasViewToolbar,
   kCanvasViewEventStatus,
   kCanvasViewToolTips,
   kCanvasViewColors,
   kCanvasViewFonts,
   kCanvasViewMarkers,
   kCanvasViewIconify,
   kCanvasViewX3D,
   kCanvasViewOpenGL,

   kCanvasOptionAutoResize,
   kCanvasOptionResizeCanvas,
   kCanvasOptionMoveOpaque,
   kCanvasOptionResizeOpaque,
   kCanvasOptionInterrupt,
   kCanvasOptionRefresh,
   kCanvasOptionAutoExec,
   kCanvasOptionStatistics,
   kCanvasOptionHistTitle,
   kCanvasOptionFitParams,
   kCanvasOptionCanEdit,

   kCanvasInspectRoot,
   kCanvasClassesTree,
   kCanvasFitPanel,
   kCanvasToolsBrowser,
   kCanvasToolsBuilder,
   kCanvasToolsRecorder,

   kCanvasHelpAbout,
   kCanvasHelpOnCanvas,
   kCanvasHelpOnMenus,
   kCanvasHelpOnGraphicsEd,
   kCanvasHelpOnBrowser,
   kCanvasHelpOnObjects,
   kCanvasHelpOnPS,

   kCanvasToolModify,
   kCanvasToolArc,
   kCanvasToolLine,
   kCanvasToolArrow,
   kCanvasToolDiamond,
   kCanvasToolEllipse,
   kCanvasToolPad,
   kCanvasToolPave,
   kCanvasToolPLabel,
   kCanvasToolPText,
   kCanvasToolPsText,
   kCanvasToolGraph,
   kCanvasToolCurlyLine,
   kCanvasToolCurlyArc,
   kCanvasToolLatex,
   kCanvasToolMarker,
   kCanvasToolCutG

};

static const char *gCanvasOpenTypes[] = { "ROOT files",   "*.root",
                                     "All files",    "*",
                                     0,              0 };

static const char *gSaveAsTypes[] = { "PDF",          "*.pdf",
                                      "SVG",          "*.svg",
                                      "TeX",          "*.tex",
                                      "PostScript",   "*.ps",
                                      "Encapsulated PostScript", "*.eps",
                                      "PNG",          "*.png",
                                      "JPEG",         "*.jpg",
                                      "GIF",          "*.gif",
                                      "ROOT macros",  "*.C",
                                      "ROOT files",   "*.root",
                                      "XML",          "*.xml",
                                      "XPM",          "*.xpm",
                                      "TIFF",         "*.tiff",
                                      "XCF",          "*.xcf",
                                      "All files",    "*",
                                      0,              0 };

static ToolBarData_t gCanvasToolBarData[] = {
   // { filename,      tooltip,            staydown,  id,              button}
   { "newcanvas.xpm",  "New",              kFALSE,    kCanvasFileNewCanvas,  0 },
   { "open.xpm",       "Open",             kFALSE,    kCanvasFileOpen,       0 },
   { "save.xpm",       "Save As",          kFALSE,    kCanvasFileSaveAs,     0 },
   { "printer.xpm",    "Print",            kFALSE,    kCanvasFilePrint,      0 },
   { "",               "",                 kFALSE,    -1,              0 },
   { "interrupt.xpm",  "Interrupt",        kFALSE,    kCanvasOptionInterrupt,0 },
   { "refresh2.xpm",   "Refresh",          kFALSE,    kCanvasOptionRefresh,  0 },
   { "",               "",                 kFALSE,    -1,              0 },
   { "inspect.xpm",    "Inspect",          kFALSE,    kCanvasInspectRoot,    0 },
   { "browser.xpm",    "Browser",          kFALSE,    kCanvasToolsBrowser, 0 },
   { 0,                0,                  kFALSE,    0,               0 }
};

static ToolBarData_t gToolBarData1[] = {
   { "pointer.xpm",    "Modify",           kFALSE,    kCanvasToolModify,     0 },
   { "arc.xpm",        "Arc",              kFALSE,    kCanvasToolArc,        0 },
   { "line.xpm",       "Line",             kFALSE,    kCanvasToolLine,       0 },
   { "arrow.xpm",      "Arrow",            kFALSE,    kCanvasToolArrow,      0 },
   { "diamond.xpm",    "Diamond",          kFALSE,    kCanvasToolDiamond,    0 },
   { "ellipse.xpm",    "Ellipse",          kFALSE,    kCanvasToolEllipse,    0 },
   { "pad.xpm",        "Pad",              kFALSE,    kCanvasToolPad,        0 },
   { "pave.xpm",       "Pave",             kFALSE,    kCanvasToolPave,       0 },
   { "pavelabel.xpm",  "Pave Label",       kFALSE,    kCanvasToolPLabel,     0 },
   { "pavetext.xpm",   "Pave Text",        kFALSE,    kCanvasToolPText,      0 },
   { "pavestext.xpm",  "Paves Text",       kFALSE,    kCanvasToolPsText,     0 },
   { "graph.xpm",      "Graph",            kFALSE,    kCanvasToolGraph,      0 },
   { "curlyline.xpm",  "Curly Line",       kFALSE,    kCanvasToolCurlyLine,  0 },
   { "curlyarc.xpm",   "Curly Arc",        kFALSE,    kCanvasToolCurlyArc,   0 },
   { "latex.xpm",      "Text/Latex",       kFALSE,    kCanvasToolLatex,      0 },
   { "marker.xpm",     "Marker",           kFALSE,    kCanvasToolMarker,     0 },
   { "cut.xpm",        "Graphical Cut",    kFALSE,    kCanvasToolCutG,       0 },
   { 0,                0,                  kFALSE,    0,               0 }
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TRootContainer                                                       //
//                                                                      //
// Utility class used by TRootCanvas. The TRootContainer is the frame   //
// embedded in the TGCanvas widget. The ROOT graphics goes into this    //
// frame. This class is used to enable input events on this graphics    //
// frame and forward the events to the TRootCanvas handlers.            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TRootContainer : public TGCompositeFrame {
private:
   TRootCanvas  *fCanvas;    // pointer back to canvas imp
public:
   TRootContainer(TRootCanvas *c, Window_t id, const TGWindow *parent);

   Bool_t  HandleButton(Event_t *ev) override;
   Bool_t  HandleDoubleClick(Event_t *ev) override
               { return fCanvas->HandleContainerDoubleClick(ev); }
   Bool_t  HandleConfigureNotify(Event_t *ev) override
               { TGFrame::HandleConfigureNotify(ev);
                  return fCanvas->HandleContainerConfigure(ev); }
   Bool_t  HandleKey(Event_t *ev) override
               { return fCanvas->HandleContainerKey(ev); }
   Bool_t  HandleMotion(Event_t *ev) override
               { return fCanvas->HandleContainerMotion(ev); }
   Bool_t  HandleExpose(Event_t *ev) override
               { return fCanvas->HandleContainerExpose(ev); }
   Bool_t  HandleCrossing(Event_t *ev) override
               { return fCanvas->HandleContainerCrossing(ev); }
   void    SavePrimitive(std::ostream &out, Option_t * = "") override;
   void    SetEditable(Bool_t) override { }
};

////////////////////////////////////////////////////////////////////////////////
/// Create a canvas container.

TRootContainer::TRootContainer(TRootCanvas *c, Window_t id, const TGWindow *p)
   : TGCompositeFrame(gClient, id, p)
{
   fCanvas = c;

   gVirtualX->GrabButton(fId, kAnyButton, kAnyModifier,
                         kButtonPressMask | kButtonReleaseMask |
                         kPointerMotionMask, kNone, kNone);

   AddInput(kKeyPressMask | kKeyReleaseMask | kPointerMotionMask |
            kExposureMask | kStructureNotifyMask | kLeaveWindowMask);
   fEditDisabled = kEditDisable;
}

////////////////////////////////////////////////////////////////////////////////
/// Directly handle scroll mouse buttons (4 and 5), only pass buttons
/// 1, 2 and 3 on to the TCanvas.

Bool_t TRootContainer::HandleButton(Event_t *event)
{
   TGViewPort *vp = (TGViewPort*)fParent;
   UInt_t page = vp->GetHeight()/4;
   Int_t newpos;

   gVirtualX->SetInputFocus(GetMainFrame()->GetId());

   if (event->fCode == kButton4) {
      //scroll up
      newpos = fCanvas->fCanvasWindow->GetVsbPosition() - page;
      if (newpos < 0) newpos = 0;
      fCanvas->fCanvasWindow->SetVsbPosition(newpos);
//      return kTRUE;
   }
   if (event->fCode == kButton5) {
      // scroll down
      newpos = fCanvas->fCanvasWindow->GetVsbPosition() + page;
      fCanvas->fCanvasWindow->SetVsbPosition(newpos);
//      return kTRUE;
   }
   return fCanvas->HandleContainerButton(event);
}


////////////////////////////////////////////////////////////////////////////////
/// Create a basic ROOT canvas.

TRootCanvas::TRootCanvas(TCanvas *c, const char *name, UInt_t width, UInt_t height)
   : TGMainFrame(gClient->GetRoot(), width, height), TCanvasImp(c)
{
   CreateCanvas(name);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   Resize(width, height);
}

////////////////////////////////////////////////////////////////////////////////
/// Create a basic ROOT canvas.

TRootCanvas::TRootCanvas(TCanvas *c, const char *name, Int_t x, Int_t y, UInt_t width, UInt_t height)
   : TGMainFrame(gClient->GetRoot(), width, height), TCanvasImp(c)
{
   CreateCanvas(name);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   MoveResize(x, y, width, height);
   SetWMPosition(x, y);
}

////////////////////////////////////////////////////////////////////////////////
/// Create the actual canvas.

void TRootCanvas::CreateCanvas(const char *name)
{
   fButton    = 0;
   fAutoFit   = kTRUE;   // check also menu entry
   fEditor    = 0;
   fEmbedded  = kFALSE;

   // Create menus
   fFileSaveMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fFileSaveMenu->AddEntry(Form("%s.&ps",  name), kCanvasFileSaveAsPS);
   fFileSaveMenu->AddEntry(Form("%s.&eps", name), kCanvasFileSaveAsEPS);
   fFileSaveMenu->AddEntry(Form("%s.p&df", name), kCanvasFileSaveAsPDF);
   fFileSaveMenu->AddEntry(Form("%s.&tex", name), kCanvasFileSaveAsTEX);
   fFileSaveMenu->AddEntry(Form("%s.&gif", name), kCanvasFileSaveAsGIF);

   static Int_t img = 0;

   if (!img) {
      Int_t sav = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      TImage* itmp = TImage::Create();
      img = itmp ? 1 : -1;
      if (itmp) {
         delete itmp;
         itmp=NULL;
      }
      gErrorIgnoreLevel = sav;
   }
   if (img > 0) {
      fFileSaveMenu->AddEntry(Form("%s.&jpg",name),  kCanvasFileSaveAsJPG);
      fFileSaveMenu->AddEntry(Form("%s.&png",name),  kCanvasFileSaveAsPNG);
   }

   fFileSaveMenu->AddEntry(Form("%s.&C",   name), kCanvasFileSaveAsC);
   fFileSaveMenu->AddEntry(Form("%s.&root",name), kCanvasFileSaveAsRoot);

   fFileMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fFileMenu->AddEntry("&New Canvas",   kCanvasFileNewCanvas);
   fFileMenu->AddEntry("&Open...",      kCanvasFileOpen);
   fFileMenu->AddEntry("&Close Canvas", kCanvasFileCloseCanvas);
   fFileMenu->AddSeparator();
   fFileMenu->AddPopup("&Save",         fFileSaveMenu);
   fFileMenu->AddEntry("Save &As...",   kCanvasFileSaveAs);
   fFileMenu->AddSeparator();
   fFileMenu->AddEntry("&Print...",     kCanvasFilePrint);
   fFileMenu->AddSeparator();
   fFileMenu->AddEntry("&Quit ROOT",    kCanvasFileQuit);

   fEditClearMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fEditClearMenu->AddEntry("&Pad",     kCanvasEditClearPad);
   fEditClearMenu->AddEntry("&Canvas",  kCanvasEditClearCanvas);

   fEditMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fEditMenu->AddEntry("&Style...",     kCanvasEditStyle);
   fEditMenu->AddSeparator();
   fEditMenu->AddEntry("Cu&t",          kCanvasEditCut);
   fEditMenu->AddEntry("&Copy",         kCanvasEditCopy);
   fEditMenu->AddEntry("&Paste",        kCanvasEditPaste);
   fEditMenu->AddSeparator();
   fEditMenu->AddPopup("C&lear",        fEditClearMenu);
   fEditMenu->AddSeparator();
   fEditMenu->AddEntry("&Undo",         kCanvasEditUndo);
   fEditMenu->AddEntry("&Redo",         kCanvasEditRedo);

   fEditMenu->DisableEntry(kCanvasEditCut);
   fEditMenu->DisableEntry(kCanvasEditCopy);
   fEditMenu->DisableEntry(kCanvasEditPaste);
   fEditMenu->DisableEntry(kCanvasEditUndo);
   fEditMenu->DisableEntry(kCanvasEditRedo);

   fViewWithMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fViewWithMenu->AddEntry("&X3D",      kCanvasViewX3D);
   fViewWithMenu->AddEntry("&OpenGL",   kCanvasViewOpenGL);

   fViewMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fViewMenu->AddEntry("&Editor",       kCanvasViewEditor);
   fViewMenu->AddEntry("&Toolbar",      kCanvasViewToolbar);
   fViewMenu->AddEntry("Event &Statusbar", kCanvasViewEventStatus);
   fViewMenu->AddEntry("T&oolTip Info", kCanvasViewToolTips);
   fViewMenu->AddSeparator();
   fViewMenu->AddEntry("&Colors",       kCanvasViewColors);
   fViewMenu->AddEntry("&Fonts",        kCanvasViewFonts);
   fViewMenu->AddEntry("&Markers",      kCanvasViewMarkers);
   fViewMenu->AddSeparator();
   fViewMenu->AddEntry("&Iconify",      kCanvasViewIconify);
   fViewMenu->AddSeparator();
   fViewMenu->AddPopup("&View With",    fViewWithMenu);

   fViewMenu->DisableEntry(kCanvasViewFonts);

   fOptionMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fOptionMenu->AddEntry("&Auto Resize Canvas",  kCanvasOptionAutoResize);
   fOptionMenu->AddEntry("&Resize Canvas",       kCanvasOptionResizeCanvas);
   fOptionMenu->AddEntry("&Move Opaque",         kCanvasOptionMoveOpaque);
   fOptionMenu->AddEntry("Resize &Opaque",       kCanvasOptionResizeOpaque);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Interrupt",           kCanvasOptionInterrupt);
   fOptionMenu->AddEntry("R&efresh",             kCanvasOptionRefresh);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Pad Auto Exec",       kCanvasOptionAutoExec);
   fOptionMenu->AddSeparator();
   fOptionMenu->AddEntry("&Statistics",          kCanvasOptionStatistics);
   fOptionMenu->AddEntry("Histogram &Title",     kCanvasOptionHistTitle);
   fOptionMenu->AddEntry("&Fit Parameters",      kCanvasOptionFitParams);
   fOptionMenu->AddEntry("Can Edit &Histograms", kCanvasOptionCanEdit);

   // Opaque options initialized in InitWindow()
   fOptionMenu->CheckEntry(kCanvasOptionAutoResize);
   if (gStyle->GetOptStat())
      fOptionMenu->CheckEntry(kCanvasOptionStatistics);
   if (gStyle->GetOptTitle())
      fOptionMenu->CheckEntry(kCanvasOptionHistTitle);
   if (gStyle->GetOptFit())
      fOptionMenu->CheckEntry(kCanvasOptionFitParams);
   if (gROOT->GetEditHistograms())
      fOptionMenu->CheckEntry(kCanvasOptionCanEdit);

   fToolsMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fToolsMenu->AddEntry("&Inspect ROOT",   kCanvasInspectRoot);
   fToolsMenu->AddEntry("&Class Tree",     kCanvasClassesTree);
   fToolsMenu->AddEntry("&Fit Panel",      kCanvasFitPanel);
   fToolsMenu->AddEntry("&Start Browser",  kCanvasToolsBrowser);
   fToolsMenu->AddEntry("&Gui Builder",    kCanvasToolsBuilder);
   fToolsMenu->AddEntry("&Event Recorder", kCanvasToolsRecorder);

   fHelpMenu = new TGPopupMenu(fClient->GetDefaultRoot());
   fHelpMenu->AddLabel("Basic Help On...");
   fHelpMenu->AddSeparator();
   fHelpMenu->AddEntry("&Canvas",          kCanvasHelpOnCanvas);
   fHelpMenu->AddEntry("&Menus",           kCanvasHelpOnMenus);
   fHelpMenu->AddEntry("&Graphics Editor", kCanvasHelpOnGraphicsEd);
   fHelpMenu->AddEntry("&Browser",         kCanvasHelpOnBrowser);
   fHelpMenu->AddEntry("&Objects",         kCanvasHelpOnObjects);
   fHelpMenu->AddEntry("&PostScript",      kCanvasHelpOnPS);
   fHelpMenu->AddSeparator();
   fHelpMenu->AddEntry("&About ROOT...",   kCanvasHelpAbout);

   // This main frame will process the menu commands
   fFileMenu->Associate(this);
   fFileSaveMenu->Associate(this);
   fEditMenu->Associate(this);
   fEditClearMenu->Associate(this);
   fViewMenu->Associate(this);
   fViewWithMenu->Associate(this);
   fOptionMenu->Associate(this);
   fToolsMenu->Associate(this);
   fHelpMenu->Associate(this);

   // Create menubar layout hints
   fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 1, 1);
   fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

   // Create menubar
   fMenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File",    fFileMenu,    fMenuBarItemLayout);
   fMenuBar->AddPopup("&Edit",    fEditMenu,    fMenuBarItemLayout);
   fMenuBar->AddPopup("&View",    fViewMenu,    fMenuBarItemLayout);
   fMenuBar->AddPopup("&Options", fOptionMenu,  fMenuBarItemLayout);
   fMenuBar->AddPopup("&Tools",   fToolsMenu,   fMenuBarItemLayout);
   fMenuBar->AddPopup("&Help",    fHelpMenu,    fMenuBarHelpLayout);

   AddFrame(fMenuBar, fMenuBarLayout);

   fHorizontal1 = new TGHorizontal3DLine(this);
   fHorizontal1Layout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   AddFrame(fHorizontal1, fHorizontal1Layout);

   // Create toolbar dock
   fToolDock = new TGDockableFrame(this);
   fToolDock->SetCleanup();
   fToolDock->EnableHide(kFALSE);
   AddFrame(fToolDock, fDockLayout = new TGLayoutHints(kLHintsExpandX));

   // will allocate it later
   fToolBar = 0;
   fVertical1 = 0;
   fVertical2 = 0;
   fVertical1Layout = 0;
   fVertical2Layout = 0;

   fToolBarSep = new TGHorizontal3DLine(this);
   fToolBarLayout = new TGLayoutHints(kLHintsTop |  kLHintsExpandX);
   AddFrame(fToolBarSep, fToolBarLayout);

   fMainFrame = new TGCompositeFrame(this, GetWidth() + 4, GetHeight() + 4,
                                      kHorizontalFrame);
   fMainFrameLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

   // Create editor frame that will host the pad editor
   fEditorFrame = new TGCompositeFrame(fMainFrame, 175, fMainFrame->GetHeight()+4, kFixedWidth);
   fEditorLayout = new TGLayoutHints(kLHintsExpandY | kLHintsLeft);
   fMainFrame->AddFrame(fEditorFrame, fEditorLayout);

   // Create canvas and canvas container that will host the ROOT graphics
   fCanvasWindow = new TGCanvas(fMainFrame, GetWidth()+4, GetHeight()+4,
                                kSunkenFrame | kDoubleBorder);

   fCanvasID = -1;

   if (fCanvas->UseGL()) {
      fCanvas->SetSupportGL(kFALSE);
      //first, initialize GL (if not yet)
      if (!gGLManager) {
         TString x = "win32";
         if (gVirtualX->InheritsFrom("TGX11"))
            x = "x11";
         else if (gVirtualX->InheritsFrom("TGCocoa"))
            x = "osx";

         TPluginHandler *ph = gROOT->GetPluginManager()->FindHandler("TGLManager", x);

         if (ph && ph->LoadPlugin() != -1) {
            if (!ph->ExecPlugin(0))
               Error("CreateCanvas", "GL manager plugin failed");
         }
      }

      if (gGLManager) {
         fCanvasID = gGLManager->InitGLWindow((ULongptr_t)fCanvasWindow->GetViewPort()->GetId());
         if (fCanvasID != -1) {
            //Create gl context.
            const Int_t glCtx = gGLManager->CreateGLContext(fCanvasID);
            if (glCtx != -1) {
               fCanvas->SetSupportGL(kTRUE);
               fCanvas->SetGLDevice(glCtx);//Now, fCanvas is responsible for context deletion!
            } else
               Error("CreateCanvas", "GL context creation failed.");
         } else
            Error("CreateCanvas", "GL window creation failed\n");
      }
   }

   if (fCanvasID == -1)
      fCanvasID = gVirtualX->InitWindow((ULongptr_t)fCanvasWindow->GetViewPort()->GetId());

   Window_t win = gVirtualX->GetWindowID(fCanvasID);
   fCanvasContainer = new TRootContainer(this, win, fCanvasWindow->GetViewPort());
   fCanvasWindow->SetContainer(fCanvasContainer);
   fCanvasLayout = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY | kLHintsRight);

   fMainFrame->AddFrame(fCanvasWindow, fCanvasLayout);
   AddFrame(fMainFrame, fMainFrameLayout);

   // create the tooltip with a timeout of 250 ms
   fToolTip = new TGToolTip(fClient->GetDefaultRoot(), fCanvasWindow, "", 250);

   fCanvas->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)",
                    "TRootCanvas", this,
                    "EventInfo(Int_t, Int_t, Int_t, TObject*)");

   // Create status bar
   int parts[] = { 33, 10, 10, 47 };
   fStatusBar = new TGStatusBar(this, 10, 10);
   fStatusBar->SetParts(parts, 4);

   fStatusBarLayout = new TGLayoutHints(kLHintsBottom | kLHintsLeft | kLHintsExpandX, 2, 2, 1, 1);

   AddFrame(fStatusBar, fStatusBarLayout);

   // Misc
   SetWindowName(name);
   SetIconName(name);
   fIconPic = SetIconPixmap("macro_s.xpm");
   SetClassHints("ROOT", "Canvas");

   SetEditDisabled(kEditDisable);
   MapSubwindows();

   // by default status bar, tool bar and pad editor are hidden
   HideFrame(fStatusBar);
   HideFrame(fToolDock);
   HideFrame(fToolBarSep);
   HideFrame(fHorizontal1);

   ShowToolBar(kFALSE);
   ShowEditor(kFALSE);

   // we need to use GetDefaultSize() to initialize the layout algorithm...
   Resize(GetDefaultSize());

   gVirtualX->SetDNDAware(fId, fDNDTypeList);
   SetDNDTarget(kTRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// Delete ROOT basic canvas. Order is significant. Delete in reverse
/// order of creation.

TRootCanvas::~TRootCanvas()
{
   delete fToolTip;
   if (fIconPic) gClient->FreePicture(fIconPic);
   if (fEditor && !fEmbedded) delete fEditor;
   if (fToolBar) {
      Disconnect(fToolDock, "Docked()",   this, "AdjustSize()");
      Disconnect(fToolDock, "Undocked()", this, "AdjustSize()");
      fToolBar->Cleanup();
      delete fToolBar;
   }

   if (!MustCleanup()) {
      delete fStatusBar;
      delete fStatusBarLayout;
      delete fCanvasContainer;
      delete fCanvasWindow;

      delete fEditorFrame;
      delete fEditorLayout;
      delete fMainFrame;
      delete fMainFrameLayout;
      delete fToolBarSep;
      delete fToolDock;
      delete fToolBarLayout;
      delete fHorizontal1;
      delete fHorizontal1Layout;

      delete fMenuBar;
      delete fMenuBarLayout;
      delete fMenuBarItemLayout;
      delete fMenuBarHelpLayout;
      delete fCanvasLayout;
      delete fDockLayout;
   }

   delete fFileMenu;
   delete fFileSaveMenu;
   delete fEditMenu;
   delete fEditClearMenu;
   delete fViewMenu;
   delete fViewWithMenu;
   delete fOptionMenu;
   delete fToolsMenu;
   delete fHelpMenu;
}

////////////////////////////////////////////////////////////////////////////////
/// Called via TCanvasImp interface by TCanvas.

void TRootCanvas::Close()
{
   TVirtualPadEditor* gged = TVirtualPadEditor::GetPadEditor(kFALSE);
   if(gged && gged->GetCanvas() == fCanvas) {
      if (fEmbedded) {
         ((TGedEditor *)gged)->SetModel(0, 0, kButton1Down);
         ((TGedEditor *)gged)->SetCanvas(0);
      }
      else gged->Hide();
   }

   gVirtualX->CloseWindow();
}

////////////////////////////////////////////////////////////////////////////////
/// Really delete the canvas and this GUI.

void TRootCanvas::ReallyDelete()
{
   TVirtualPadEditor* gged = TVirtualPadEditor::GetPadEditor(kFALSE);
   if(gged && gged->GetCanvas() == fCanvas) {
      if (fEmbedded) {
         ((TGedEditor *)gged)->SetModel(0, 0, kButton1Down);
         ((TGedEditor *)gged)->SetCanvas(0);
      }
      else gged->Hide();
   }

   fToolTip->Hide();
   Disconnect(fCanvas, "ProcessedEvent(Int_t, Int_t, Int_t, TObject*)",
              this, "EventInfo(Int_t, Int_t, Int_t, TObject*)");

   fCanvas->SetCanvasImp(0);
   fCanvas->Clear();
   fCanvas->SetName("");
   if (gPad && gPad->GetCanvas() == fCanvas)
      gPad = nullptr;
   delete this;
}

////////////////////////////////////////////////////////////////////////////////
/// In case window is closed via WM we get here.

void TRootCanvas::CloseWindow()
{
   DeleteWindow();
}

////////////////////////////////////////////////////////////////////////////////
/// Return width of canvas container.

UInt_t TRootCanvas::GetCwidth() const
{
   return fCanvasContainer->GetWidth();
}

////////////////////////////////////////////////////////////////////////////////
/// Return height of canvas container.

UInt_t TRootCanvas::GetCheight() const
{
   return fCanvasContainer->GetHeight();
}

////////////////////////////////////////////////////////////////////////////////
/// Gets the size and position of the window containing the canvas. This
/// size includes the menubar and borders.

UInt_t TRootCanvas::GetWindowGeometry(Int_t &x, Int_t &y, UInt_t &w, UInt_t &h)
{
   gVirtualX->GetWindowSize(fId, x, y, w, h);

   Window_t childdum;
   gVirtualX->TranslateCoordinates(fId, gClient->GetDefaultRoot()->GetId(),
                                   0, 0, x, y, childdum);
   if (!fCanvas->GetShowEditor()) return 0;
   return fEditorFrame->GetWidth();
}

////////////////////////////////////////////////////////////////////////////////
/// Set text in status bar.

void TRootCanvas::SetStatusText(const char *txt, Int_t partidx)
{
   fStatusBar->SetText(txt, partidx);
}

////////////////////////////////////////////////////////////////////////////////
/// Handle menu and other command generated by the user.

Bool_t TRootCanvas::ProcessMessage(Longptr_t msg, Longptr_t parm1, Longptr_t)
{
   TRootHelpDialog *hd;
   TList *lc;

   switch (GET_MSG(msg)) {

      case kC_COMMAND:

         switch (GET_SUBMSG(msg)) {

            case kCM_BUTTON:
            case kCM_MENU:

               switch (parm1) {
                  // Handle toolbar items...
				   case kCanvasToolModify:
                     gROOT->SetEditorMode();
                     break;
				   case kCanvasToolArc:
                     gROOT->SetEditorMode("Arc");
                     break;
				   case kCanvasToolLine:
                     gROOT->SetEditorMode("Line");
                     break;
				   case kCanvasToolArrow:
                     gROOT->SetEditorMode("Arrow");
                     break;
				   case kCanvasToolDiamond:
                     gROOT->SetEditorMode("Diamond");
                     break;
				   case kCanvasToolEllipse:
                     gROOT->SetEditorMode("Ellipse");
                     break;
				   case kCanvasToolPad:
                     gROOT->SetEditorMode("Pad");
                     break;
				   case kCanvasToolPave:
                     gROOT->SetEditorMode("Pave");
                     break;
				   case kCanvasToolPLabel:
                     gROOT->SetEditorMode("PaveLabel");
                     break;
				   case kCanvasToolPText:
                     gROOT->SetEditorMode("PaveText");
                     break;
				   case kCanvasToolPsText:
                     gROOT->SetEditorMode("PavesText");
                     break;
				   case kCanvasToolGraph:
                     gROOT->SetEditorMode("PolyLine");
                     break;
				   case kCanvasToolCurlyLine:
                     gROOT->SetEditorMode("CurlyLine");
                     break;
				   case kCanvasToolCurlyArc:
                     gROOT->SetEditorMode("CurlyArc");
                     break;
				   case kCanvasToolLatex:
                     gROOT->SetEditorMode("Text");
                     break;
				   case kCanvasToolMarker:
                     gROOT->SetEditorMode("Marker");
                     break;
				   case kCanvasToolCutG:
                     gROOT->SetEditorMode("CutG");
                     break;

                  // Handle File menu items...
				   case kCanvasFileNewCanvas:
                     gROOT->MakeDefCanvas();
                     break;
				   case kCanvasFileOpen:
                     {
                        static TString dir(".");
                        TGFileInfo fi;
				         fi.fFileTypes = gCanvasOpenTypes;
                        fi.SetIniDir(dir);
                        new TGFileDialog(fClient->GetDefaultRoot(), this, kFDOpen,&fi);
                        if (!fi.fFilename) return kTRUE;
                        dir = fi.fIniDir;
                        TFile::Open(fi.fFilename, "update");
                        TIter next(gROOT->GetListOfBrowsers());
                        TBrowser *b;
                        while ((b = (TBrowser *)next())) {
                           TRootBrowser *rb = dynamic_cast<TRootBrowser *>(b->GetBrowserImp());
                           if (rb) {
                              TGFileBrowser *fb = dynamic_cast<TGFileBrowser *>(rb->GetActBrowser());
                              if (fb)
                                 fb->Selected(0);
                           }
                        }
                        gROOT->RefreshBrowsers();
                     }
                     break;
				   case kCanvasFileSaveAs:
                     {
                        TString workdir = gSystem->WorkingDirectory();
                        static TString dir(".");
                        static Int_t typeidx = 0;
                        static Bool_t overwr = kFALSE;
                        TGFileInfo fi;
                        TString defaultType = gEnv->GetValue("Canvas.SaveAsDefaultType", ".pdf");
                        if (typeidx == 0) {
                           for (int i=1;gSaveAsTypes[i];i+=2) {
                              TString ftype = gSaveAsTypes[i];
                              if (ftype.EndsWith(defaultType.Data())) {
                                 typeidx = i-1;
                                 break;
                              }
                           }
                        }
                        fi.fFileTypes   = gSaveAsTypes;
                        fi.SetIniDir(dir);
                        fi.fFileTypeIdx = typeidx;
                        fi.fOverwrite = overwr;
                        new TGFileDialog(fClient->GetDefaultRoot(), this, kFDSave, &fi);
                        gSystem->ChangeDirectory(workdir.Data());
                        if (!fi.fFilename) return kTRUE;
                        Bool_t  appendedType = kFALSE;
                        TString fn = fi.fFilename;
                        TString ft = fi.fFileTypes[fi.fFileTypeIdx+1];
                        dir     = fi.fIniDir;
                        typeidx = fi.fFileTypeIdx;
                        overwr  = fi.fOverwrite;
again:
                        if (fn.EndsWith(".root") ||
                            fn.EndsWith(".ps")   ||
                            fn.EndsWith(".eps")  ||
                            fn.EndsWith(".pdf")  ||
                            fn.EndsWith(".svg")  ||
                            fn.EndsWith(".tex")  ||
                            fn.EndsWith(".gif")  ||
                            fn.EndsWith(".xml")  ||
                            fn.EndsWith(".xpm")  ||
                            fn.EndsWith(".jpg")  ||
                            fn.EndsWith(".png")  ||
                            fn.EndsWith(".xcf")  ||
                            fn.EndsWith(".tiff")) {
                           fCanvas->SaveAs(fn);
                        } else if (fn.EndsWith(".C"))
                           fCanvas->SaveSource(fn);
                        else {
                           if (!appendedType) {
                              if (ft.Index(".") != kNPOS) {
                                 fn += ft(ft.Index("."), ft.Length());
                                 appendedType = kTRUE;
                                 goto again;
                              }
                           }
                           Warning("ProcessMessage", "file %s cannot be saved with this extension", fi.fFilename);
                        }
                        for (int i=1;gSaveAsTypes[i];i+=2) {
                           TString ftype = gSaveAsTypes[i];
                           ftype.ReplaceAll("*.", ".");
                           if (fn.EndsWith(ftype.Data())) {
                              typeidx = i-1;
                              break;
                           }
                        }
                     }
                     break;
				   case kCanvasFileSaveAsRoot:
                     fCanvas->SaveAs(".root");
                     break;
				   case kCanvasFileSaveAsC:
                     fCanvas->SaveSource();
                     break;
				   case kCanvasFileSaveAsPS:
                     fCanvas->SaveAs();
                     break;
				   case kCanvasFileSaveAsEPS:
                     fCanvas->SaveAs(".eps");
                     break;
				   case kCanvasFileSaveAsPDF:
                     fCanvas->SaveAs(".pdf");
                     break;
				   case kCanvasFileSaveAsGIF:
                     fCanvas->SaveAs(".gif");
                     break;
				   case kCanvasFileSaveAsJPG:
                     fCanvas->SaveAs(".jpg");
                     break;
				   case kCanvasFileSaveAsPNG:
                     fCanvas->SaveAs(".png");
                     break;
				   case kCanvasFileSaveAsTEX:
                     fCanvas->SaveAs(".tex");
                     break;
				   case kCanvasFilePrint:
                     PrintCanvas();
                     break;
				   case kCanvasFileCloseCanvas:
                     SendCloseMessage();
                     break;
				   case kCanvasFileQuit:
                     if (!gApplication->ReturnFromRun()) {
                        if ((TVirtualPadEditor::GetPadEditor(kFALSE) != 0))
                           TVirtualPadEditor::Terminate();
                        SendCloseMessage();
                     }
                     if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
                        TVirtualPadEditor::Terminate();
                     if (TClass::GetClass("TStyleManager", kFALSE, kTRUE))
                        gROOT->ProcessLine("TStyleManager::Terminate()");
                     gApplication->Terminate(0);
                     break;

                  // Handle Edit menu items...
				   case kCanvasEditStyle:
                     if (!TClass::GetClass("TStyleManager"))
                        gSystem->Load("libGed");
                     gROOT->ProcessLine("TStyleManager::Show()");
                     break;
				   case kCanvasEditCut:
                     // still noop
                     break;
				   case kCanvasEditCopy:
                     // still noop
                     break;
				   case kCanvasEditPaste:
                     // still noop
                     break;
				   case kCanvasEditUndo:
                     // noop
                     break;
                  case kCanvasEditRedo:
                     // noop
                     break;
                  case kCanvasEditClearPad:
                     gPad->Clear();
                     gPad->Modified();
                     gPad->Update();
                     break;
                  case kCanvasEditClearCanvas:
                     fCanvas->Clear();
                     fCanvas->Modified();
                     fCanvas->Update();
                     break;

                  // Handle View menu items...
                  case kCanvasViewEditor:
                     fCanvas->ToggleEditor();
                     break;
                  case kCanvasViewToolbar:
                     fCanvas->ToggleToolBar();
                     break;
                  case kCanvasViewEventStatus:
                     fCanvas->ToggleEventStatus();
                     break;
                  case kCanvasViewToolTips:
                     fCanvas->ToggleToolTips();
                     break;
                  case kCanvasViewColors:
                     {
                        TVirtualPad *padsav = gPad->GetCanvas();
                        //This was the code with the old color table
                        //   TCanvas *m = new TCanvas("colors","Color Table");
                        //   TPad::DrawColorTable();
                        //   m->Update();
                        TColorWheel *wheel = new TColorWheel();
                        wheel->Draw();

                        //tp: with Cocoa, window is visible (and repainted)
                        //before wheel->Draw() was called and you can see "empty"
                        //canvas.
                        gPad->Update();
                        //
                        if (padsav) padsav->cd();
                     }
                     break;
                  case kCanvasViewFonts:
                     // noop
                     break;
                  case kCanvasViewMarkers:
                     {
                        TVirtualPad *padsav = gPad ? gPad->GetCanvas() : nullptr;
                        TCanvas *m = new TCanvas("markers","Marker Types",600,200);
                        TMarker::DisplayMarkerTypes();
                        m->Update();
                        if (padsav) padsav->cd();
                     }
                     break;
                  case kCanvasViewIconify:
                     Iconify();
                     break;
                  case kCanvasViewX3D:
                     gPad->GetViewer3D("x3d");
                     break;
                  case kCanvasViewOpenGL:
                     gPad->GetViewer3D("ogl");
                     break;

                  // Handle Option menu items...
                  case kCanvasOptionAutoExec:
                     fCanvas->ToggleAutoExec();
                     if (fCanvas->GetAutoExec()) {
                        fOptionMenu->CheckEntry(kCanvasOptionAutoExec);
                     } else {
                        fOptionMenu->UnCheckEntry(kCanvasOptionAutoExec);
                     }
                     break;
                  case kCanvasOptionAutoResize:
                     {
                        fAutoFit = fAutoFit ? kFALSE : kTRUE;
                        int opt = fCanvasContainer->GetOptions();
                        if (fAutoFit) {
                           opt &= ~kFixedSize;
                           fOptionMenu->CheckEntry(kCanvasOptionAutoResize);
                        } else {
                           opt |= kFixedSize;
                           fOptionMenu->UnCheckEntry(kCanvasOptionAutoResize);
                        }
                        fCanvasContainer->ChangeOptions(opt);
                        // in case of autofit this will generate a configure
                        // event for the container and this will force the
                        // update of the TCanvas
                        //Layout();
                     }
                     Layout();
                     break;
                  case kCanvasOptionResizeCanvas:
                     FitCanvas();
                     break;
                  case kCanvasOptionMoveOpaque:
                     if (fCanvas->OpaqueMoving()) {
                        fCanvas->MoveOpaque(0);
                         fOptionMenu->UnCheckEntry(kCanvasOptionMoveOpaque);
                     } else {
                        fCanvas->MoveOpaque(1);
                         fOptionMenu->CheckEntry(kCanvasOptionMoveOpaque);
                     }
                     break;
                  case kCanvasOptionResizeOpaque:
                     if (fCanvas->OpaqueResizing()) {
                        fCanvas->ResizeOpaque(0);
                         fOptionMenu->UnCheckEntry(kCanvasOptionResizeOpaque);
                     } else {
                        fCanvas->ResizeOpaque(1);
                        fOptionMenu->CheckEntry(kCanvasOptionResizeOpaque);
                     }
                     break;
                  case kCanvasOptionInterrupt:
                     gROOT->SetInterrupt();
                     break;
                  case kCanvasOptionRefresh:
                     fCanvas->Paint();
                     fCanvas->Update();
                     break;
                  case kCanvasOptionStatistics:
                     if (gStyle->GetOptStat()) {
                        gStyle->SetOptStat(0);
                        delete gPad->FindObject("stats");
                         fOptionMenu->UnCheckEntry(kCanvasOptionStatistics);
                     } else {
                        gStyle->SetOptStat(1);
                         fOptionMenu->CheckEntry(kCanvasOptionStatistics);
                     }
                     gPad->Modified();
                     fCanvas->Update();
                     break;
                  case kCanvasOptionHistTitle:
                     if (gStyle->GetOptTitle()) {
                        gStyle->SetOptTitle(0);
                        delete gPad->FindObject("title");
                         fOptionMenu->UnCheckEntry(kCanvasOptionHistTitle);
                     } else {
                        gStyle->SetOptTitle(1);
                         fOptionMenu->CheckEntry(kCanvasOptionHistTitle);
                     }
                     gPad->Modified();
                     fCanvas->Update();
                     break;
                  case kCanvasOptionFitParams:
                     if (gStyle->GetOptFit()) {
                        gStyle->SetOptFit(0);
                         fOptionMenu->UnCheckEntry(kCanvasOptionFitParams);
                     } else {
                        gStyle->SetOptFit(1);
                         fOptionMenu->CheckEntry(kCanvasOptionFitParams);
                     }
                     gPad->Modified();
                     fCanvas->Update();
                     break;
                  case kCanvasOptionCanEdit:
                     if (gROOT->GetEditHistograms()) {
                        gROOT->SetEditHistograms(kFALSE);
                         fOptionMenu->UnCheckEntry(kCanvasOptionCanEdit);
                     } else {
                        gROOT->SetEditHistograms(kTRUE);
                         fOptionMenu->CheckEntry(kCanvasOptionCanEdit);
                     }
                     break;

                  // Handle Tools menu items...
                  case kCanvasInspectRoot:
                     fCanvas->cd();
                     gROOT->Inspect();
                     fCanvas->Update();
                     break;
                  case kCanvasToolsBrowser:
                     new TBrowser("browser");
                     break;
                  case kCanvasToolsBuilder:
                     TGuiBuilder::Instance();
                     break;
                  case kCanvasToolsRecorder:
                     gROOT->ProcessLine("new TGRecorder()");
                     break;

                  // Handle Tools menu items...
                  case kCanvasClassesTree:
                     {
                        TString cdef;
                        lc = (TList*)gROOT->GetListOfCanvases();
                        if (lc->FindObject("ClassTree")) {
                           cdef = TString::Format("ClassTree_%d", lc->GetSize()+1);
                        } else {
                           cdef = "ClassTree";
                        }
                        new TClassTree(cdef.Data(), "TObject");
                        fCanvas->Update();
                     }
                     break;

                case kCanvasFitPanel:
                     {
                        // use plugin manager to create instance of TFitEditor
                        TPluginHandler *handler = gROOT->GetPluginManager()->FindHandler("TFitEditor");
                        if (handler && handler->LoadPlugin() != -1) {
                           if (handler->ExecPlugin(2, fCanvas, 0) == 0)
                              Error("FitPanel", "Unable to crate the FitPanel");
                        }
                        else
                           Error("FitPanel", "Unable to find the FitPanel plug-in");
                     }
                     break;

                  // Handle Help menu items...
                  case kCanvasHelpAbout:
                     {
#ifdef WIN32
                        new TWin32SplashThread(kTRUE);
#else

                        char str[32];
                        snprintf(str, 32, "About ROOT %s...", gROOT->GetVersion());
                        hd = new TRootHelpDialog(this, str, 600, 400);
                        hd->SetText(gHelpAbout);
                        hd->Popup();
#endif
                     }
                     break;
                  case kCanvasHelpOnCanvas:
                     hd = new TRootHelpDialog(this, "Help on Canvas...", 600, 400);
                     hd->SetText(gHelpCanvas);
                     hd->Popup();
                     break;
                  case kCanvasHelpOnMenus:
                     hd = new TRootHelpDialog(this, "Help on Menus...", 600, 400);
                     hd->SetText(gHelpPullDownMenus);
                     hd->Popup();
                     break;
                  case kCanvasHelpOnGraphicsEd:
                     hd = new TRootHelpDialog(this, "Help on Graphics Editor...", 600, 400);
                     hd->SetText(gHelpGraphicsEditor);
                     hd->Popup();
                     break;
                  case kCanvasHelpOnBrowser:
                     hd = new TRootHelpDialog(this, "Help on Browser...", 600, 400);
                     hd->SetText(gHelpBrowser);
                     hd->Popup();
                     break;
                  case kCanvasHelpOnObjects:
                     hd = new TRootHelpDialog(this, "Help on Objects...", 600, 400);
                     hd->SetText(gHelpObjects);
                     hd->Popup();
                     break;
                  case kCanvasHelpOnPS:
                     hd = new TRootHelpDialog(this, "Help on PostScript...", 600, 400);
                     hd->SetText(gHelpPostscript);
                     hd->Popup();
                     break;
               }
            default:
               break;
         }
      default:
         break;
   }
   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Called by TCanvas ctor to get window indetifier.

Int_t TRootCanvas::InitWindow()
{
   if (fCanvas->OpaqueMoving())
      fOptionMenu->CheckEntry(kCanvasOptionMoveOpaque);
   if (fCanvas->OpaqueResizing())
      fOptionMenu->CheckEntry(kCanvasOptionResizeOpaque);

   return fCanvasID;
}

////////////////////////////////////////////////////////////////////////////////
/// Set size of canvas container. Units in pixels.
/// If w==0 and h==0, set autofit mode

void TRootCanvas::SetCanvasSize(UInt_t w, UInt_t h)
{
   // turn off autofit, we want to stay at the given size
   int opt = fCanvasContainer->GetOptions();
   if (!w && !h) {
      fAutoFit = kTRUE;
      fOptionMenu->CheckEntry(kCanvasOptionAutoResize);
      opt &= ~kFixedSize;    // turn off fixed size mode
   } else {
      fAutoFit = kFALSE;
      fOptionMenu->UnCheckEntry(kCanvasOptionAutoResize);
      opt |= kFixedSize;    // turn on fixed size mode
   }
   fCanvasContainer->ChangeOptions(opt);
   fCanvasContainer->SetWidth(w);
   fCanvasContainer->SetHeight(h);
   Layout();  // force layout (will update container to given size)
   fCanvas->Resize();
   fCanvas->Update();
}

////////////////////////////////////////////////////////////////////////////////
/// Set canvas position (units in pixels).

void TRootCanvas::SetWindowPosition(Int_t x, Int_t y)
{
   Move(x, y);
}

////////////////////////////////////////////////////////////////////////////////
/// Set size of canvas (units in pixels).

void TRootCanvas::SetWindowSize(UInt_t w, UInt_t h)
{
   Resize(w, h);

   // Make sure the change of size is really done.
   gVirtualX->Update(1);
   if (!gThreadXAR) {
      gSystem->Sleep(100);
      gSystem->ProcessEvents();
      gSystem->Sleep(10);
      gSystem->ProcessEvents();
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Put canvas window on top of the window stack.

void TRootCanvas::RaiseWindow()
{
   gVirtualX->RaiseWindow(GetId());
}

////////////////////////////////////////////////////////////////////////////////
/// Change title on window.

void TRootCanvas::SetWindowTitle(const char *title)
{
   SetWindowName(title);
   SetIconName(title);
   fToolDock->SetWindowName(Form("ToolBar: %s", title));
}

////////////////////////////////////////////////////////////////////////////////
/// Fit canvas container to current window size.

void TRootCanvas::FitCanvas()
{
   if (!fAutoFit) {
      int opt = fCanvasContainer->GetOptions();
      int oopt = opt;
      opt &= ~kFixedSize;   // turn off fixed size mode
      fCanvasContainer->ChangeOptions(opt);
      Layout();  // force layout
      fCanvas->Resize();
      fCanvas->Update();
      fCanvasContainer->ChangeOptions(oopt);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Print the canvas.

void TRootCanvas::PrintCanvas()
{
   Int_t ret = 0;
   Bool_t pname = kTRUE;
   char *printer, *printCmd;
   static TString sprinter, sprintCmd;

   if (sprinter == "")
      printer = StrDup(gEnv->GetValue("Print.Printer", ""));
   else
      printer = StrDup(sprinter);
   if (sprintCmd == "")
#ifndef WIN32
      printCmd = StrDup(gEnv->GetValue("Print.Command", ""));
#else
      printCmd = StrDup(gEnv->GetValue("Print.Command", "start AcroRd32.exe /p"));
#endif
   else
      printCmd = StrDup(sprintCmd);

   new TGPrintDialog(fClient->GetDefaultRoot(), this, 400, 150,
                     &printer, &printCmd, &ret);
   if (ret) {
      sprinter  = printer;
      sprintCmd = printCmd;

      if (sprinter == "")
         pname = kFALSE;

      TString fn = "rootprint";
      FILE *f = gSystem->TempFileName(fn, gEnv->GetValue("Print.Directory", gSystem->TempDirectory()));
      if (f) fclose(f);
      fn += TString::Format(".%s",gEnv->GetValue("Print.FileType", "pdf"));
      fCanvas->Print(fn);

      TString cmd = sprintCmd;
      if (cmd.Contains("%p"))
         cmd.ReplaceAll("%p", sprinter);
      else if (pname) {
         cmd += " "; cmd += sprinter; cmd += " ";
      }

      if (cmd.Contains("%f"))
         cmd.ReplaceAll("%f", fn);
      else {
         cmd += " "; cmd += fn; cmd += " ";
      }

      gSystem->Exec(cmd);
#ifndef WIN32
      gSystem->Unlink(fn);
#endif
   }
   delete [] printer;
   delete [] printCmd;
}

////////////////////////////////////////////////////////////////////////////////
/// Display a tooltip with infos about the primitive below the cursor.

void TRootCanvas::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected)
{
   fToolTip->Hide();
   if (!fCanvas->GetShowToolTips() || selected == 0 ||
       event != kMouseMotion || fButton != 0)
      return;
   TString tipInfo;
   TString objInfo = selected->GetObjectInfo(px, py);
   if (objInfo.BeginsWith("-")) {
      // if the string begins with '-', display only the object info
      objInfo.Remove(TString::kLeading, '-');
      tipInfo = objInfo;
   }
   else {
      const char *title = selected->GetTitle();
      tipInfo += TString::Format("%s::%s", selected->ClassName(),
                                 selected->GetName());
      if (title && strlen(title))
         tipInfo += TString::Format("\n%s", selected->GetTitle());
      tipInfo += TString::Format("\n%d, %d", px, py);
      if (!objInfo.IsNull())
         tipInfo += TString::Format("\n%s", objInfo.Data());
   }
   fToolTip->SetText(tipInfo.Data());
   fToolTip->SetPosition(px+15, py+15);
   fToolTip->Reset();
}

////////////////////////////////////////////////////////////////////////////////
/// Show or hide menubar.

void TRootCanvas::ShowMenuBar(Bool_t show)
{
   if (show)  ShowFrame(fMenuBar);
   else       HideFrame(fMenuBar);
}

////////////////////////////////////////////////////////////////////////////////
/// Show or hide statusbar.

void TRootCanvas::ShowStatusBar(Bool_t show)
{
   UInt_t dh = fClient->GetDisplayHeight();
   UInt_t ch = fCanvas->GetWindowHeight();

   UInt_t h = GetHeight();
   UInt_t sh = fStatusBar->GetHeight()+2;

   if (show) {
      ShowFrame(fStatusBar);
      fViewMenu->CheckEntry(kCanvasViewEventStatus);
      if (dh - ch >= sh) h = h + sh;
      else h = ch;
   } else {
      HideFrame(fStatusBar);
      fViewMenu->UnCheckEntry(kCanvasViewEventStatus);
      if (dh - ch < sh) h = ch;
      else h = h - sh;
   }
   Resize(GetWidth(), h);
}

////////////////////////////////////////////////////////////////////////////////
/// Show or hide side frame.

void TRootCanvas::ShowEditor(Bool_t show)
{
   TVirtualPad::TContext ctxt(Canvas(), kFALSE);

   UInt_t w = GetWidth();
   UInt_t e = fEditorFrame->GetWidth();
   UInt_t h = GetHeight();
   UInt_t s = fHorizontal1->GetHeight();

   auto lambda_show = [&, this]() {
      if (show) {
         if (!fEditor)
            CreateEditor();
         TVirtualPadEditor *gged = TVirtualPadEditor::GetPadEditor(kFALSE);
         if (gged && gged->GetCanvas() == fCanvas) {
            gged->Hide();
         }
         if (!fViewMenu->IsEntryChecked(kCanvasViewToolbar) || fToolDock->IsUndocked()) {
            ShowFrame(fHorizontal1);
            h += s;
         }
         fMainFrame->ShowFrame(fEditorFrame);
         fEditor->Show();
         fViewMenu->CheckEntry(kCanvasViewEditor);
         w += e;
      } else {
         if (!fViewMenu->IsEntryChecked(kCanvasViewToolbar) || fToolDock->IsUndocked()) {
            HideFrame(fHorizontal1);
            h -= s;
         }
         if (fEditor)
            fEditor->Hide();
         fMainFrame->HideFrame(fEditorFrame);
         fViewMenu->UnCheckEntry(kCanvasViewEditor);
         w -= e;
      }
   };

   if (fParent && fParent != fClient->GetDefaultRoot()) {
      TGMainFrame *main = (TGMainFrame *)fParent->GetMainFrame();
      fMainFrame->HideFrame(fEditorFrame);
      if (main && main->InheritsFrom("TRootBrowser")) {
         TRootBrowser *browser = (TRootBrowser *)main;
         if (!fEmbedded)
            browser->GetTabRight()->Connect("Selected(Int_t)", "TRootCanvas",
                                            this, "Activated(Int_t)");
         fEmbedded = kTRUE;
         if (show && (!fEditor || !((TGedEditor *)fEditor)->IsMapped())) {
            if (!browser->GetTabLeft()->GetTabTab("Pad Editor")) {
               if (browser->GetActFrame()) { //already in edit mode
                  TTimer::SingleShot(200, "TRootCanvas", this, "ShowEditor(=kTRUE)");
               } else {
                  browser->StartEmbedding(TRootBrowser::kLeft);
                  if (!fEditor)
                     fEditor = TVirtualPadEditor::GetPadEditor(kTRUE);
                  else {
                     ((TGedEditor *)fEditor)->ReparentWindow(fClient->GetRoot());
                     ((TGedEditor *)fEditor)->MapWindow();
                  }
                  browser->StopEmbedding("Pad Editor");
                  if (fEditor) {
                     fEditor->SetGlobal(kFALSE);
                     gROOT->GetListOfCleanups()->Remove((TGedEditor *)fEditor);
                     ((TGedEditor *)fEditor)->SetCanvas(fCanvas);
                     ((TGedEditor *)fEditor)->SetModel(fCanvas, fCanvas, kButton1Down);
                  }
               }
            }
            else
               fEditor = TVirtualPadEditor::GetPadEditor(kFALSE);
         }
         if (show) browser->GetTabLeft()->SetTab("Pad Editor");
      } else {
         lambda_show();
         main->Layout();
      }
   }
   else {
      lambda_show();
      Resize(w, h);
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Create embedded editor.

void TRootCanvas::CreateEditor()
{
   fEditorFrame->SetEditDisabled(kEditEnable);
   fEditorFrame->SetEditable();
   gPad = Canvas();
   // next two lines are related to the old editor
   Int_t show = gEnv->GetValue("Canvas.ShowEditor", 0);
   gEnv->SetValue("Canvas.ShowEditor","true");
   fEditor = TVirtualPadEditor::LoadEditor();
   if (fEditor) fEditor->SetGlobal(kFALSE);
   fEditorFrame->SetEditable(kEditDisable);
   fEditorFrame->SetEditable(kFALSE);

   // next line is related to the old editor
   if (show == 0) gEnv->SetValue("Canvas.ShowEditor","false");
}

////////////////////////////////////////////////////////////////////////////////
/// Show or hide toolbar.

void TRootCanvas::ShowToolBar(Bool_t show)
{
   if (show && !fToolBar) {

      fToolBar = new TGToolBar(fToolDock, 60, 20, kHorizontalFrame);
      fToolDock->AddFrame(fToolBar, fHorizontal1Layout);

      Int_t spacing = 6, i;
      for (i = 0; gCanvasToolBarData[i].fPixmap; i++) {
         if (strlen(gCanvasToolBarData[i].fPixmap) == 0) {
            spacing = 6;
            continue;
         }
         fToolBar->AddButton(this, &gCanvasToolBarData[i], spacing);
         spacing = 0;
      }
      fVertical1 = new TGVertical3DLine(fToolBar);
      fVertical2 = new TGVertical3DLine(fToolBar);
      fVertical1Layout = new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 4,2,0,0);
      fVertical2Layout = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
      fToolBar->AddFrame(fVertical1, fVertical1Layout);
      fToolBar->AddFrame(fVertical2, fVertical2Layout);

      spacing = 6;
      for (i = 0; gToolBarData1[i].fPixmap; i++) {
         if (strlen(gToolBarData1[i].fPixmap) == 0) {
            spacing = 6;
            continue;
         }
         fToolBar->AddButton(this, &gToolBarData1[i], spacing);
         spacing = 0;
      }
      fToolDock->MapSubwindows();
      fToolDock->Layout();
      fToolDock->SetWindowName(Form("ToolBar: %s", GetWindowName()));
      fToolDock->Connect("Docked()", "TRootCanvas", this, "AdjustSize()");
      fToolDock->Connect("Undocked()", "TRootCanvas", this, "AdjustSize()");
   }

   if (!fToolBar) return;

   UInt_t h = GetHeight();
   UInt_t sh = fToolBarSep->GetHeight();
   UInt_t dh = fToolBar->GetHeight();

   if (show) {
      ShowFrame(fToolDock);
      if (!fViewMenu->IsEntryChecked(kCanvasViewEditor)) {
         ShowFrame(fHorizontal1);
         h = h + sh;
      }
      ShowFrame(fToolBarSep);
      fViewMenu->CheckEntry(kCanvasViewToolbar);
      h = h + dh + sh;
   } else {
      if (fToolDock->IsUndocked()) {
         fToolDock->DockContainer();
         h = h + 2*sh;
      } else h = h - dh;

      HideFrame(fToolDock);
      if (!fViewMenu->IsEntryChecked(kCanvasViewEditor)) {
         HideFrame(fHorizontal1);
         h = h - sh;
      }
      HideFrame(fToolBarSep);
      h = h - sh;
      fViewMenu->UnCheckEntry(kCanvasViewToolbar);
   }
   Resize(GetWidth(), h);
}

////////////////////////////////////////////////////////////////////////////////
/// Enable or disable tooltip info.

void TRootCanvas::ShowToolTips(Bool_t show)
{
   if (show)
      fViewMenu->CheckEntry(kCanvasViewToolTips);
   else
      fViewMenu->UnCheckEntry(kCanvasViewToolTips);
}

////////////////////////////////////////////////////////////////////////////////
/// Returns kTRUE if the editor is shown.

Bool_t TRootCanvas::HasEditor() const
{
    return (fEditor) && fViewMenu->IsEntryChecked(kCanvasViewEditor);
}

////////////////////////////////////////////////////////////////////////////////
/// Returns kTRUE if the menu bar is shown.

Bool_t TRootCanvas::HasMenuBar() const
{
   return (fMenuBar) && fMenuBar->IsMapped();
}

////////////////////////////////////////////////////////////////////////////////
/// Returns kTRUE if the status bar is shown.

Bool_t TRootCanvas::HasStatusBar() const
{
   return (fStatusBar) && fStatusBar->IsMapped();
}

////////////////////////////////////////////////////////////////////////////////
/// Returns kTRUE if the tool bar is shown.

Bool_t TRootCanvas::HasToolBar() const
{
   return (fToolBar) && fToolBar->IsMapped();
}

////////////////////////////////////////////////////////////////////////////////
/// Returns kTRUE if the tooltips are enabled.

Bool_t TRootCanvas::HasToolTips() const
{
   return (fCanvas) && fCanvas->GetShowToolTips();
}

////////////////////////////////////////////////////////////////////////////////
/// Keep the same canvas size while docking/undocking toolbar.

void TRootCanvas::AdjustSize()
{
   UInt_t h = GetHeight();
   UInt_t dh = fToolBar->GetHeight();
   UInt_t sh = fHorizontal1->GetHeight();

   if (fToolDock->IsUndocked()) {
      if (!fViewMenu->IsEntryChecked(kCanvasViewEditor)) {
         HideFrame(fHorizontal1);
         h = h - sh;
      }
      HideFrame(fToolBarSep);
      h = h - dh - sh;
   } else {
      if (!fViewMenu->IsEntryChecked(kCanvasViewEditor)) {
         ShowFrame(fHorizontal1);
         h = h + sh;
      }
      ShowFrame(fToolBarSep);
      h = h + dh + sh;
   }
   Resize(GetWidth(), h);
}

////////////////////////////////////////////////////////////////////////////////
/// Handle mouse button events in the canvas container.

Bool_t TRootCanvas::HandleContainerButton(Event_t *event)
{
   Int_t button = event->fCode;
   Int_t x = event->fX;
   Int_t y = event->fY;

   if (event->fType == kButtonPress) {
      if (fToolTip && fCanvas->GetShowToolTips()) {
         fToolTip->Hide();
         gVirtualX->UpdateWindow(0);
         gSystem->ProcessEvents();
      }
      fButton = button;
      if (button == kButton1) {
         if (event->fState & kKeyShiftMask)
            fCanvas->HandleInput(kButton1Shift, x, y);
         else
            fCanvas->HandleInput(kButton1Down, x, y);
      }
      if (button == kButton2)
         fCanvas->HandleInput(kButton2Down, x, y);
      if (button == kButton3) {
         fCanvas->HandleInput(kButton3Down, x, y);
         fButton = 0;  // button up is consumed by TContextMenu
      }

   } else if (event->fType == kButtonRelease) {
      if (button == kButton4)
         fCanvas->HandleInput(kWheelUp, x, y);
      if (button == kButton5)
         fCanvas->HandleInput(kWheelDown, x, y);
      if (button == kButton1)
         fCanvas->HandleInput(kButton1Up, x, y);
      if (button == kButton2)
         fCanvas->HandleInput(kButton2Up, x, y);
      if (button == kButton3)
         fCanvas->HandleInput(kButton3Up, x, y);

      fButton = 0;
   }

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle mouse button double click events in the canvas container.

Bool_t TRootCanvas::HandleContainerDoubleClick(Event_t *event)
{
   Int_t button = event->fCode;
   Int_t x = event->fX;
   Int_t y = event->fY;

   if (button == kButton1)
      fCanvas->HandleInput(kButton1Double, x, y);
   if (button == kButton2)
      fCanvas->HandleInput(kButton2Double, x, y);
   if (button == kButton3)
      fCanvas->HandleInput(kButton3Double, x, y);

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle configure (i.e. resize) event.

Bool_t TRootCanvas::HandleContainerConfigure(Event_t *)
{
   if (fAutoFit) {
      fCanvas->Resize();
      fCanvas->Update();
   }

   if (fCanvas->HasFixedAspectRatio()) {
      // get menu height
      static Int_t dh = 0;
      if (!dh)
         dh = GetHeight() - fCanvasContainer->GetHeight();
      UInt_t h = TMath::Nint(fCanvasContainer->GetWidth()/
                             fCanvas->GetAspectRatio()) + dh;
      SetWindowSize(GetWidth(), h);
   }
   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle keyboard events in the canvas container.

Bool_t TRootCanvas::HandleContainerKey(Event_t *event)
{
   static EGEventType previous_event = kOtherEvent;
   static UInt_t previous_keysym = 0;

   if (event->fType == kGKeyPress) {
      fButton = event->fCode;
      UInt_t keysym;
      char str[2];
      gVirtualX->LookupString(event, str, sizeof(str), keysym);

      if (str[0] == kESC){   // ESC sets the escape flag
         gROOT->SetEscape();
         fCanvas->HandleInput(kButton1Up, 0, 0);
         fCanvas->HandleInput(kMouseMotion, 0, 0);
         gPad->Modified();
         return kTRUE;
      }
      if (str[0] == 3)   // ctrl-c sets the interrupt flag
         gROOT->SetInterrupt();

      // handle arrow keys
      if (keysym > 0x1011 && keysym < 0x1016) {
         Window_t dum1, dum2, wid;
         UInt_t mask = 0;
         Int_t mx, my, tx, ty;
         wid = gVirtualX->GetDefaultRootWindow();
         gVirtualX->QueryPointer(wid, dum1, dum2, mx, my, mx, my, mask);
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(),
                                         fCanvasContainer->GetId(),
                                         mx, my, tx, ty, dum1);
         fCanvas->HandleInput(kArrowKeyPress, tx, ty);
         // handle case where we got consecutive same keypressed events coming
         // from auto-repeat on Windows (as it fires only successive keydown events)
         if ((previous_keysym == keysym) && (previous_event == kGKeyPress)) {
            switch (keysym) {
               case 0x1012: // left
                  gVirtualX->Warp(--mx, my, wid); --tx;
                  break;
               case 0x1013: // up
                  gVirtualX->Warp(mx, --my, wid); --ty;
                  break;
               case 0x1014: // right
                  gVirtualX->Warp(++mx, my, wid); ++tx;
                  break;
               case 0x1015: // down
                  gVirtualX->Warp(mx, ++my, wid); ++ty;
                  break;
               default:
                  break;
            }
            fCanvas->HandleInput(kArrowKeyRelease, tx, ty);
         }
         previous_keysym = keysym;
      }
      else {
         fCanvas->HandleInput(kKeyPress, str[0], keysym);
      }
   } else if (event->fType == kKeyRelease) {
      UInt_t keysym;
      char str[2];
      gVirtualX->LookupString(event, str, sizeof(str), keysym);

      if (keysym > 0x1011 && keysym < 0x1016) {
         Window_t dum1, dum2, wid;
         UInt_t mask = 0;
         Int_t mx, my, tx, ty;
         wid = gVirtualX->GetDefaultRootWindow();
         gVirtualX->QueryPointer(wid, dum1, dum2, mx, my, mx, my, mask);
         switch (keysym) {
            case 0x1012: // left
               gVirtualX->Warp(--mx, my, wid);
               break;
            case 0x1013: // up
               gVirtualX->Warp(mx, --my, wid);
               break;
            case 0x1014: // right
               gVirtualX->Warp(++mx, my, wid);
               break;
            case 0x1015: // down
               gVirtualX->Warp(mx, ++my, wid);
               break;
            default:
               break;
         }
         gVirtualX->TranslateCoordinates(gClient->GetDefaultRoot()->GetId(),
                                         fCanvasContainer->GetId(),
                                         mx, my, tx, ty, dum1);
         fCanvas->HandleInput(kArrowKeyRelease, tx, ty);
         previous_keysym = keysym;
      }
      fButton = 0;
   }
   previous_event = event->fType;
   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle mouse motion event in the canvas container.

Bool_t TRootCanvas::HandleContainerMotion(Event_t *event)
{
   Int_t x = event->fX;
   Int_t y = event->fY;

   if (fButton == 0)
      fCanvas->HandleInput(kMouseMotion, x, y);
   if (fButton == kButton1) {
      if (event->fState & kKeyShiftMask)
         fCanvas->HandleInput(EEventType(8), x, y);
      else
         fCanvas->HandleInput(kButton1Motion, x, y);
   }
   if (fButton == kButton2)
      fCanvas->HandleInput(kButton2Motion, x, y);

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle expose events.

Bool_t TRootCanvas::HandleContainerExpose(Event_t *event)
{
   if (event->fCount == 0) {
      fCanvas->Flush();
   }

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle enter/leave events. Only leave is activated at the moment.

Bool_t TRootCanvas::HandleContainerCrossing(Event_t *event)
{
   Int_t x = event->fX;
   Int_t y = event->fY;

   // pointer grabs create also an enter and leave event but with fCode
   // either kNotifyGrab or kNotifyUngrab, don't propagate these events
   if (event->fType == kLeaveNotify && event->fCode == kNotifyNormal)
      fCanvas->HandleInput(kMouseLeave, x, y);

   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle drop events.

Bool_t TRootCanvas::HandleDNDDrop(TDNDData *data)
{
   static Atom_t rootObj  = gVirtualX->InternAtom("application/root", kFALSE);
   static Atom_t uriObj  = gVirtualX->InternAtom("text/uri-list", kFALSE);

   if (data->fDataType == rootObj) {
      TBufferFile buf(TBuffer::kRead, data->fDataLength, (void *)data->fData);
      buf.SetReadMode();
      TObject *obj = (TObject *)buf.ReadObjectAny(TObject::Class());
      if (!obj) return kTRUE;
      gPad->Clear();
      if (obj->InheritsFrom("TKey")) {
         TObject *object = (TObject *)gROOT->ProcessLine(Form("((TKey *)0x%zx)->ReadObj();", (size_t)obj));
         if (!object) return kTRUE;
         if (object->InheritsFrom("TGraph"))
            object->Draw("ALP");
         else if (object->InheritsFrom("TImage"))
            object->Draw("x");
         else if (object->IsA()->GetMethodAllAny("Draw"))
            object->Draw();
      }
      else if (obj->InheritsFrom("TGraph"))
         obj->Draw("ALP");
      else if (obj->IsA()->GetMethodAllAny("Draw"))
         obj->Draw();
      gPad->Modified();
      gPad->Update();
      return kTRUE;
   }
   else if (data->fDataType == uriObj) {
      TString sfname((char *)data->fData);
      if (sfname.Length() > 7) {
         sfname.ReplaceAll("\r\n", "");
         TUrl uri(sfname.Data());
         if (sfname.EndsWith(".bmp") ||
            sfname.EndsWith(".gif") ||
            sfname.EndsWith(".jpg") ||
            sfname.EndsWith(".png") ||
            sfname.EndsWith(".ps")  ||
            sfname.EndsWith(".eps") ||
            sfname.EndsWith(".pdf") ||
            sfname.EndsWith(".tiff") ||
            sfname.EndsWith(".xpm")) {
            TImage *img = TImage::Open(uri.GetFile());
            if (img) {
               img->Draw("x");
               img->SetEditable(kTRUE);
            }
         }
         gPad->Modified();
         gPad->Update();
      }
   }
   return kFALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle dragging position events.

Atom_t TRootCanvas::HandleDNDPosition(Int_t x, Int_t y, Atom_t action,
                                      Int_t /*xroot*/, Int_t /*yroot*/)
{
   TPad *pad = fCanvas->Pick(x, y, 0);
   if (pad) {
      pad->cd();
      gROOT->SetSelectedPad(pad);
      // make sure the pad is highlighted (on Windows)
      pad->Update();
   }
   return action;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle drag enter events.

Atom_t TRootCanvas::HandleDNDEnter(Atom_t *typelist)
{
   static Atom_t rootObj  = gVirtualX->InternAtom("application/root", kFALSE);
   static Atom_t uriObj  = gVirtualX->InternAtom("text/uri-list", kFALSE);
   Atom_t ret = kNone;
   for (int i = 0; typelist[i] != kNone; ++i) {
      if (typelist[i] == rootObj)
         ret = rootObj;
      if (typelist[i] == uriObj)
         ret = uriObj;
   }
   return ret;
}

////////////////////////////////////////////////////////////////////////////////
/// Handle drag leave events.

Bool_t TRootCanvas::HandleDNDLeave()
{
   return kTRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// Slot handling tab switching in the browser, to properly set the canvas
/// and the model to the editor.

void TRootCanvas::Activated(Int_t id)
{
   if (fEmbedded) {
      TGTab *sender = (TGTab *)gTQSender;
      if (sender) {
         TGCompositeFrame *cont = sender->GetTabContainer(id);
         if (cont == fParent) {
            if (!fEditor)
               fEditor = TVirtualPadEditor::GetPadEditor(kFALSE);
            if (fEditor && ((TGedEditor *)fEditor)->IsMapped()) {
               ((TGedEditor *)fEditor)->SetCanvas(fCanvas);
               ((TGedEditor *)fEditor)->SetModel(fCanvas, fCanvas, kButton1Down);
            }
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
/// Save a canvas container as a C++ statement(s) on output stream out.

void TRootContainer::SavePrimitive(std::ostream &out, Option_t * /*= ""*/)
{
   out << "\n   // canvas container\n";
   out << "   Int_t canvasID = gVirtualX->InitWindow((ULongptr_t)" << GetParent()->GetParent()->GetName()
       << "->GetId());\n";
   out << "   Window_t winC = gVirtualX->GetWindowID(canvasID);\n";
   out << "   TGCompositeFrame *" << GetName() << " = new TGCompositeFrame(gClient, winC, " << GetParent()->GetName()
       << ");\n";
}
