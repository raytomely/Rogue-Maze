#include "vector3d.h"
#include <math.h>

void setVector3D(Vector3D *v, double x,double y,double z)
{
    v->x = x;
    v->y = y;
    v->z = z;
}

void copyVector3D(Vector3D *v_dest, Vector3D *v_source)
{
    v_dest->x = v_source->x;
    v_dest->y = v_source->y;
    v_dest->z = v_source->z;
}

Vector3D* addVector3D(Vector3D *v1, Vector3D *v2)
{
    temp_v.x = v1->x + v2->x;
    temp_v.y = v1->y + v2->y;
    temp_v.z = v1->z + v2->z;
    return &temp_v;
}

Vector3D* subVector3D(Vector3D *v1, Vector3D *v2)
{
    temp_v.x = v1->x - v2->x;
    temp_v.y = v1->y - v2->y;
    temp_v.z = v1->z - v2->z;
    return &temp_v;
}

Vector3D* mulVector3D(Vector3D *v, double a)
{
    temp_v.x = v->x * a;
    temp_v.y = v->y * a;
    temp_v.z = v->z * a;
    return &temp_v;
}

Vector3D* divVector3D(Vector3D *v, double a)
{
    temp_v.x = v->x / a;
    temp_v.y = v->y / a;
    temp_v.z = v->z / a;
    return &temp_v;
}

void crossProduct(Vector3D *v1, Vector3D *v2, Vector3D *v_result)
{
    v_result->x = v1->y*v2->z - v1->z*v2->y;
    v_result->y = v1->z*v2->x - v1->x*v2->z;
    v_result->z = v1->x*v2->y - v1->y*v2->x;
}

double lengthVector3D(Vector3D *v)
{
    return sqrt( v->x*v->x + v->y*v->y + v->z*v->z );
}

void normalizeVector3D(Vector3D *v)
{
    double length = lengthVector3D(v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}


