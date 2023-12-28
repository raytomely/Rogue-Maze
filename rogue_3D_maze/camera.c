#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "camera.h"


// FreeFly Camera :
// ================

FreeFlyCamera freeFlyCamera;
TrackBallCamera trackBallCamera;

int default_keyconf[KEYCONF_LENGTH] = {SDLK_UP, SDLK_DOWN, SDLK_RIGHT, SDLK_LEFT, SDLK_SPACE};

int default_keystates[KEYCONF_LENGTH] = {0, 0, 0, 0, 0};

int default_rotkeys[4] = { SDLK_w, SDLK_s, SDLK_a, SDLK_d };

int default_rotstates[4] = { 0, 0, 0, 0 };

void initFreeFlyCamera(FreeFlyCamera *camera)
{
    Vector3D position = {0, 0, 1.5}; //{40, 0, 30};
    camera->position = position;
    camera->phi = -0;
    camera->theta = 180; //90; //180;
    camera->rotspeed =3;
    FreeFlyCameraVectorsFromAngles(camera);

    camera->speed = 0.05; //0.01;
    camera->sensivity = 0.2;
    camera->keyconf = default_keyconf;
    camera->keystates = default_keystates;
    camera->rotkeys = default_rotkeys;
    camera->rotstates = default_rotstates;
    memset(camera->keystates, 0, sizeof(int)*KEYCONF_LENGTH);
    memset(camera->rotstates, 0, sizeof(int)*4);
}

void FreeFlyCameraOnMouseMotion(FreeFlyCamera *camera, SDL_MouseMotionEvent *event)
{
    camera->theta -= event->xrel*camera->sensivity;
    camera->phi -= event->yrel*camera->sensivity;
    FreeFlyCameraVectorsFromAngles(camera);
}

void FreeFlyCameraOnMouseButton(FreeFlyCamera *camera, SDL_MouseButtonEvent *event)
{
    if ((event->button == SDL_BUTTON_WHEELUP)&&(event->type == SDL_MOUSEBUTTONDOWN)) // up move
    {
        camera->verticalMotionActive = 1;
        camera->timeBeforeStoppingVerticalMotion = 250;
        camera->verticalMotionDirection = 1;

    }
    else if ((event->button == SDL_BUTTON_WHEELDOWN)&&(event->type == SDL_MOUSEBUTTONDOWN)) // down move
    {
        camera->verticalMotionActive = 1;
        camera->timeBeforeStoppingVerticalMotion = 250;
        camera->verticalMotionDirection = -1;
    }
}

void FreeFlyCameraOnKeyboard(FreeFlyCamera *camera, SDL_KeyboardEvent *event)
{
    int i;
    for (i = 0; i < KEYCONF_LENGTH; i++)
    {
        if (event->keysym.sym == camera->keyconf[i])
        {
            camera->keystates[i] = (event->type == SDL_KEYDOWN);
            break;
        }
    }
    FreeFlyCameraOnRotKeys(camera, event);
}

void FreeFlyCameraOnRotKeys(FreeFlyCamera *camera, SDL_KeyboardEvent *event)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        if (event->keysym.sym == camera->rotkeys[i])
        {
            camera->rotstates[i] = (event->type == SDL_KEYDOWN);
            break;
        }
    }
}

void FreeFlyCameraRotate(FreeFlyCamera *camera)
{
    if (camera->rotstates[uprotkey])
        camera->phi += camera->rotspeed;
    if (camera->rotstates[downrotkey])
        camera->phi -= camera->rotspeed;
    if (camera->rotstates[rightrotkey])
        camera->theta += camera->rotspeed;
    if (camera->rotstates[leftrotkey])
        camera->theta -= camera->rotspeed;
    FreeFlyCameraVectorsFromAngles(camera);
}

void FreeFlyCameraAnimate(FreeFlyCamera *camera, Uint32 timestep)
{
    FreeFlyCameraRotate(camera);
    double realspeed = (camera->keystates[boostKey]) ? (10*camera->speed) : camera->speed;
    if (camera->keystates[forwardKey])
        camera->position = *addVector3D(&camera->position, mulVector3D(&camera->forwardVector, (realspeed * timestep)));
    if (camera->keystates[backwardKey])
        camera->position = *subVector3D(&camera->position, mulVector3D(&camera->forwardVector, (realspeed * timestep)));
    if (camera->keystates[leftKey])
        camera->position = *addVector3D(&camera->position, mulVector3D(&camera->lateralVector, (realspeed * timestep)));
    if (camera->keystates[rightKey])
        camera->position = *subVector3D(&camera->position, mulVector3D(&camera->lateralVector, (realspeed * timestep)));
    if (camera->verticalMotionActive)
    {
        if (timestep > camera->timeBeforeStoppingVerticalMotion)
            camera->verticalMotionActive = 0;
        else
           camera->timeBeforeStoppingVerticalMotion -= timestep;
        camera->position.y += camera->verticalMotionDirection*realspeed*timestep;
        //camera->position.z += camera->verticalMotionDirection*realspeed*timestep;
    }
    camera->target = *addVector3D(&camera->position, &camera->forwardVector);
}


