#include "GLViewFinalProject.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include <cmath>

using namespace Aftr;

GLViewFinalProject* GLViewFinalProject::New( const std::vector< std::string >& args )
{
   GLViewFinalProject* glv = new GLViewFinalProject( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewFinalProject::GLViewFinalProject( const std::vector< std::string >& args ) : GLView( args )
{
   //Initialize any member variables that need to be used inside of LoadMap() here.
   //Note: At this point, the Managers are not yet initialized. The Engine initialization
   //occurs immediately after this method returns (see GLViewFinalProject::New() for
   //reference). Then the engine invoke's GLView::loadMap() for this module.
   //After loadMap() returns, GLView::onCreate is finally invoked.

   //The order of execution of a module startup:
   //GLView::New() is invoked:
   //    calls GLView::init()
   //       calls GLView::loadMap() (as well as initializing the engine's Managers)
   //    calls GLView::onCreate()

   //GLViewFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewFinalProject::onCreate()
{
   //GLViewFinalProject::onCreate() is invoked after this module's LoadMap() is completed.
   //At this point, all the managers are initialized. That is, the engine is fully initialized.

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}


GLViewFinalProject::~GLViewFinalProject()
{
   //Implicitly calls GLView::~GLView()
}


void GLViewFinalProject::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.
   
   if (gameIsRunning) {
       Vector boardPos = snowboardWO->getPosition();
       if (isMovingRight) {
           if (boardPos[1] <= -40) {
               isMovingRight = false;
           }
           else {
               snowboardWO->moveRelative(Vector(5, -2, -tan(DEGtoRAD * 15) * 5));
               boardPos = snowboardWO->getPosition();

               griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               this->cam->setCameraLookAtPoint(boardPos);

               if (boardPos[1] == 40 || boardPos[1] == 0) {
                   isMovingRight = false;
               }
           }
       }
       else if (isMovingLeft) {
           if (boardPos[1] >= 40) {
               isMovingLeft = false;
           }
           else {
               snowboardWO->moveRelative(Vector(5, 2, -tan(DEGtoRAD * 15) * 5));
               boardPos = snowboardWO->getPosition();

               griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               this->cam->setCameraLookAtPoint(boardPos);

               if (boardPos[1] == 0 || boardPos[1] == -40) {
                   isMovingLeft = false;
               }
           }
           
       }
       else if (isJumping) {
           if (boardPos[2] >= jumpApex) {
               isJumping = false;
               isFalling = true;
           }
           else {

               snowboardWO->moveRelative(Vector(5, 0, 1));
               boardPos = snowboardWO->getPosition();

               griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               //this->cam->setPosition(boardPos[0] - 40, this->cam->getPosition()[1], this->cam->getPosition()[2]);
           }
       }
       else if (isFalling) {
           std::cout << "isFalling\n";
           WO* plane = worldLst->getWOByID(terrainPlanes[0]);
           Vector planePos = plane->getPosition();
           float boardDistFromCenter = planePos[0] - boardPos[0];
           float heightDiffFromCenter = tan(DEGtoRAD * 15) * boardDistFromCenter;
           std::cout << "INFO\n" << planePos << std::endl << boardDistFromCenter << std::endl << heightDiffFromCenter << std::endl << boardPos[2] << std::endl;
           if (boardPos[2] <= planePos[2] + heightDiffFromCenter + 2) {
               std::cout << "Stopped falling\n";
               snowboardWO->setPosition(boardPos[0], boardPos[1], planePos[2] + heightDiffFromCenter + 2);
               isFalling = false;
           }
           else {
               snowboardWO->moveRelative(Vector(5, 0, -4));
               boardPos = snowboardWO->getPosition();

               griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               //this->cam->setPosition(boardPos[0] - 40, this->cam->getPosition()[1], this->cam->getPosition()[2]);
           }
       }
       else if (isSliding) {
           if (slideCount < 20) {
               if (slideCount == 0) {
                   snowboardWO->rotateAboutRelY(DEGtoRAD * 90);
                   snowboardWO->rotateAboutGlobalY(DEGtoRAD * -90);
                   griffWO->rotateAboutRelZ(DEGtoRAD * 90);
                   //griffWO->rotateAboutRelY(DEGtoRAD * 90);
                   griffWO->rotateAboutGlobalY(DEGtoRAD * -90);
               }
               slideCount++;

               snowboardWO->moveRelative(Vector(5, 0, -tan(DEGtoRAD * 15) * 5));
               boardPos = snowboardWO->getPosition();
               //snowboardWO->setPosition(boardPos[0], boardPos[1], boardPos[2] + 2);

               griffWO->setPosition(boardPos.at(0) - 12, boardPos.at(1), boardPos.at(2) + 6);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               this->cam->setCameraLookAtPoint(boardPos);
           }
           else {
               snowboardWO->rotateToIdentity();
               snowboardWO->rotateAboutGlobalX(DEGtoRAD * -90);
               snowboardWO->rotateAboutGlobalZ(DEGtoRAD * 90);
               snowboardWO->rotateAboutGlobalY(DEGtoRAD * 15);

               griffWO->rotateToIdentity();
               griffWO->rotateAboutGlobalY(DEGtoRAD * 15);

               isSliding = false;
               slideCount = 0;

               snowboardWO->moveRelative(Vector(5, 0, -tan(DEGtoRAD * 15) * 5));
               boardPos = snowboardWO->getPosition();

               griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
               this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
               this->cam->setCameraLookAtPoint(boardPos);
           }        
       }
       else {
           snowboardWO->moveRelative(Vector(5, 0, -tan(DEGtoRAD * 15) * 5));
           boardPos = snowboardWO->getPosition();

           griffWO->setPosition(boardPos.at(0), boardPos.at(1), boardPos.at(2) + 5.5);
           this->cam->setPosition(boardPos[0] - 40, boardPos[1], boardPos[2] + 30);
           this->cam->setCameraLookAtPoint(boardPos);
       }
   }
   if (isNewRender()) {
       updateTerrain();
   }

   if (isColliding()) {
       gameIsRunning = false;
   }

}


void GLViewFinalProject::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewFinalProject::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewFinalProject::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewFinalProject::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewFinalProject::onKeyDown( const SDL_KeyboardEvent& key )
{
   GLView::onKeyDown( key );
   if( key.keysym.sym == SDLK_0 )
      this->setNumPhysicsStepsPerRender( 1 );

   if( key.keysym.sym == SDLK_1 )
   {
       gameIsRunning = true;
       //this->cam->setCameraLookDirection(Vector(1.0, 0.0, 0.0));
       this->cam->setPosition(-20, 3, 8);
       //this->cam->setCameraLookDirection(Vector(0, 0, 0));
   }

   if (key.keysym.sym == SDLK_RIGHT)
   {
       isMovingRight = true;
       isMovingLeft = false;
   }

   if (key.keysym.sym == SDLK_LEFT)
   {
       isMovingLeft = true;
       isMovingRight = false;
   }

   if (key.keysym.sym == SDLK_UP)
   {
       if (!isFalling) {
           jumpApex = snowboardWO->getPosition()[2] + 15;
           isJumping = true;
       }
   }

   if (key.keysym.sym == SDLK_DOWN)
   {
       if (!isSliding) {
           isSliding = true;
       }
   }
}


void GLViewFinalProject::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}

void GLViewFinalProject::onJoyButtonDown(const SDL_JoyButtonEvent& joy) {

    GLView::onJoyButtonDown(joy);

    if (joy.button == 0) {
        isMovingLeft = true;
        isMovingRight = false;
    }
    if (joy.button == 1) {
        isMovingRight = true;
        isMovingLeft = false;
    }
    
    if (joy.button == 2) {
        if (!isFalling) {
            jumpApex = snowboardWO->getPosition()[2] + 15;
            isJumping = true;
        }
    }
    if (joy.button == 3) 
    {
        if (!isSliding) {
            isSliding = true;
        }
    }
    

}


void Aftr::GLViewFinalProject::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = false;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition(-20, 3, 8);

   std::string snowboard(ManagerEnvironmentConfiguration::getLMM() + "/models/snowboard/10535_Snowboard_v1_L3.obj");
   std::string griff(ManagerEnvironmentConfiguration::getLMM() + "/models/griff/griff.obj");
   

   snowboardWO = WO::New(snowboard, Vector(0.1,0.1,0.1), MESH_SHADING_TYPE::mstFLAT);
   snowboardWO->setPosition(0, 0, 1);
   snowboardWO->rotateAboutGlobalX(DEGtoRAD * -90);
   snowboardWO->rotateAboutGlobalZ(DEGtoRAD * 90);
   snowboardWO->rotateAboutGlobalY(DEGtoRAD * 15);
   snowboardWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   
   this->worldLst->push_back(snowboardWO);

   griffWO = WO::New(griff, Vector(0.07, 0.07, 0.07), MESH_SHADING_TYPE::mstFLAT);
   griffWO->setPosition(0, 0, 6.5);
   griffWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   griffWO->rotateAboutGlobalY(DEGtoRAD * 15);
  
   this->worldLst->push_back(griffWO);
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg" );
   //skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

   {
      //Create a light
      float ga = 0.1f; //Global Ambient Light level for this module
      ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
      WOLight* light = WOLight::New();
      light->isDirectionalLight( true );
      light->setPosition( Vector( 0, 0, 100 ) );
      //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
      //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
      light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
      light->setLabel( "Light" );
      worldLst->push_back( light );
   }

   {
      //Create the SkyBox
      WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->setLabel( "Sky Box" );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      worldLst->push_back( wo );
   }

   //{
   //   //Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
   //   WO* wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
   //   ((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
   //   wo->setPosition( Vector(0,0,0) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   //Create the infinite grass plane that uses NVIDIAPhysX(the floor)
   //   WO* wo = WONVStaticPlane::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0, 0 ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 ).getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   //Create the infinite grass plane (the floor)
   //   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0, 50.0f ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0.5f, 75.0f ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue( "sharedmultimediapath" ) + "/models/sphereRp5.wrl", Vector( 1.0f, 1.0f, 1.0f ), mstSMOOTH );
   //   wo->setPosition( 0, 0, 100.0f );
   //   wo->setLabel( "Sphere" );
   //   this->worldLst->push_back( wo );
   //}

   //{
   //   WO* wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
   //   ((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
   //   ((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
   //   wo->setPosition( Vector( 20, 20, 20 ) );
   //   wo->setLabel( "Paladin" );
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo );
   //   netLst->push_back( wo );
   //   this->setActor( wo );
   //}
   //
   //{
   //   WO* wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
   //   wo->setPosition( Vector( 20, 10, 20 ) );
   //   wo->isVisible = false; //hide the WOHuman's bounding box
   //   ((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
   //   wo->setLabel( "Human Cyborg" );
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo ); //Push the WOHuman as an actor
   //   netLst->push_back( wo );
   //   this->setActor( wo ); //Start module where human is the actor
   //}

   //{
   //   //Create and insert the WOWheeledVehicle
   //   std::vector< std::string > wheels;
   //   std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   wheels.push_back( wheelStr );
   //   WO* wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
   //   wo->setPosition( Vector( 5, -15, 20 ) );
   //   wo->setLabel( "Car 1970s Beater" );
   //   ((WOODE*)wo)->mass = 200;
   //   worldLst->push_back( wo );
   //   actorLst->push_back( wo );
   //   this->setActor( wo );
   //   netLst->push_back( wo );
   //}
   
   //Make a Dear Im Gui instance via the WOImGui in the engine... This calls
   //the default Dear ImGui demo that shows all the features... To create your own,
   //inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).
   


   initChunks();
}


