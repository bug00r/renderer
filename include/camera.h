#if 0
/**
	This is a simple color class. 
*/
#endif

#ifndef CAMERA_H
#define CAMERA_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "vec.h"
#include "mat.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327
#endif

typedef struct {
	Vec3 lb;
	Vec3 rb;
	Vec3 lt;
	Vec3 rt;
	Vec3 normal;
} Plane;

typedef struct {
	Plane left;
	Plane right;
	Plane top;
	Plane bottom;
	Plane near;
	Plane far;
} Frustum;

typedef struct {
	Mat4 view;
	Mat4 projection;
	Mat4 transformation;
	Vec3 forward;
	Vec3 left;
	Vec3 up;
	Vec3 from;
	Vec3 to;
	Frustum frustum;
	float l,r,t,b,n,f;
} Camera;

void setviewport(Camera *  camera, const float l,const float r,const float t,const float b,const float near,const float far);

void config_camera(Camera *  newcamera, const Vec3 *  from, const Vec3 *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);
void config_camera_perspective(Camera *  newcamera, const Vec3 *  from, const Vec3 *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);

void camera_lookAt(Camera *  camera, const Vec3 *  from, const Vec3 *  to);

void createProjectionOrtho(Camera *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void createProjectionPerspective(Camera *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void print_camera(const Camera *  camera);

#endif
