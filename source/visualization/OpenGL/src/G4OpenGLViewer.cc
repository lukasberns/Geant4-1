//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4OpenGLViewer.cc,v 1.63 2010-10-05 15:45:19 lgarnier Exp $
// GEANT4 tag $Name: not supported by cvs2svn $
//
// 
// Andrew Walkden  27th March 1996
// OpenGL view - opens window, hard copy, etc.

#ifdef G4VIS_BUILD_OPENGL_DRIVER

#include "G4ios.hh"
#include "G4OpenGLViewer.hh"
#include "G4OpenGLSceneHandler.hh"
#include "G4OpenGLTransform3D.hh"
#include "G4OpenGL2PSAction.hh"

#include "G4Scene.hh"
#include "G4VisExtent.hh"
#include "G4LogicalVolume.hh"
#include "G4VSolid.hh"
#include "G4Point3D.hh"
#include "G4Normal3D.hh"
#include "G4Plane3D.hh"
#include "G4AttHolder.hh"
#include "G4AttCheck.hh"

// GL2PS
#include "Geant4_gl2ps.h"

#include <sstream>

G4OpenGLViewer::G4OpenGLViewer (G4OpenGLSceneHandler& scene):
G4VViewer (scene, -1),
fPrintColour (true),
fVectoredPs (true),
fOpenGLSceneHandler(scene),
background (G4Colour(0.,0.,0.)),
transparency_enabled (true),
antialiasing_enabled (false),
haloing_enabled (false),
fStartTime(-DBL_MAX),
fEndTime(DBL_MAX),
fFadeFactor(0.),
fDisplayHeadTime(false),
fDisplayHeadTimeX(-0.9),
fDisplayHeadTimeY(-0.9),
fDisplayHeadTimeSize(24.),
fDisplayHeadTimeRed(0.),
fDisplayHeadTimeGreen(1.),
fDisplayHeadTimeBlue(1.),
fDisplayLightFront(false),
fDisplayLightFrontX(0.),
fDisplayLightFrontY(0.),
fDisplayLightFrontZ(0.),
fDisplayLightFrontT(0.),
fDisplayLightFrontRed(0.),
fDisplayLightFrontGreen(1.),
fDisplayLightFrontBlue(0.),
fPrintSizeX(-1),
fPrintSizeY(-1),
fPrintFilename ("G4OpenGL"),
fPrintFilenameIndex(0),
fPointSize (0),
fSizeHasChanged(0)
{
  // Make changes to view parameters for OpenGL...
  fVP.SetAutoRefresh(true);
  fDefaultVP.SetAutoRefresh(true);

  fGL2PSAction = new G4OpenGL2PSAction();

  //  glClearColor (0.0, 0.0, 0.0, 0.0);
  //  glClearDepth (1.0);
  //  glDisable (GL_BLEND);
  //  glDisable (GL_LINE_SMOOTH);
  //  glDisable (GL_POLYGON_SMOOTH);

}

G4OpenGLViewer::~G4OpenGLViewer () {}

void G4OpenGLViewer::InitializeGLView () 
{
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glClearDepth (1.0);
  glDisable (GL_BLEND);
  glDisable (GL_LINE_SMOOTH);
  glDisable (GL_POLYGON_SMOOTH);

  fWinSize_x = fVP.GetWindowSizeHintX();
  fWinSize_y = fVP.GetWindowSizeHintY();
}  

void G4OpenGLViewer::ClearView () {
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::ClearView\n");
#endif
  glClearColor (background.GetRed(),
		background.GetGreen(),
		background.GetBlue(),
		1.);
  glClearDepth (1.0);
  //Below line does not compile with Mesa includes. 
  //glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
  glClear (GL_COLOR_BUFFER_BIT);
  glClear (GL_DEPTH_BUFFER_BIT);
  glClear (GL_STENCIL_BUFFER_BIT);
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::ClearView flush\n");
#endif
  glFlush ();
}