void GLViewFinalProject::createFinalProjectWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWayPointSpherical::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}

void GLViewFinalProject::initChunks() {
    std::string ground(ManagerEnvironmentConfiguration::getLMM() + "/models/terrain/snowplane.wrl");

    for (int i = 0; i < 5; i++) {
        WO* plane = WO::New(ground, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
        int planeID = plane->getID();
        if (terrainPlanes.size() > 0) { // use last planes pos
            WO* lastChunk = worldLst->getWOByID(terrainPlanes.at(terrainPlanes.size() - 1));
            auto lastChunkPos = lastChunk->getPosition();
            plane->setPosition(Vector(lastChunkPos[0] + 385, 0, lastChunkPos[2] - 103.2)); // 400 apart interval is fine
        }
        else plane->setPosition(Vector(0, 0, 0));
        plane->rotateAboutRelY(DEGtoRAD * 15);
        plane->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        //plane->upon_async_model_loaded([plane]()
        //    {
        //        ModelMeshSkin& groundSkin = plane->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
        //        groundSkin.getMultiTextureSet().at(0).setTexRepeats(5.0f);
        //        groundSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
        //        groundSkin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
        //        groundSkin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
        //        groundSkin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
        //    });
        worldLst->push_back(plane);
        terrainPlanes.push_back(planeID);
        addChunksObjs(planeID);
    }

}

void GLViewFinalProject::updateTerrain() {
    WO* oldestPlane = worldLst->getWOByID(terrainPlanes.at(0)); // grab chunk furthest behind camera
    WO* newestPlane = worldLst->getWOByID(terrainPlanes.at(terrainPlanes.size()-1)); // grab chunk furthest ahead camera

    auto lastChunkPos = newestPlane->getPosition();
    oldestPlane->setPosition(Vector(lastChunkPos[0] + 385, 0, lastChunkPos[2] - 103.2));
    Vector planePos = oldestPlane->getPosition();

    srand(time(0));

    for (int i = 0; i < terrainWOs[terrainPlanes.at(0)].size(); i++) { // shift furthest back trees to forward most plane
        WO* treeWO = worldLst->getWOByID(terrainWOs[terrainPlanes.at(0)].at(i));
        auto xpos_modifier = ((rand() % 401) - 200);
        if (i % 2 == 0) {
            treeWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + ((rand() % 131) + 70), planePos.at(2) + 7 - xpos_modifier * 0.25); // left side trees
        }
        else {
            treeWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + ((rand() % 131) - 200), planePos.at(2) + 7 - xpos_modifier * 0.25); // right side trees
        }
    }

    int xpos_modifier = -100;
    int totalObstacles = 3;

    for (int i = 0; i < ObstacleWOs[terrainPlanes.at(0)].size(); i++) { // shift furthest back obstacles to forward most plane
        WO* obstacleWO = worldLst->getWOByID(ObstacleWOs[terrainPlanes.at(0)].at(i));
        int laneNum = (rand() % 3) + 1;
        int zShift = 0;

        if (obstacleWO->getLabel() == "rock") zShift = -2;
        else if (obstacleWO->getLabel() == "fence") zShift = -3;
        else if (obstacleWO->getLabel() == "dumpster") zShift = -1;
        else if (obstacleWO->getLabel() == "balloons") zShift = -2;

        if (laneNum == 1) {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + 40, (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // left lane
        }
        else if (laneNum == 2) {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1), (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // middle lane
        }
        else {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) - 40, (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // right lane
        }
        xpos_modifier = xpos_modifier + (400 / totalObstacles);
    }

    int oldestPlaneID = terrainPlanes.at(0);

    for (int i = 0; i < terrainPlanes.size() - 1; i++) { // update terrain plain ID vector to match new positionings
        terrainPlanes.at(i) = terrainPlanes.at(i + 1);
    }
    terrainPlanes.at(terrainPlanes.size() - 1) = oldestPlaneID;
}

