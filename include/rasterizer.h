#ifndef RASTERIZER_H
#define RASTERIZER_H
/**

    The Rasterizer offers Interfaces for drawing Objects to an Buffer in dependency
    from some raster Options like Anti Aliasing, Wireframe or Point Frame output.

    Furthermore it offers a raster state Object which includes calculations of
    Natural Device, Raster Coordinates and a lots of other calculations. There
    are interfaces to handle precalculation for Objects. This could be interesting
    for multithreaded or KI based rasterization. Internal Flags will be handling 
    pre calculations. This could be also useful if you want to draw 2D Raster based
    Informations like Text Labels or 2D Bounding Rectangles. 

*/

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "vec.h"
#include "mat.h"
#include "utilsmath.h" 
#include "geometry.h"
#include "texture.h"

/**
    Raster State of Render Object.
*/
typedef struct {
    //ndc coords of object
    //raster coords of objects
    //(maybe)z-values of object coordinates
    vec3_t  ndc[3];
    vec3_t  raster[3];
    float   rZ[3];
    float   weight[3];       //weight of barycentric
} raster_state_t;

typedef struct {
    unsigned int    cntVertex;       //cnt of Object Vertex values: 1-3
    vec3_t          vec[3];          //world coordinates of each vertex          
	cRGB_t          color[3];        //color value of each vertex
	vec2_t          texCoord[3];     //texture coordinate of each vertex  
    texture_t       *texture;        //pointer to used texture
} raster_obj_t;

/**

    Summary of Options which will be take effect on the rasterization Process.

*/
typedef struct {
    //Coloring: Vertex Color or Texture Colors or maybe a user Function?
    //Antialiasing: 1, 2, 4 depends on Buffer Size
    //Render Type: solid, wireframe, point
    int dummy;
} raster_opt_t;

typedef void (*RASTER_FUNC)(void *, const raster_obj_t * obj, raster_state_t *state);

/**
    A Raster Context contains all needed Data for rastering Objects.
    Objects could be a point, a line or a triangle
    


*/
typedef struct {
    int             imgWidth;
	int             imgHeight;
	float           imgWidth_half;
	float           imgHeight_half;
	
    int             bufWidth;
	int             bufHeight;

	unsigned int    samplestep;
	unsigned int    used_samples;
	vec2_t          *samples;
	float           sample_factor;
	
    cRGB_t          *frameBuffer;
	
    float           *zBuffer;
	float           min_z;
	float           max_z;

    const mat4_t    *ct;                    //transformation matrix
    raster_opt_t    opts;
    raster_state_t  state;

    RASTER_FUNC     POINT_RASTER_FUNC;
	RASTER_FUNC     LINE_RASTER_FUNC;
	RASTER_FUNC     TRIANGLE_RASTER_FUNC;
} raster_ctx_t;

void raster(raster_ctx_t *ctx, const raster_obj_t * obj, raster_state_t *state);
void raster_precalc(raster_ctx_t *ctx, const raster_obj_t * obj, raster_state_t *state);

/* 
	This function set solid renderer view mode. This means it renders interpolated colors by vertex or texture like expected.
*/
void raster_set_vmode_solid(raster_ctx_t * ctx);
/* 
	This function set solid renderer point mode. This means it renders only vertex points colors.
*/
void raster_set_vmode_point(raster_ctx_t * ctx);
/* 
	This function set solid renderer point mode. This means it renders only edge lines where it is possibl, points will be handled like in point view mode.
*/
void raster_set_vmode_line(raster_ctx_t * ctx);

#endif