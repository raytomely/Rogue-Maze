#include "vector3d.h"
#include <SDL/SDL.h>


// FreeFly Camera :
// ================

#define KEYCONF_LENGTH 5

//Enum for buttons
enum { forwardKey, backwardKey, rightKey, leftKey, boostKey };

// rotation keys
enum { uprotkey, downrotkey, rightrotkey, leftrotkey };

// controls keys
typedef struct
{
    int forwardKey, backwardKey, rightKey, leftKey, boostKey;
}KeyConf;

typedef struct
{
    int forwardKey, backwardKey, rightKey, leftKey, boostKey;
}KeyStates;

typedef struct
{
    double speed;
    double sensivity;
    double rotspeed;

    //vertical motion stuffs
    Uint32 timeBeforeStoppingVerticalMotion;
    int verticalMotionActive;
    int verticalMotionDirection;

    int *keystates;
    int *keyconf;
    int *rotkeys;
    int *rotstates;

    Vector3D position;  // position de la caméra dans l'absolu;
    Vector3D target;  // point viewed by the camera in absolute terms;
    Vector3D forwardVector;  // vector giving the direction of gaze (and therefore forward movement);
    Vector3D lateralVector;  // vector perpendicular to the gaze for lateral movement;
    double theta;  // camera horizontal rotation angle (around vertical);
    double phi;  // angle de rotation verticale de la caméra.
}FreeFlyCamera;

void initFreeFlyCamera(FreeFlyCamera *camera);
void FreeFlyCameraOnMouseMotion(FreeFlyCamera *camera, SDL_MouseMotionEvent *event);
void FreeFlyCameraOnMouseButton(FreeFlyCamera *camera, SDL_MouseButtonEvent *event);
void FreeFlyCameraOnKeyboard(FreeFlyCamera *camera, SDL_KeyboardEvent *event);
void FreeFlyCameraOnRotKeys(FreeFlyCamera *camera, SDL_KeyboardEvent *event);
void FreeFlyCameraRotate(FreeFlyCamera *camera);
void FreeFlyCameraAnimate(FreeFlyCamera *camera, Uint32 timestep);
void FreeFlyCameraSetPosition(FreeFlyCamera *camera, Vector3D *position);
void FreeFlyCameraVectorsFromAngles(FreeFlyCamera *camera);
void FreeFlyCameraLookAt(FreeFlyCamera *camera);


// Trackball Camera :
// ================

typedef struct
{
    double motionSensivity; // used to store camera sensitivity to mouse movements;
	double scrollSensivity; // sensitivity of the camera to mouse scrolling (“step” of a movement);
    int hold; // Are we currently holding down the left mouse button?
    double distance; // distance between the camera and the center of the scene;
    double angleY; // vertical rotation angle of the scene;
    double angleZ; // horizontal rotation angle of the scene (therefore around the vertical).
}TrackBallCamera;

void initTrackballCamera(TrackBallCamera *camera);
void TrackBallCameraOnMouseMotion(TrackBallCamera *camera, SDL_MouseMotionEvent *event);
void TrackBallCameraOnMouseButton(TrackBallCamera *camera, SDL_MouseButtonEvent *event);
void TrackBallCameraOnKeyboard(TrackBallCamera *camera, SDL_KeyboardEvent *event);
void TrackBallCameraLookAt(TrackBallCamera *camera);