void GLViewFinalProject::addChunksObjs(int ID) {
    std::string tree(ManagerEnvironmentConfiguration::getLMM() + "/models/terrain/lowpolytree.obj");
    std::string rock(ManagerEnvironmentConfiguration::getLMM() + "/models/rock/rock.fbx");
    std::string balloons(ManagerEnvironmentConfiguration::getLMM() + "/models/balloons/13498_Balloon_Arch_v1_l2.obj");
    std::string dumpster(ManagerEnvironmentConfiguration::getSMM() + "/models/dumpster-big.wrl");
    std::string fence(ManagerEnvironmentConfiguration::getSMM() + "/models/picketfence.wrl");

    WO* plane = worldLst->getWOByID(ID);
    Vector planePos = plane->getPosition();
    srand(time(0));

    for (int i = 0; i < 50; i++) { // trees
        int treeScale = (rand() % 3) + 6;
        WO* treeWO = WO::New(tree, Vector(treeScale, treeScale, treeScale), MESH_SHADING_TYPE::mstFLAT);
        auto xpos_modifier = ((rand() % 401) - 200);
        if (i % 2 == 0) {
            treeWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + ((rand() % 131) + 70), planePos.at(2) + 7 - xpos_modifier * 0.25); // left side trees
        }
        else {
            treeWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + ((rand() % 131) - 200), planePos.at(2) + 7 - xpos_modifier * 0.25); // right side trees
        }
        treeWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        terrainWOs[ID].push_back(treeWO->getID()); // trees to terrain list
        worldLst->push_back(treeWO);
    }


    int xpos_modifier = -100;
    int totalObstacles = 3;
    for (int i = 0; i < totalObstacles; i++) { // obstacles

        int objType = (rand() % 10) + 1;
        int laneNum = (rand() % 3) + 1;
        int zShift = 0;
        WO* obstacleWO;

        objType = 10;

        if (objType <= 4) {
            obstacleWO = WO::New(rock, Vector(10.0, 10.0, 10.0), MESH_SHADING_TYPE::mstFLAT);
            obstacleWO->setLabel("rock");
            zShift = -2;
        }
        else if (objType > 4 && objType <= 7) {
            obstacleWO = WO::New(fence, Vector(25.0, 25.0, 25.0), MESH_SHADING_TYPE::mstFLAT);
            obstacleWO->setLabel("fence");
            zShift = -3;
        }
        else if (objType > 7 && objType <= 9) {
            obstacleWO = WO::New(dumpster, Vector(15.0, 15.0, 15.0), MESH_SHADING_TYPE::mstFLAT);
            obstacleWO->rotateAboutGlobalY(DEGtoRAD * 180);
            obstacleWO->rotateAboutGlobalZ(DEGtoRAD * 90);
            obstacleWO->setLabel("dumpster");
            zShift = -1;
        }
        else {
            obstacleWO = WO::New(balloons, Vector(0.2, 0.2, 0.2), MESH_SHADING_TYPE::mstFLAT);
            obstacleWO->rotateAboutGlobalZ(DEGtoRAD * -90);
            obstacleWO->rotateAboutGlobalY(DEGtoRAD * 90);
            obstacleWO->setLabel("balloons");
            zShift = -2;
        }

        if (laneNum == 2 && terrainPlanes.size() < 2) laneNum = 1;

        if (laneNum == 1) {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) + 40, (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // left lane
        }
        else if (laneNum == 2) {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) , (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // middle lane
        }
        else {
            obstacleWO->setPosition(planePos.at(0) + xpos_modifier, planePos.at(1) - 40, (planePos.at(2) + 7 - xpos_modifier * 0.25) + zShift); // right lane
        }

        obstacleWO->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        ObstacleWOs[ID].push_back(obstacleWO->getID()); // obstacles to terrain list
        this->worldLst->push_back(obstacleWO);
        xpos_modifier = xpos_modifier + (400 / totalObstacles);
    }

}

