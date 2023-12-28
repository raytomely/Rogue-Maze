#ifndef VECTOR3D_H
#define VECTOR3D_H

typedef struct
{
    double x;
    double y;
    double z;
}Vector3D;

Vector3D temp_v;

void setVector3D(Vector3D *v, double x,double y,double z);

void copyVector3D(Vector3D *v_dest, Vector3D *v_source);

Vector3D* addVector3D(Vector3D *v1, Vector3D *v2);

Vector3D* subVector3D(Vector3D *v1, Vector3D *v2);

Vector3D* mulVector3D(Vector3D *v, double a);

Vector3D* divVector3D(Vector3D *v, double a);

void crossProduct(Vector3D *v1, Vector3D *v2, Vector3D *v_result);

double lengthVector3D(Vector3D *v);

void normalizeVector3D(Vector3D *v);

#endif //VECTOR3D_H