void G4OpenGLViewer::ResizeWindow(unsigned int aWidth, unsigned int aHeight) {
  if ((fWinSize_x != aWidth) || (fWinSize_y != aHeight)) {
    fWinSize_x = aWidth;
    fWinSize_y = aHeight;
    fSizeHasChanged = true;
  } else {
    fSizeHasChanged = false;
  }
}

/**
 * Set the viewport of the scene
 * MAXIMUM SIZE is :
 * GLint dims[2];
 * glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
 */
void G4OpenGLViewer::ResizeGLView()
{
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::ResizeGLView %d %d &:%d\n",fWinSize_x,fWinSize_y,this);
#endif
  // Check size
  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);

  if ((dims[0] !=0 ) && (dims[1] !=0)) {

    if (fWinSize_x > (unsigned)dims[0]) {
      G4cerr << "Try to resize view greater than max X viewport dimension. Desired size "<<dims[0] <<" is resize to "<<  dims[0] << G4endl;
      fWinSize_x = dims[0];
    }
    if (fWinSize_y > (unsigned)dims[1]) {
      G4cerr << "Try to resize view greater than max Y viewport dimension. Desired size "<<dims[0] <<" is resize to "<<  dims[1] << G4endl;
      fWinSize_y = dims[1];
    }
  }
    
  glViewport(0, 0, fWinSize_x,fWinSize_y);   


}


