#include <stdlib.h>
#include <stdio.h>
#include "renderer.h"

/**
 *
 *  This function filters or clips line based on camera position. If both line vector lay
 *  behind the cam there is no need to draw. If both line points in front of the cam, they
 *  will leaved unhandled. If there is one point in front of the cam and one behind, there
 *  will be computed the intersection point and the given line will be clipped and the result
 *  stored in destination vectors. 
 * 
 * 	If return value is true, the destination vectors should be handled, otherwise not.
 */
static bool __line_filter_or_clip(renderer_t* _renderer, vec3_t* _dest_start, vec3_t* _dest_end,
								  const vec3_t* _line_start, const vec3_t* _line_end ) {
	
	renderer_t* renderer = _renderer;
	plane_t *near = &renderer->camera.frustum.near;
	vec3_t *normal = &near->normal;

	vec3_t	*line_start = (vec3_t*)_line_start;
	vec3_t	*line_end = (vec3_t*)_line_end;

	bool filtered = false;
	float dist_start = mu_point_plane_distance_normal(line_start, &near->lb, normal); 
	float dist_end = mu_point_plane_distance_normal(line_end, &near->lb, normal);
	bool start_out = (dist_start < 0);
	bool end_out = (dist_end < 0);

	filtered = (start_out && end_out);

	//printf("---------\nstart out: %i(d:%f)  end out: %i(d:%f)\n", start_out, dist_start, end_out, dist_end);
	//printf("line: ");
	//vec3_print(line_start);
	//vec3_print(line_end);
	//print_camera(&renderer->camera);

	vec3_copy_dest(_dest_start, line_start);
	vec3_copy_dest(_dest_end, line_end);

	if (filtered)  {	
		return false;
	} else if ( !start_out && !end_out ) {
		return true;
	}

	vec3_t intersect;
	if (mu_line_plane_intersection_normal(&intersect, line_start, line_end, &near->lb, normal)) {
		vec3_t *to_clip = ( start_out ? _dest_start : _dest_end);
		vec3_copy_dest(to_clip, &intersect);
		//printf("line clip: ");
		//vec3_print(to_clip);
	}

	return true;
}

static void __copy_vertex_to_rasterObj(const vertex_t *_curVertex, unsigned int _vertexIdx, raster_obj_t *_rasterObj)
{
	const vertex_t *curVertex = _curVertex;
	raster_obj_t *rasterObj = _rasterObj;
	unsigned int vertexIndx = _vertexIdx;

	vec3_copy_dest(&rasterObj->vec[vertexIndx], &curVertex->vec);
	crgb_crgb_copy(&rasterObj->color[vertexIndx], &curVertex->color);
	vec2_copy_dest(&rasterObj->texCoord[vertexIndx], &curVertex->texCoord);
	
}

static void __shape_to_rasterObj(renderer_t *_renderer, const shape_t * _shape, raster_obj_t *_rasterObj)
{
	renderer_t *renderer = _renderer;
	const shape_t *shape = _shape;
	raster_obj_t *rasterObj = _rasterObj;

	//Setting Raster texture
	texture_cache_t * cache = renderer->texture_cache;
	texture_t *texture = texture_cache_get(cache, (unsigned int)shape->texId); 
	rasterObj->texture = texture;

	//Setting Shape Parameter to Raster Obj
	rasterObj->cntVertex = shape->cntVertex;
	const vertex_t **vertices = (const vertex_t **)shape->vertices;
	switch(rasterObj->cntVertex)
	{
		case 3: __copy_vertex_to_rasterObj(vertices[2], 2, rasterObj);
		case 2: __copy_vertex_to_rasterObj(vertices[1], 1, rasterObj);
		case 1: __copy_vertex_to_rasterObj(vertices[0], 0, rasterObj);
		default: break;
	}



}