bool GLViewFinalProject::isNewRender() {
    WO* oldChunk = worldLst->getWOByID(terrainPlanes.at(1));
    return this->cam->getPosition().at(0) > oldChunk->getPosition().at(0);
}

bool GLViewFinalProject::isColliding() {

    int currentPlane = getCurrentPlane();
    if (currentPlane == -1) return true; // unable to locate current plane therefore end game

    auto grifPos = griffWO->getPosition();

    for (int i = 0; i < ObstacleWOs[currentPlane].size(); i++) { // check all obstacles in current plane
        WO* obstacleWO = worldLst->getWOByID(ObstacleWOs[currentPlane].at(i));
        auto obsPos = obstacleWO->getPosition();

        if (obstacleWO->getLabel() == "rock") {
            if ( (grifPos[0] > obsPos[0] - 15 && grifPos[0] < obsPos[0] + 15) &&    // grif is within bounding xBox
                 (grifPos[1] > obsPos[1] - 15 && grifPos[1] < obsPos[1] + 15) &&    // grif is within bounding yBox
                 (grifPos[2] > obsPos[2] - 15 && grifPos[2] < obsPos[2] + 15))      // grif is within bounding zBox
                 {
                if (gameIsRunning) std::cout << "COLLISION - ROCK\n";
                return true;
            }
        }
        else if (obstacleWO->getLabel() == "fence") {
            if ((grifPos[0] > obsPos[0] - 5 && grifPos[0] < obsPos[0] + 5) &&   
                (grifPos[1] > obsPos[1] - 20 && grifPos[1] < obsPos[1] + 20) &&   
                (grifPos[2] > obsPos[2] - 18 && grifPos[2] < obsPos[2] + 18))
            {
                if (gameIsRunning) std::cout << "COLLISION - FENCE\n";
                return true;
            }
        }
        else if (obstacleWO->getLabel() == "dumpster") {
            if ((grifPos[0] > obsPos[0] - 30 && grifPos[0] < obsPos[0] + 30) &&
                (grifPos[1] > obsPos[1] - 15 && grifPos[1] < obsPos[1] + 15) &&
                (grifPos[2] > obsPos[2] - 20 && grifPos[2] < obsPos[2] + 20))
            {
                if (gameIsRunning) std::cout << "COLLISION - DUMPSTER\n";
                return true;
            }
        }
        else if (obstacleWO->getLabel() == "balloons") {
            if ((grifPos[0] > obsPos[0] - 5 && grifPos[0] < obsPos[0] + 5) &&
                (grifPos[1] > obsPos[1] - 30 && grifPos[1] < obsPos[1] + 30) &&
                (grifPos[2] > obsPos[2] + 2 && grifPos[2] < obsPos[2] + 5))
            {
                if (gameIsRunning) std::cout << "COLLISION - BALOONS\n";
                return true;
            }
        }
    }
    
    return false;
}

int GLViewFinalProject::getCurrentPlane() {

    int grifXPos = griffWO->getPosition().at(0);
    for (int i = 0; i < terrainPlanes.size(); i++) {
        float terrainXPos = worldLst->getWOByID(terrainPlanes[i])->getPosition().at(0);
        if (grifXPos > terrainXPos - 200 && grifXPos < terrainXPos + 200) return terrainPlanes[i];
    }
    
    return -1;
}
