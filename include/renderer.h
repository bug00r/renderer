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
#include "utilsmath.h" 
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
} Projection;

typedef void (*RENDERER_RENDER_FUNC)(void *, const Shape*);

typedef struct _renderer {
	int imgWidth;
	int imgHeight;
	float imgWidth_half;
	float imgHeight_half;
	int bufWidth;
	int bufHeight;
	Camera camera;
	unsigned int samplestep;
	unsigned int used_samples;
	//unsigned int *wh_index;
	Vec2 * samples;
	float sample_factor;
	ColorRGB * frameBuffer;
	float * zBuffer;
	//Texture *texture; //TODO here we need a list of textures...currently we use one as test
	TextureCache *texture_cache;
	ColorRGB bgcolor;
	Projection projection;
	float min_z;
	float max_z;
	RENDERER_RENDER_FUNC POINT_RENDER_FUNC;
	RENDERER_RENDER_FUNC LINE_RENDER_FUNC;
	RENDERER_RENDER_FUNC TRIANGLE_RENDER_FUNC;
} Renderer;



#if 0
	/**
		-render function 
			- worldToCam projection
			- world => NDC conversion
			- NDC => raster conversion
	 
		should add missing vertice or object or something same to parameterlist. Not know yet. prototype data will be set inside.
	 */
#endif
void render_scene(Renderer * renderer, const Scene * scene);
void render_mesh(Renderer * renderer, const Mesh * mesh);
void render_shape(Renderer * renderer, const Shape * shape);

void renderer_clear_frame(Renderer * renderer);

/* 
	This function set solid renderer view mode. This means it renders interpolated colors by vertex or texture like expected.
*/
void renderer_set_vmode_solid(Renderer * renderer);
/* 
	This function set solid renderer point mode. This means it renders only vertex points colors.
*/
void renderer_set_vmode_point(Renderer * renderer);
/* 
	This function set solid renderer point mode. This means it renders only edge lines where it is possibl, points will be handled like in point view mode.
*/
void renderer_set_vmode_line(Renderer * renderer);

#if 0
	/**
		Create renderer
	*/
#endif
Renderer * renderer_new(int imgWidth, int imgHeight, ColorRGB * bgColor, unsigned int samplestep);

#if 0
	/**
		Deletes renderer
	*/
#endif
void renderer_free(Renderer * renderer);

#if 0
	/**
		saves renderer output
	*/
#endif
void renderer_output_ppm(Renderer * renderer, const char * filename);
#if 0
	/**
		saves z buffer output
	*/
#endif
void renderer_output_z_buffer_ppm(Renderer * renderer, const char * filename);

#endif