void render_shape(renderer_t *renderer, const shape_t *shape){

	const shape_t *  curshape = shape;
	renderer_t * curRenderer = renderer;
	raster_ctx_t *rasterCtx = &curRenderer->rasterCtx;

	rasterCtx->ct = &curRenderer->camera.transformation;

	raster_obj_t rasterObj;
	raster_state_t rasterState;

	__shape_to_rasterObj(renderer, curshape, &rasterObj);

	if ( rasterObj.cntVertex == 2 )
	{
		vec3_t _v1v, _v2v;
		const vec3_t * v1v = &_v1v, * v2v = &_v2v;
		if ( !__line_filter_or_clip(curRenderer, &_v1v, &_v2v, &rasterObj.vec[0], &rasterObj.vec[1]) ) {
			return;
		}
		vec3_copy_dest(&rasterObj.vec[0], v1v);
		vec3_copy_dest(&rasterObj.vec[1], v2v);
	}

	raster(rasterCtx, &rasterObj, &rasterState);

}

static void __r_update_pVertex(vec3_t* _pVertex, vec3_t *_normal, bbox_t *_bbox)
{
	vec3_t* pVertex = _pVertex;
	vec3_t *normal = _normal;
	bbox_t *bbox = _bbox;

	pVertex->x = ( normal->x >= 0.f ? bbox->max.x : bbox->min.x );
	pVertex->x = ( normal->y >= 0.f ? bbox->max.y : bbox->min.y );
	pVertex->x = ( normal->z >= 0.f ? bbox->max.z : bbox->min.z );
}

static void __r_update_nVertex(vec3_t* _nVertex, vec3_t *_normal, bbox_t *_bbox)
{
	vec3_t* nVertex = _nVertex;
	vec3_t *normal = _normal;
	bbox_t *bbox = _bbox;

	nVertex->x = ( normal->x >= 0.f ? bbox->min.x : bbox->max.x );
	nVertex->x = ( normal->y >= 0.f ? bbox->min.y : bbox->max.y );
	nVertex->x = ( normal->z >= 0.f ? bbox->min.z : bbox->max.z );
}

static int __r_check_plate_frustum(plane_t* _plane, bbox_t *_bbox)
{
	bbox_t *bbox = _bbox;
	plane_t* plane = _plane;
	
	uint32_t cntOutside = 0;
	
	vec3_t pVertex, nVertex;
	__r_update_nVertex(&nVertex, &plane->normal, bbox);
	__r_update_pVertex(&pVertex, &plane->normal, bbox);
	
	cntOutside += (mu_point_plane_distance_normal(&nVertex, &plane->lb, &plane->normal) < 0.f ? 1 : 0);
	cntOutside += (mu_point_plane_distance_normal(&pVertex, &plane->lb, &plane->normal) < 0.f ? 1 : 0);
	
	return cntOutside;
}

static void __r_frustum_as_vec3_array(vec3_t **_array, plane_t* _nearPlane, plane_t* _farPlane)
{
	vec3_t **array = _array;
	plane_t* nearPlane = _nearPlane;
	plane_t* farPlane = _farPlane;

	array[0] = &nearPlane->lb;
	array[1] = &nearPlane->rb;
	array[2] = &nearPlane->lt;
	array[3] = &nearPlane->rt;
	array[4] = &farPlane->lb;
	array[5] = &farPlane->rb;
	array[6] = &farPlane->lt;
	array[7] = &farPlane->rt;
}

static bool __r_bbox_in_frustum(frustum_t *_frustum, bbox_t *_bbox)
{
	frustum_t *frustum = _frustum;
	bbox_t *bbox = _bbox;

	uint32_t out = 0;

	/**
	 *  ref: https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
	 * 
	 * 	Topic: Geometric Approach - Testing Boxes II 
	 * 
	 */ 
	out += __r_check_plate_frustum(&frustum->bottom, bbox);
	out += __r_check_plate_frustum(&frustum->top, bbox);
	out += __r_check_plate_frustum(&frustum->far, bbox);
	out += __r_check_plate_frustum(&frustum->near, bbox);
	out += __r_check_plate_frustum(&frustum->left, bbox);
	out += __r_check_plate_frustum(&frustum->right, bbox);

	if (out == 12) return false;

	/**
	 * 	ref: https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	 * 
	 * */
	vec3_t*vecs[8];
	__r_frustum_as_vec3_array(&vecs[0], &frustum->near, &frustum->far);

	out = 0;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->x > bbox->max.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->x < bbox->min.x)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->y > bbox->max.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->y < bbox->min.y)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->z > bbox->max.z)?1:0); if( out==8 ) return false;
    out=0; for( int i=0; i<8; i++ ) out += ((vecs[i]->z < bbox->min.z)?1:0); if( out==8 ) return false;	

	return true;
}