void G4OpenGLViewer::SetView () {

  if (!fSceneHandler.GetScene()) {
    return;
  }
  // Calculates view representation based on extent of object being
  // viewed and (initial) viewpoint.  (Note: it can change later due
  // to user interaction via visualization system's GUI.)
  
  // Lighting.
  GLfloat lightPosition [4];
  lightPosition [0] = fVP.GetActualLightpointDirection().x();
  lightPosition [1] = fVP.GetActualLightpointDirection().y();
  lightPosition [2] = fVP.GetActualLightpointDirection().z();
  lightPosition [3] = 0.;
  // Light position is "true" light direction, so must come after gluLookAt.
  GLfloat ambient [] = { 0.2, 0.2, 0.2, 1.};
  GLfloat diffuse [] = { 0.8, 0.8, 0.8, 1.};
  glEnable (GL_LIGHT0);
  glLightfv (GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuse);
  
  G4double ratioX = 1;
  G4double ratioY = 1;
  if (fWinSize_y > fWinSize_x) {
    ratioX = ((G4double)fWinSize_y) / ((G4double)fWinSize_x);
  }
  if (fWinSize_x > fWinSize_y) {
    ratioY = ((G4double)fWinSize_x) / ((G4double)fWinSize_y);
  }
  
  // Get radius of scene, etc.
  // Note that this procedure properly takes into account zoom, dolly and pan.
  const G4Point3D targetPoint
    = fSceneHandler.GetScene()->GetStandardTargetPoint()
    + fVP.GetCurrentTargetPoint ();
  G4double radius = fSceneHandler.GetScene()->GetExtent().GetExtentRadius();
  if(radius<=0.) radius = 1.;
  const G4double cameraDistance = fVP.GetCameraDistance (radius);
  const G4Point3D cameraPosition =
    targetPoint + cameraDistance * fVP.GetViewpointDirection().unit();
  const GLdouble pnear  = fVP.GetNearDistance (cameraDistance, radius);
  const GLdouble pfar   = fVP.GetFarDistance  (cameraDistance, pnear, radius);
  const GLdouble right  = fVP.GetFrontHalfHeight (pnear, radius) * ratioY;
  const GLdouble left   = -right;
  const GLdouble top    = fVP.GetFrontHalfHeight (pnear, radius) * ratioX;
  const GLdouble bottom = -top;
  
  // FIXME
  ResizeGLView();
  //SHOULD SetWindowsSizeHint()...

  glMatrixMode (GL_PROJECTION); // set up Frustum.
  glLoadIdentity();

  const G4Vector3D scaleFactor = fVP.GetScaleFactor();
  glScaled(scaleFactor.x(),scaleFactor.y(),scaleFactor.z());
  
  if (fVP.GetFieldHalfAngle() == 0.) {
    glOrtho (left, right, bottom, top, pnear, pfar);
  }
  else {
    glFrustum (left, right, bottom, top, pnear, pfar);
  }  

  glMatrixMode (GL_MODELVIEW); // apply further transformations to scene.
  glLoadIdentity();
  
  const G4Normal3D& upVector = fVP.GetUpVector ();  
  G4Point3D gltarget;
  if (cameraDistance > 1.e-6 * radius) {
    gltarget = targetPoint;
  }
  else {
    gltarget = targetPoint - radius * fVP.GetViewpointDirection().unit();
  }

  const G4Point3D& pCamera = cameraPosition;  // An alias for brevity.
  gluLookAt (pCamera.x(),  pCamera.y(),  pCamera.z(),       // Viewpoint.
	     gltarget.x(), gltarget.y(), gltarget.z(),      // Target point.
	     upVector.x(), upVector.y(), upVector.z());     // Up vector.

  // Light position is "true" light direction, so must come after gluLookAt.
  glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);

  // OpenGL no longer seems to reconstruct clipped edges, so, when the
  // BooleanProcessor is up to it, abandon this and use generic
  // clipping in G4OpenGLSceneHandler::CreateSectionPolyhedron.  Also,
  // force kernel visit on change of clipping plane in
  // G4OpenGLStoredViewer::CompareForKernelVisit.
  //if (fVP.IsSection () ) {  // pair of back to back clip planes.
  if (false) {  // pair of back to back clip planes.
    const G4Plane3D& s = fVP.GetSectionPlane ();
    double sArray[4];
    sArray[0] = s.a();
    sArray[1] = s.b();
    sArray[2] = s.c();
    sArray[3] = s.d() + radius * 1.e-05;
    glClipPlane (GL_CLIP_PLANE0, sArray);
    glEnable (GL_CLIP_PLANE0);
    sArray[0] = -s.a();
    sArray[1] = -s.b();
    sArray[2] = -s.c();
    sArray[3] = -s.d() + radius * 1.e-05;
    glClipPlane (GL_CLIP_PLANE1, sArray);
    glEnable (GL_CLIP_PLANE1);
  } else {
    glDisable (GL_CLIP_PLANE0);
    glDisable (GL_CLIP_PLANE1);
  }

  const G4Planes& cutaways = fVP.GetCutawayPlanes();
  size_t nPlanes = cutaways.size();
  if (fVP.IsCutaway() &&
      fVP.GetCutawayMode() == G4ViewParameters::cutawayIntersection &&
      nPlanes > 0) {
    double a[4];
    a[0] = cutaways[0].a();
    a[1] = cutaways[0].b();
    a[2] = cutaways[0].c();
    a[3] = cutaways[0].d();
    glClipPlane (GL_CLIP_PLANE2, a);
    glEnable (GL_CLIP_PLANE2);
    if (nPlanes > 1) {
      a[0] = cutaways[1].a();
      a[1] = cutaways[1].b();
      a[2] = cutaways[1].c();
      a[3] = cutaways[1].d();
      glClipPlane (GL_CLIP_PLANE3, a);
      glEnable (GL_CLIP_PLANE3);
    }
    if (nPlanes > 2) {
      a[0] = cutaways[2].a();
      a[1] = cutaways[2].b();
      a[2] = cutaways[2].c();
      a[3] = cutaways[2].d();
      glClipPlane (GL_CLIP_PLANE4, a);
      glEnable (GL_CLIP_PLANE4);
    }
  } else {
    glDisable (GL_CLIP_PLANE2);
    glDisable (GL_CLIP_PLANE3);
    glDisable (GL_CLIP_PLANE4);
  }

  // Background.
  background = fVP.GetBackgroundColour ();

}

