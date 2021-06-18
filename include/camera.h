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
	vec3_t lb;
	vec3_t rb;
	vec3_t lt;
	vec3_t rt;
	vec3_t normal;
} plane_t;

typedef struct {
	plane_t left;
	plane_t right;
	plane_t top;
	plane_t bottom;
	plane_t near;
	plane_t far;
} frustum_t;

typedef struct {
	mat4_t view;
	mat4_t projection;
	mat4_t transformation;
	vec3_t forward;
	vec3_t left;
	vec3_t up;
	vec3_t from;
	vec3_t to;
	frustum_t frustum;
	float l,r,t,b,n,f;
} camera_t;

void setviewport(camera_t *  camera, const float l,const float r,const float t,const float b,const float near,const float far);

void config_camera(camera_t *  newcamera, const vec3_t *  from, const vec3_t *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);
void config_camera_perspective(camera_t *  newcamera, const vec3_t *  from, const vec3_t *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);

void camera_lookAt(camera_t *  camera, const vec3_t *  from, const vec3_t *  to);

void createProjectionOrtho(camera_t *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void createProjectionPerspective(camera_t *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void print_camera(const camera_t *  camera);

#endif