void render_mesh(renderer_t * renderer, const mesh_t *  mesh){
	bbox_t * bbox = (bbox_t*)&mesh->bbox;
	if ( bbox->created  )
	{
		if ( !__r_bbox_in_frustum(&renderer->camera.frustum, bbox) ) return;
	}
	shape_t ** shapes = mesh->shapes;
	for(unsigned int cntShape = mesh->cntShapes; cntShape-- ;) {		
		render_shape(renderer, *shapes);
		++shapes;
	} 
}

void 
render_scene(renderer_t *  renderer, const scene_t *  scene){	
	mesh_t ** meshes = scene->meshes;
	for(int cntMesh = scene->cntMesh; cntMesh--; ) {
		render_mesh(renderer, *meshes);
		++meshes;
	}
}

void renderer_clear_frame(renderer_t *renderer){
	raster_ctx_t *rasterCtx = &renderer->rasterCtx;
	const int buffersize = rasterCtx->imgWidth * 
						   rasterCtx->imgHeight * 
						   rasterCtx->samplestep * rasterCtx->samplestep;
	memset(renderer->zBuffer, 0.f, buffersize * sizeof(float));
	memset(renderer->frameBuffer, 0, buffersize * sizeof(cRGB_t));
	rasterCtx->max_z = RENDER_FLT_MAX;
	rasterCtx->min_z = 0.f;
}

// EXOPORTED TO RASTERIZER
void renderer_set_vmode_solid(renderer_t * renderer) {
	raster_set_vmode_solid(&renderer->rasterCtx);
}

void renderer_set_vmode_point(renderer_t * renderer) {
	raster_set_vmode_point(&renderer->rasterCtx);
}

void renderer_set_vmode_line(renderer_t * renderer) {
	raster_set_vmode_line(&renderer->rasterCtx);
}
// EOF EXPORTED TO RASTERIZER

renderer_t * 
renderer_new(int imgWidth, int imgHeight, cRGB_t * bgColor, unsigned int samplestep){
	
	renderer_t * newrenderer = malloc(sizeof(renderer_t));
	newrenderer->projection = RP_ORTHOGRAPHIC;
	crgb_crgb_copy(&newrenderer->bgcolor, bgColor);
	renderer_set_vmode_solid(newrenderer);
	newrenderer->texture_cache = texture_cache_new();
	renderer_clear_frame(newrenderer);

	// INIT RASTER CTX
	raster_ctx_t *rasterCtx = &newrenderer->rasterCtx;
	rasterCtx->imgWidth = imgWidth;
	rasterCtx->imgHeight = imgHeight;
	rasterCtx->imgWidth_half = 0.5f * imgWidth;
	rasterCtx->imgHeight_half = 0.5f * imgHeight;
	unsigned int us = samplestep*samplestep;
	unsigned int bw = imgWidth * us;
	rasterCtx->bufWidth = bw;
	rasterCtx->bufHeight = imgHeight;
	rasterCtx->samplestep = samplestep;
	
	unsigned int buffersize = imgWidth * imgHeight * us;
	newrenderer->frameBuffer = malloc(buffersize * sizeof(cRGB_t));
	newrenderer->zBuffer = malloc(buffersize * sizeof(float));
	
	rasterCtx->min_z = RENDER_FLT_MAX;
	rasterCtx->max_z = 0.f;
	rasterCtx->frameBuffer = newrenderer->frameBuffer;
	rasterCtx->zBuffer = newrenderer->zBuffer;

	//Transformation Matrix will be set during rendering
	rasterCtx->ct = NULL;
	
	#if 0
		//add samples logic
	#endif
	rasterCtx->used_samples = us;
	rasterCtx->sample_factor = 1.f/rasterCtx->used_samples;
	float stepstart = 0.5f / (float)samplestep; //for st = 2 step is .25  for st = 4 0.125
	float step = 2.f*stepstart; //distance between 

	rasterCtx->samples = malloc(rasterCtx->used_samples * sizeof(vec2_t));

	for( unsigned int sy = 0; sy < samplestep ;++sy ){
		for( unsigned int sx = 0; sx < samplestep ;++sx ){
			vec2_t  *cursample = &rasterCtx->samples[sy * samplestep + sx];
			cursample->x = stepstart + (sx * step);
			cursample->y = stepstart + (sy * step);
		}
	}
	// EOF INIT RASTER CTX
	
	return newrenderer;
}