void G4OpenGLViewer::HaloingFirstPass () {
  
  //To perform haloing, first Draw all information to the depth buffer
  //alone, using a chunky line width, and then Draw all info again, to
  //the colour buffer, setting a thinner line width an the depth testing 
  //function to less than or equal, so if two lines cross, the one 
  //passing behind the other will not pass the depth test, and so not
  //get rendered either side of the infront line for a short distance.

  //First, disable writing to the colo(u)r buffer...
  glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  //Now enable writing to the depth buffer...
  glDepthMask (GL_TRUE);
  glDepthFunc (GL_LESS);
  glClearDepth (1.0);

  //Finally, set the line width to something wide...
  glLineWidth (3.0);

}

void G4OpenGLViewer::HaloingSecondPass () {

  //And finally, turn the colour buffer back on with a sesible line width...
  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthFunc (GL_LEQUAL);
  glLineWidth (1.0);

}

void G4OpenGLViewer::Pick(GLdouble x, GLdouble y)
{
  //G4cout << "X: " << x << ", Y: " << y << G4endl;
  const G4int BUFSIZE = 512;
  GLuint selectBuffer[BUFSIZE];
  glSelectBuffer(BUFSIZE, selectBuffer);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);
  glMatrixMode(GL_PROJECTION);
  G4double currentProjectionMatrix[16];
  glGetDoublev(GL_PROJECTION_MATRIX, currentProjectionMatrix);
  glPushMatrix();
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  // Define 5x5 pixel pick area
  gluPickMatrix(x, viewport[3] - y, 5., 5., viewport);
  glMultMatrixd(currentProjectionMatrix);
  glMatrixMode(GL_MODELVIEW);
  DrawView();
  GLint hits = glRenderMode(GL_RENDER);
  if (hits < 0)
    G4cout << "Too many hits.  Zoom in to reduce overlaps." << G4endl;
  else if (hits > 0) {
    //G4cout << hits << " hit(s)" << G4endl;
    GLuint* p = selectBuffer;
    for (GLint i = 0; i < hits; ++i) {
      GLuint nnames = *p++;
      p++; //OR GLuint zmin = *p++;
      p++; //OR GLuint zmax = *p++;
      //G4cout << "Hit " << i << ": " << nnames << " names"
      //     << "\nzmin: " << zmin << ", zmax: " << zmax << G4endl;
      for (GLuint j = 0; j < nnames; ++j) {
	GLuint name = *p++;
	//G4cout << "Name " << j << ": PickName: " << name << G4endl;
	std::map<GLuint, G4AttHolder*>::iterator iter =
	  fOpenGLSceneHandler.fPickMap.find(name);
	if (iter != fOpenGLSceneHandler.fPickMap.end()) {
	  G4AttHolder* attHolder = iter->second;
	  if(attHolder && attHolder->GetAttDefs().size()) {
	    for (size_t i = 0; i < attHolder->GetAttDefs().size(); ++i) {
	      G4cout << G4AttCheck(attHolder->GetAttValues()[i],
				   attHolder->GetAttDefs()[i]);
	    }
	  }
	}
      }
      G4cout << G4endl;
    }
  }
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}




GLubyte* G4OpenGLViewer::grabPixels (int inColor, unsigned int width, unsigned int height) {
  
  GLubyte* buffer;
  GLint swapbytes, lsbfirst, rowlength;
  GLint skiprows, skippixels, alignment;
  GLenum format;
  int size;

  if (inColor) {
    format = GL_RGB;
    size = width*height*3;
  } else {
    format = GL_LUMINANCE;
    size = width*height*1;
  }

  buffer = new GLubyte[size];
  if (buffer == NULL)
    return NULL;

  glGetIntegerv (GL_UNPACK_SWAP_BYTES, &swapbytes);
  glGetIntegerv (GL_UNPACK_LSB_FIRST, &lsbfirst);
  glGetIntegerv (GL_UNPACK_ROW_LENGTH, &rowlength);

  glGetIntegerv (GL_UNPACK_SKIP_ROWS, &skiprows);
  glGetIntegerv (GL_UNPACK_SKIP_PIXELS, &skippixels);
  glGetIntegerv (GL_UNPACK_ALIGNMENT, &alignment);

  glPixelStorei (GL_UNPACK_SWAP_BYTES, GL_FALSE);
  glPixelStorei (GL_UNPACK_LSB_FIRST, GL_FALSE);
  glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);

  glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
  glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

  glReadBuffer(GL_FRONT);
  glReadPixels (0, 0, (GLsizei)width, (GLsizei)height, format, GL_UNSIGNED_BYTE, (GLvoid*) buffer);

  glPixelStorei (GL_UNPACK_SWAP_BYTES, swapbytes);
  glPixelStorei (GL_UNPACK_LSB_FIRST, lsbfirst);
  glPixelStorei (GL_UNPACK_ROW_LENGTH, rowlength);
  
  glPixelStorei (GL_UNPACK_SKIP_ROWS, skiprows);
  glPixelStorei (GL_UNPACK_SKIP_PIXELS, skippixels);
  glPixelStorei (GL_UNPACK_ALIGNMENT, alignment);
  
  return buffer;
}

