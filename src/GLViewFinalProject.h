#pragma once

#include "GLView.h"

namespace Aftr
{
   class Camera;

/**
   \class GLViewFinalProject
   \author Scott Nykl 
   \brief A child of an abstract GLView. This class is the top-most manager of the module.

   Read \see GLView for important constructor and init information.

   \see GLView

    \{
*/

class GLViewFinalProject : public GLView
{
public:
   static GLViewFinalProject* New( const std::vector< std::string >& outArgs );
   virtual ~GLViewFinalProject();
   virtual void updateWorld(); ///< Called once per frame
   virtual void loadMap(); ///< Called once at startup to build this module's scene
   virtual void createFinalProjectWayPoints();
   virtual void onResizeWindow( GLsizei width, GLsizei height );
   virtual void onMouseDown( const SDL_MouseButtonEvent& e );
   virtual void onMouseUp( const SDL_MouseButtonEvent& e );
   virtual void onMouseMove( const SDL_MouseMotionEvent& e );
   virtual void onKeyDown( const SDL_KeyboardEvent& key );
   virtual void onKeyUp( const SDL_KeyboardEvent& key );
   void initChunks();
   void updateTerrain();
   void addChunksObjs(int ID);
   bool isNewRender();

protected:
   GLViewFinalProject( const std::vector< std::string >& args );
   virtual void onCreate();   

   std::vector<int> terrainPlanes;
   std::map<int, std::vector<int>> terrainWOs;
   bool gameIsRunning = false;
   bool isMovingLeft = false;
   bool isMovingRight = false;
   bool isJumping = false;
   bool isFalling = false;
   bool isSliding = false;
   int slideCount = 0;
   float jumpApex = 0;
   WO* snowboardWO;
   WO* griffWO;
};

/** \} */

} //namespace Aftr