void 
renderer_free(renderer_t * renderer){
	free(renderer->frameBuffer);
	free(renderer->zBuffer);
	raster_ctx_t *rasterCtx = &renderer->rasterCtx;
	free(rasterCtx->samples);
	texture_cache_free(&renderer->texture_cache);
	free(renderer);
}


void 
renderer_output_ppm(renderer_t * renderer, const char * filename){
	unsigned int colcnt=0, bi=0, samplestart;
	raster_ctx_t *rasterCtx = &renderer->rasterCtx;
	int i, j, imgW = rasterCtx->imgWidth, imgH = rasterCtx->imgHeight;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", imgW, imgH);
	cRGB_t fc;
	unsigned char color[3*imgW*imgH];
    for (j = 0; j < imgH; ++j){
	  bi = j * rasterCtx->bufWidth;
	  for (i = 0; i < imgW; ++i){
		fc.r = 0.f, fc.g = 0.f, fc.b = 0.f;
		samplestart = bi + (i*rasterCtx->used_samples);
		for (unsigned int sample = rasterCtx->used_samples; sample--;){
			cRGB_t * c = &rasterCtx->frameBuffer[samplestart + sample];
			//crgb_crgb_add(&fc, c);
			fc.r += c->r;
			fc.g += c->g;
			fc.b += c->b;
		}
	 
		color[colcnt++] = (unsigned char)(fc.r * 255.f);
		color[colcnt++] = (unsigned char)(fc.g * 255.f);
		color[colcnt++] = (unsigned char)(fc.b * 255.f);
	  }
    }
	(void) fwrite(color, 1, 3*imgW*imgH, fp);
    (void) fclose(fp);
}

void 
renderer_output_z_buffer_ppm(renderer_t *renderer, const char * filename){
	unsigned int colcnt=0,bi=0,samplestart;
	raster_ctx_t *rasterCtx = &renderer->rasterCtx;
	int i, j, imgW = rasterCtx->imgWidth, imgH = rasterCtx->imgHeight;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", imgW, imgH);
	float _color, samplefactor = rasterCtx->sample_factor;
	unsigned char color[3*imgW*imgH];
	
    for (j = 0; j < imgH; ++j){
	  bi = j * rasterCtx->bufWidth;
	  for (i = 0; i < imgW; ++i){
		_color = 0.f;
		samplestart = bi + (i*rasterCtx->used_samples);
		for (unsigned int sample = rasterCtx->used_samples; sample--;){
			_color += rasterCtx->zBuffer[samplestart + sample];
		}
		
		_color *= samplefactor ;
		
		if ( _color != RENDER_FLT_MAX ){
			_color = (unsigned char)interpolate_lin(_color, rasterCtx->max_z, 0.f, rasterCtx->min_z, 255.f);
		} else {
			_color = 0.f;
		}
				
		color[colcnt++] = _color;
		color[colcnt++] = _color;
		color[colcnt++] = _color;
	  }
    }
	(void) fwrite(color, 1, 3*imgW*imgH, fp);
    (void) fclose(fp);
}
