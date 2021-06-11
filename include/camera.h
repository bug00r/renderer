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
	vec3_t start;
	vec3_t end;
	vec3_t up_start;
	vec3_t up_end;
} clip_t;

typedef struct {
	mat4_t view;
	mat4_t projection;
	mat4_t transformation;
	vec3_t forward;
	vec3_t left;
	vec3_t up;
	clip_t clip_line;
	float l,r,t,b,n,f;
} camera_t;

void setviewport(camera_t *  camera, const float l,const float r,const float t,const float b,const float near,const float far);

void config_camera(camera_t *  newcamera, const vec3_t *  from, const vec3_t *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);
void config_camera_perspective(camera_t *  newcamera, const vec3_t *  from, const vec3_t *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);

void config_camera_opengl(camera_t *  newcamera, const vec3_t *  from, const vec3_t *  to, 
				   const float l, const float r, const float t, const float b, const float near, const float far);

void camera_lookAt_opengl(camera_t *  camera, const vec3_t *  from, const vec3_t *  to);
void camera_lookAt_ortho(camera_t *  camera, const vec3_t *  from, const vec3_t *  to);
void camera_lookAt_perspective(camera_t *  camera, const vec3_t *  from, const vec3_t *  to);

void createProjectionPerspectiveOpenGl(camera_t *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void createProjectionOrtho2(camera_t *  camera, const float r, const float t, const float near, const float far);

void createProjectionOrtho(camera_t *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void createProjectionPerspective(camera_t *  camera, const float l, const float r, const float t, const float b, const float near, const float far);

void print_camera(const camera_t *  camera);

#endif