void G4OpenGLViewer::printEPS() {
  bool res;
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::printEPS file:%s Vec:%d Name:%s\n",getRealPrintFilename().c_str(),fVectoredPs,GetName().c_str());
#endif

  // Change the LC_NUMERIC value in order to have "." separtor and not ","
  // This case is only useful for French, Canadien...
  char* oldLocale = (char*)(malloc(strlen(setlocale(LC_NUMERIC,NULL))+1));
  if(oldLocale!=NULL) strcpy(oldLocale,setlocale(LC_NUMERIC,NULL));
  setlocale(LC_NUMERIC,"C");

  if (fVectoredPs) {
    res = printVectoredEPS();
  } else {
    res = printNonVectoredEPS();
  }

  // restore the local
  if (oldLocale) {
    setlocale(LC_NUMERIC,oldLocale);
    free(oldLocale);
  }

  if (res == false) {
    G4cerr << "Error while saving file... "<<getRealPrintFilename().c_str()<< G4endl;
  } else {
    G4cout << "File "<<getRealPrintFilename().c_str()<<" has been saved " << G4endl;
  }

  // increment index if necessary
  if ( fPrintFilenameIndex != -1) {
    fPrintFilenameIndex++;
  }

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::printEPS END\n");
#endif
}

bool G4OpenGLViewer::printVectoredEPS() {
  return printGl2PS();
}