void FreeFlyCameraSetPosition(FreeFlyCamera *camera, Vector3D *position)
{
    camera->position = *position;
    camera->target = *addVector3D(&camera->position, &camera->forwardVector);
}


void FreeFlyCameraVectorsFromAngles(FreeFlyCamera *camera)
{
    static Vector3D up =  {0,1,0};
    //static Vector3D up =  {0,0,1};
    if (camera->phi > 89)
        camera->phi = 89;
    else if (camera->phi < -89)
        camera->phi = -89;
    double r_temp = cos(camera->phi*M_PI/180);
    camera->forwardVector.y = sin(camera->phi*M_PI/180);
    camera->forwardVector.z = r_temp*cos(camera->theta*M_PI/180);
    camera->forwardVector.x = r_temp*sin(camera->theta*M_PI/180);
    /*camera->forwardVector.z = sin(camera->phi*M_PI/180);
    camera->forwardVector.x = r_temp*cos(camera->theta*M_PI/180);
    camera->forwardVector.y = r_temp*sin(camera->theta*M_PI/180);*/


    crossProduct(&up, &camera->forwardVector, &camera->lateralVector);

    normalizeVector3D(&camera->lateralVector);

    camera->target = *addVector3D(&camera->position, &camera->forwardVector);
}

void FreeFlyCameraLookAt(FreeFlyCamera *camera)
{
    gluLookAt(camera->position.x, camera->position.y, camera->position.z,
              camera->target.x, camera->target.y, camera->target.z,
              0,1,0); //0,1,0);
    TrackBallCameraLookAt(&trackBallCamera);
}


// TrackBall Camera :
// ================

void initTrackballCamera(TrackBallCamera *camera)
{
    camera->hold = 0; // Are we currently holding down the left mouse button?
    camera->angleY = 0; // vertical rotation angle of the scene;
    camera->angleZ = 0; // horizontal rotation angle of the scene (therefore around the vertical).
    camera->distance = 2; //   distance between the camera and the center of the scene;
    camera->motionSensivity = 0.3; // used to store camera sensitivity to mouse movements;
    camera->scrollSensivity = 1; // sensitivity of the camera to mouse scrolling (“step” of a movement);
}

void TrackBallCameraOnMouseMotion(TrackBallCamera *camera, SDL_MouseMotionEvent *event)
{
    if (camera->hold)
    {
        camera->angleZ += event->xrel*camera->motionSensivity;
        camera->angleY += event->yrel*camera->motionSensivity;
        if (camera->angleY > 90)
            camera->angleY = 90;
        else if (camera->angleY < -90)
            camera->angleY = -90;
    }
}

void TrackBallCameraOnMouseButton(TrackBallCamera *camera, SDL_MouseButtonEvent *event)
{
    if (event->button == SDL_BUTTON_LEFT)
    {
        if ((camera->hold)&&(event->type == SDL_MOUSEBUTTONUP))
        {
            camera->hold = 0;
        }
        else if ((!camera->hold)&&(event->type == SDL_MOUSEBUTTONDOWN))
        {
            camera->hold = 1;
        }
    }
    /*else if ((event->button == SDL_BUTTON_WHEELUP)&&(event->type == SDL_MOUSEBUTTONDOWN))
    {
        camera->distance -= camera->scrollSensivity;
        if (camera->distance < 0.1)
            camera->distance = 0.1;
    }
    else if ((event->button == SDL_BUTTON_WHEELDOWN)&&(event->type == SDL_MOUSEBUTTONDOWN))
    {
            camera->distance += camera->scrollSensivity;
    }*/
}

void TrackBallCameraOnKeyboard(TrackBallCamera *camera, SDL_KeyboardEvent *event)
{
    if ((event->type == SDL_KEYDOWN)&&(event->keysym.sym == SDLK_HOME))
    {
        camera->angleY = 0;
        camera->angleZ = 0;
    }
}

void TrackBallCameraLookAt(TrackBallCamera *camera)
{
    /*gluLookAt(camera->distance,0,0,
              0,0,0,
              0,0,1);*/

    //GLdouble tile_size = (GLdouble)1/640*32;
    //GLdouble width = 80*tile_size, height = 24*tile_size;

    //glTranslated(-width,-height,0);
    //glTranslated(-width,0,0);

    /*glRotated(camera->angleX,1,0,0);
    glRotated(camera->angleY,0,1,0);*/
    glRotated(camera->angleY,0,1,0);
    glRotated(camera->angleZ,0,0,1);


    //glTranslated(width,0,0);
    //glTranslated(width,height,0);
}








