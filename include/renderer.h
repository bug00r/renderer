#if 0
/**
	This is a simple rendereing engine. 
*/
#endif

#ifndef RENDERER_H
#define RENDERER_H

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "vec.h"
#include "mat.h"
#include "utils_math.h"
#include "geometry.h"
#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "texture.h"
#include "texture_cache.h"

#define RENDER_FLT_MAX 339615136492207130000000000000000000000.000000

typedef enum {
	RP_PERSPECTIVE, 
	RP_ORTHOGRAPHIC
} projection_t;

typedef void (*RENDERER_RENDER_FUNC)(void *, const shape_t*);

typedef struct _renderer {
	int imgWidth;
	int imgHeight;
	float imgWidth_half;
	float imgHeight_half;
	int bufWidth;
	int bufHeight;
	camera_t camera;
	unsigned int samplestep;
	unsigned int used_samples;
	//unsigned int *wh_index;
	vec2_t * samples;
	float sample_factor;
	cRGB_t * frameBuffer;
	float * zBuffer;
	//texture_t *texture; //TODO here we need a list of textures...currently we use one as test
	texture_cache_t *texture_cache;
	cRGB_t bgcolor;
	projection_t projection;
	float min_z;
	float max_z;
	RENDERER_RENDER_FUNC POINT_RENDER_FUNC;
	RENDERER_RENDER_FUNC LINE_RENDER_FUNC;
	RENDERER_RENDER_FUNC TRIANGLE_RENDER_FUNC;
} renderer_t;



#if 0
	/**
		-render function 
			- worldToCam projection
			- world => NDC conversion
			- NDC => raster conversion
	 
		should add missing vertice or object or something same to parameterlist. Not know yet. prototype data will be set inside.
	 */
#endif
void render_scene(renderer_t * renderer, const scene_t * scene);
void render_mesh(renderer_t * renderer, const mesh_t * mesh);
void render_shape(renderer_t * renderer, const shape_t * shape);

void renderer_clear_frame(renderer_t * renderer);

/* 
	This function set solid renderer view mode. This means it renders interpolated colors by vertex or texture like expected.
*/
void renderer_set_vmode_solid(renderer_t * renderer);
/* 
	This function set solid renderer point mode. This means it renders only vertex points colors.
*/
void renderer_set_vmode_point(renderer_t * renderer);
/* 
	This function set solid renderer point mode. This means it renders only edge lines where it is possibl, points will be handled like in point view mode.
*/
void renderer_set_vmode_line(renderer_t * renderer);

#if 0
	/**
		Create renderer
	*/
#endif
renderer_t * renderer_new(int imgWidth, int imgHeight, cRGB_t * bgColor, unsigned int samplestep);

#if 0
	/**
		Deletes renderer
	*/
#endif
void renderer_free(renderer_t * renderer);

#if 0
	/**
		saves renderer output
	*/
#endif
void renderer_output_ppm(renderer_t * renderer, const char * filename);
#if 0
	/**
		saves z buffer output
	*/
#endif
void renderer_output_z_buffer_ppm(renderer_t * renderer, const char * filename);

#endif