bool G4OpenGLViewer::printNonVectoredEPS () {

  int width = getRealPrintSizeX();
  int height = getRealPrintSizeY();

#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::printNonVectoredEPS file:%s Vec:%d X:%d Y:%d col:%d fWinX:%d fWinY:%d\n",getRealPrintFilename().c_str(),fVectoredPs,width,height,fPrintColour,fWinSize_x,fWinSize_y);
#endif
  FILE* fp;
  GLubyte* pixels;
  GLubyte* curpix;
  int components, pos, i;

  pixels = grabPixels (fPrintColour, width, height);

  if (pixels == NULL) {
      G4cerr << "Failed to get pixels from OpenGl viewport" << G4endl;
    return false;
  }
  if (fPrintColour) {
    components = 3;
  } else {
    components = 1;
  }
  std::string name = getRealPrintFilename();
  fp = fopen (name.c_str(), "w");
  if (fp == NULL) {
    G4cerr << "Can't open filename " << name.c_str() << G4endl;
    return false;
  }
  
  fprintf (fp, "%%!PS-Adobe-2.0 EPSF-1.2\n");
  fprintf (fp, "%%%%Title: %s\n", name.c_str());
  fprintf (fp, "%%%%Creator: OpenGL pixmap render output\n");
  fprintf (fp, "%%%%BoundingBox: 0 0 %d %d\n", width, height);
  fprintf (fp, "%%%%EndComments\n");
  fprintf (fp, "gsave\n");
  fprintf (fp, "/bwproc {\n");
  fprintf (fp, "    rgbproc\n");
  fprintf (fp, "    dup length 3 idiv string 0 3 0 \n");
  fprintf (fp, "    5 -1 roll {\n");
  fprintf (fp, "    add 2 1 roll 1 sub dup 0 eq\n");
  fprintf (fp, "    { pop 3 idiv 3 -1 roll dup 4 -1 roll dup\n");
  fprintf (fp, "       3 1 roll 5 -1 roll } put 1 add 3 0 \n");
  fprintf (fp, "    { 2 1 roll } ifelse\n");
  fprintf (fp, "    }forall\n");
  fprintf (fp, "    pop pop pop\n");
  fprintf (fp, "} def\n");
  fprintf (fp, "systemdict /colorimage known not {\n");
  fprintf (fp, "   /colorimage {\n");
  fprintf (fp, "       pop\n");
  fprintf (fp, "       pop\n");
  fprintf (fp, "       /rgbproc exch def\n");
  fprintf (fp, "       { bwproc } image\n");
  fprintf (fp, "   }  def\n");
  fprintf (fp, "} if\n");
  fprintf (fp, "/picstr %d string def\n", width * components);
  fprintf (fp, "%d %d scale\n", width, height);
  fprintf (fp, "%d %d %d\n", width, height, 8);
  fprintf (fp, "[%d 0 0 %d 0 0]\n", width, height);
  fprintf (fp, "{currentfile picstr readhexstring pop}\n");
  fprintf (fp, "false %d\n", components);
  fprintf (fp, "colorimage\n");
  
  curpix = (GLubyte*) pixels;
  pos = 0;
  for (i = width*height*components; i>0; i--) {
    fprintf (fp, "%02hx ", *(curpix++));
    if (++pos >= 32) {
      fprintf (fp, "\n");
      pos = 0; 
    }
  }
  if (pos)
    fprintf (fp, "\n");

  fprintf (fp, "grestore\n");
  fprintf (fp, "showpage\n");
  delete pixels;
  fclose (fp);

  // Reset for next time (useful is size change)
  //  fPrintSizeX = -1;
  //  fPrintSizeY = -1;

  return true;
}


bool G4OpenGLViewer::printGl2PS() {

  int width = getRealPrintSizeX();
  int height = getRealPrintSizeY();

  if (!fGL2PSAction) return false;

  fGL2PSAction->setFileName(getRealPrintFilename().c_str());
  // try to resize
  int X = fWinSize_x;
  int Y = fWinSize_y;

  fWinSize_x = width;
  fWinSize_y = height;
  // Laurent G. 16/03/10 : Not the good way to do. 
  // We should draw in a new offscreen context instead of
  // resizing and drawing in current window...
  // This should be solve when we will do an offscreen method
  // to render OpenGL
  // See : 
  // http://developer.apple.com/Mac/library/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_offscreen/opengl_offscreen.html
  // http://www.songho.ca/opengl/gl_fbo.html

  ResizeGLView();
  if (fGL2PSAction->enableFileWriting()) {

    // Set the viewport
    //    fGL2PSAction->setViewport(0, 0, getRealPrintSizeX(),getRealPrintSizeY());  
    // By default, we choose the line width (trajectories...)
    fGL2PSAction->setLineWidth(1);
    // By default, we choose the point size (markers...)
    fGL2PSAction->setPointSize(2);

    DrawView ();
    fGL2PSAction->disableFileWriting();
  }

  fWinSize_x = X;
  fWinSize_y = Y;
  ResizeGLView();
  DrawView ();

  // Reset for next time (useful is size change)
  //  fPrintSizeX = 0;
  //  fPrintSizeY = 0;

  return true;
}

unsigned int G4OpenGLViewer::getWinWidth() {
  return fWinSize_x;
}

unsigned int G4OpenGLViewer::getWinHeight() {
  return fWinSize_y;
}

G4bool G4OpenGLViewer::sizeHasChanged() {
  return fSizeHasChanged;
}

G4int G4OpenGLViewer::getRealPrintSizeX() {
  if (fPrintSizeX == -1) {
    return fWinSize_x;
  }
  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);

  // L.Garnier 01-2010: Some problems with mac 10.6
  if ((dims[0] !=0 ) && (dims[1] !=0)) {
    if (fPrintSizeX > dims[0]){
      return dims[0];
    }
  }
  if (fPrintSizeX < -1){
    return 0;
  }
  return fPrintSizeX;
}

G4int G4OpenGLViewer::getRealPrintSizeY() {
  if (fPrintSizeY == -1) {
    return fWinSize_y;
  }
  GLint dims[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);

  // L.Garnier 01-2010: Some problems with mac 10.6
  if ((dims[0] !=0 ) && (dims[1] !=0)) {
    if (fPrintSizeY > dims[1]){
      return dims[1];
    }
  }
  if (fPrintSizeY < -1){
    return 0;
  }
  return fPrintSizeY;
}

void G4OpenGLViewer::setPrintSize(G4int X, G4int Y) {
  fPrintSizeX = X;
  fPrintSizeY = Y;
}

void G4OpenGLViewer::setPrintFilename(G4String name,G4bool inc) {
  if (name != "") {
    fPrintFilename = name;
  } else {
    fPrintFilename = "G4OpenGL";  // by default
  }
  if (inc) {
    fPrintFilenameIndex=0;
  } else {
    fPrintFilenameIndex=-1;
  }
}

std::string G4OpenGLViewer::getRealPrintFilename() {
  std::string temp = fPrintFilename;
  if (fPrintFilenameIndex != -1) {
    temp += std::string("_");
    std::ostringstream os;
    os << fPrintFilenameIndex;
    std::string nb_str = os.str();
    temp += nb_str;
  }
  temp += ".eps";
  return temp;
}

GLdouble G4OpenGLViewer::getSceneNearWidth()
{
  if (!fSceneHandler.GetScene()) {
    return 0;
  }
  const G4Point3D targetPoint
    = fSceneHandler.GetScene()->GetStandardTargetPoint()
    + fVP.GetCurrentTargetPoint ();
  G4double radius = fSceneHandler.GetScene()->GetExtent().GetExtentRadius();
  if(radius<=0.) radius = 1.;
  const G4double cameraDistance = fVP.GetCameraDistance (radius);
  const GLdouble pnear   = fVP.GetNearDistance (cameraDistance, radius);
  return 2 * fVP.GetFrontHalfHeight (pnear, radius);
}

GLdouble G4OpenGLViewer::getSceneFarWidth()
{
  if (!fSceneHandler.GetScene()) {
    return 0;
  }
  const G4Point3D targetPoint
    = fSceneHandler.GetScene()->GetStandardTargetPoint()
    + fVP.GetCurrentTargetPoint ();
  G4double radius = fSceneHandler.GetScene()->GetExtent().GetExtentRadius();
  if(radius<=0.) radius = 1.;
  const G4double cameraDistance = fVP.GetCameraDistance (radius);
  const GLdouble pnear   = fVP.GetNearDistance (cameraDistance, radius);
  const GLdouble pfar    = fVP.GetFarDistance  (cameraDistance, pnear, radius);
  return 2 * fVP.GetFrontHalfHeight (pfar, radius);
}


GLdouble G4OpenGLViewer::getSceneDepth()
{
  if (!fSceneHandler.GetScene()) {
    return 0;
  }
  const G4Point3D targetPoint
    = fSceneHandler.GetScene()->GetStandardTargetPoint()
    + fVP.GetCurrentTargetPoint ();
  G4double radius = fSceneHandler.GetScene()->GetExtent().GetExtentRadius();
  if(radius<=0.) radius = 1.;
  const G4double cameraDistance = fVP.GetCameraDistance (radius);
  const GLdouble pnear   = fVP.GetNearDistance (cameraDistance, radius);
  return fVP.GetFarDistance  (cameraDistance, pnear, radius)- pnear;
}



void G4OpenGLViewer::rotateScene(G4double dx, G4double dy,G4double deltaRotation)
{
  if (!fSceneHandler.GetScene()) {
    return;
  }

  G4Vector3D vp;
  G4Vector3D up;
  
  G4Vector3D xprime;
  G4Vector3D yprime;
  G4Vector3D zprime;
  
  G4double delta_alpha;
  G4double delta_theta;
  
  G4Vector3D new_vp;
  G4Vector3D new_up;
  
  G4double cosalpha;
  G4double sinalpha;
  
  G4Vector3D a1;
  G4Vector3D a2;
  G4Vector3D delta;
  G4Vector3D viewPoint;

    
  //phi spin stuff here
  
  vp = fVP.GetViewpointDirection ().unit ();
  up = fVP.GetUpVector ().unit ();
  
  yprime = (up.cross(vp)).unit();
  zprime = (vp.cross(yprime)).unit();
  
  if (fVP.GetLightsMoveWithCamera()) {
    delta_alpha = dy * deltaRotation;
    delta_theta = -dx * deltaRotation;
  } else {
    delta_alpha = -dy * deltaRotation;
    delta_theta = dx * deltaRotation;
  }    
  
  delta_alpha *= deg;
  delta_theta *= deg;
  
  new_vp = std::cos(delta_alpha) * vp + std::sin(delta_alpha) * zprime;
  
  // to avoid z rotation flipping
  // to allow more than 360� rotation

  if (fVP.GetLightsMoveWithCamera()) {
    new_up = (new_vp.cross(yprime)).unit();
    if (new_vp.z()*vp.z() <0) {
      new_up.set(new_up.x(),-new_up.y(),new_up.z());
    }
  } else {
    new_up = up;
    if (new_vp.z()*vp.z() <0) {
      new_up.set(new_up.x(),-new_up.y(),new_up.z());
    }
  }
  fVP.SetUpVector(new_up);
  ////////////////
  // Rotates by fixed azimuthal angle delta_theta.
  
  cosalpha = new_up.dot (new_vp.unit());
  sinalpha = std::sqrt (1. - std::pow (cosalpha, 2));
  yprime = (new_up.cross (new_vp.unit())).unit ();
  xprime = yprime.cross (new_up);
  // Projection of vp on plane perpendicular to up...
  a1 = sinalpha * xprime;
  // Required new projection...
  a2 = sinalpha * (std::cos (delta_theta) * xprime + std::sin (delta_theta) * yprime);
  // Required Increment vector...
  delta = a2 - a1;
  // So new viewpoint is...
  viewPoint = new_vp.unit() + delta;
  
  fVP.SetViewAndLights (viewPoint);
}


void G4OpenGLViewer::rotateSceneInViewDirection(G4double dx, G4double dy,G4double deltaRotation)
{
  if (!fSceneHandler.GetScene()) {
    return;
  }

  G4Vector3D vp;
  G4Vector3D up;
  
  G4Vector3D xprime;
  G4Vector3D yprime;
  G4Vector3D zprime;
  
  G4Vector3D new_vp;
  G4Vector3D new_up;
  
  G4Vector3D a1;
  G4Vector3D a2;
  G4Vector3D delta;
  G4Vector3D viewPoint;

    
  //phi spin stuff here
  
#ifdef G4DEBUG_VIS_OGL
  printf("G4OpenGLViewer::rotateScene dx:%f dy:%f delta:%f\n",dx,dy, deltaRotation);
#endif

  vp = fVP.GetViewpointDirection ().unit();
  up = fVP.GetUpVector ().unit();

  G4Vector3D zPrimeVector = G4Vector3D(up.y()*vp.z()-up.z()*vp.y(),
                             up.z()*vp.x()-up.x()*vp.z(),
                             up.x()*vp.y()-up.y()*vp.x());

  viewPoint = vp/deltaRotation + (zPrimeVector*dx - up*dy) ;
  new_up = G4Vector3D(viewPoint.y()*zPrimeVector.z()-viewPoint.z()*zPrimeVector.y(),
                       viewPoint.z()*zPrimeVector.x()-viewPoint.x()*zPrimeVector.z(),
                       viewPoint.x()*zPrimeVector.y()-viewPoint.y()*zPrimeVector.x());

  G4Vector3D new_upUnit = new_up.unit();
  
  

   fVP.SetUpVector(new_upUnit);
   fVP.SetViewAndLights (viewPoint);
}

#endif