#include <stdlib.h>
#include <stdio.h>
#include "renderer.h"


static void _set_color_to_fb_(cRGB_t * frameBuffer,
							  const unsigned int * bi , 
							  const float * sample_factor,
							  const cRGB_t * new_color) {
	cRGB_t * fbc = frameBuffer + *bi;
	const float sf = *sample_factor;
	fbc->r = new_color->r * sf;
	fbc->g = new_color->g * sf;
	fbc->b = new_color->b * sf;
}

static bool _compute_px_color(cRGB_t * color, 
							  const barycentric_t *bc, 
							  const float * weight1, const float * weight2, const float * weight3,
							  const texture_t * texture,
							  const cRGB_t * v1c, const cRGB_t * v2c, const cRGB_t * v3c,
							  const vec2_t * v1t, const vec2_t * v2t, const vec2_t * v3t,
							  const int *texId) {
			
	const float z0 = bc->bc0*(*weight1);
	const float z1 = bc->bc1*(*weight2);
	const float z2 = bc->bc2*(*weight3);
	const float z3 = 1.f/(z0 + z1 + z2);
	
	switch(*texId) {
		case -1: {
			color->r = (z0*v1c->r + z1*v2c->r + z2*v3c->r ) * z3;
			color->g = (z0*v1c->g + z1*v2c->g + z2*v3c->g ) * z3;
			color->b = (z0*v1c->b + z1*v2c->b + z2*v3c->b ) * z3;
			}
			break;
		default: {
				if ( texture == NULL ) return false;

				float texx = ( z0*v1t->x + z1*v2t->x + z2*v3t->x ) * z3 * texture->width;
				float texy = ( z0*v1t->y + z1*v2t->y + z2*v3t->y ) * z3 * texture->height;

				//texx = ceilf(interpolate_lin(texx, 0.f, 0.f, 1.f, texture->width));
				//texy = ceilf(interpolate_lin(texy, 0.f, 0.f, 1.f, texture->height));
				crgb_array2D_get(texture->buffer, (int)texx, (int)texy, color);
				//printf("tx: %f ty: %f, rgb: %f %f %f\n", texx, texy, color->r, color->g, color->b);

				/*cRGB_t * txc = &((cRGB_t *)texture->buffer->entries)[(int)texy * texture->width + (int)texx];
				color->r = txc->r;
				color->g = txc->g;
				color->b = txc->b;
				*/
				if ( color->r == 1.f && color->g == 0.f && color->b == 1.f  )
				{
					return false;
				}
			}						
			break;
	}

	return true;
}


static void update_sample(vec3_t * _pixelSample, const vec2_t ** _cursample,
						  const unsigned int *curW, const unsigned int *curH
						  ) {
	const vec2_t * cursample = *_cursample;
	vec3_t * pixelSample = _pixelSample;
	
	pixelSample->x = *curW;
	pixelSample->x += cursample->x;
	pixelSample->y = *curH;
	pixelSample->y += cursample->y;
	++(*_cursample);
}

static bool _compute_and_set_z_point(const float * rz1, const unsigned int * bi, float * zBuffer) {
		
	float * old_z = zBuffer + *bi;
             
	if ( *rz1 < *old_z ) { return true; }
	
	*old_z = *rz1;	
	
	return false;
}

static bool _compute_sample_and_check_point(vec3_t * _pixelSample, const vec2_t ** _cursample,
										 unsigned int *curW, unsigned int *curH,
										 const unsigned int *imgW, const unsigned int *imgH,
										 const vec3_t * pRaster1) { 

	vec3_t * pixelSample = _pixelSample;
	update_sample(pixelSample, _cursample, curW, curH);

	if ((pRaster1->x >= 0) &&
	 (pRaster1->x <= *imgW) &&
	 (pRaster1->y >= 0) &&
	 (pRaster1->y <= *imgH)) {
		*curW = pRaster1->x;
		*curH = pRaster1->y;
		return false;
	 }
	 
	return true;
}

/*
		if(	(((bc->w0_12 = (pixelSample->x - pRaster2->x) * (pRaster3->y - pRaster2->y) - (pixelSample->y - pRaster2->y) * (pRaster3->x - pRaster2->x))<0.f) ||
		(( bc->w1_20  = (pixelSample->x - pRaster3->x) * (pRaster1->y - pRaster3->y) - (pixelSample->y - pRaster3->y) * (pRaster1->x - pRaster3->x))<0.f) ||	
		(( bc->w2_01  = (pixelSample->x - pRaster1->x) * (pRaster2->y - pRaster1->y) - (pixelSample->y - pRaster1->y) * (pRaster2->x - pRaster1->x))<0.f))) 
		{ return true;}
	
	bc->bc0 = bc->w0_12 * bc->area;
	bc->bc1 = bc->w1_20 * bc->area;
	bc->bc2 = bc->w2_01 * bc->area;

	bc.area = 1.f/((pRaster3.x - pRaster1.x) * (pRaster2.y - pRaster1.y) - (pRaster3.y - pRaster1.y) * (pRaster2.x - pRaster1.x));
*/
static bool _compute_sample_bc_and_check_line(vec3_t * _pixelSample, const vec2_t ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 barycentric_t * _bc, const vec3_t * pRaster1, const vec3_t * pRaster2) { 
	vec3_t * pixelSample = _pixelSample;
	barycentric_t * bc = _bc;
	update_sample(pixelSample, _cursample, curW, curH);

	bc->bc0 = (pixelSample->x - pRaster1->x ) * (pRaster2->y - pRaster1->y) - (pixelSample->y - pRaster1->y) * (pRaster2->x - pRaster1->x);

	/*if (bc->bc0 < 0) {
		bc->bc0 = (pixelSample->x - pRaster2->x ) * (pRaster1->y - pRaster2->y) - (pixelSample->y - pRaster2->y) * (pRaster1->x - pRaster2->x);
	}*/

	float edge = bc->bc0;

	vec3_t limitvec;
	vec3_sub_dest(&limitvec, pRaster2, pRaster1);
	float limit = vec3_length(&limitvec);
	limit *= 0.5f;
	if ( ( edge <= limit && edge >= -limit ) ) {

		vec2_t tmp;
		vec2_sub_dest(&tmp, (vec2_t*)pRaster2, (vec2_t*)pRaster1);
		float len = vec2_length(&tmp);
		vec2_sub_dest(&tmp, (vec2_t*)pixelSample, (vec2_t*)pRaster1);
		float len2 = vec2_length(&tmp);
		/*(1)*/
		bc->bc0 = len2 / len;
		bc->bc1 = (len-len2) / len;
			
		/*(2)
		bc->bc0 = len;
		bc->bc1 = len2;
		*/
		return false;
	} 
	return true;								 
}


static bool _compute_and_set_z_line(const float * rz1, const float * rz2, const barycentric_t *bc, 
								    const unsigned int * bi, float * zBuffer) {
	/*(1)*/
	float z = *rz1;
	if ( *rz1 != *rz2 ) {
		z =  *rz1 * bc->bc1; 
		z += *rz2 * bc->bc0; 
	}

	/*(2)
	float _rz1a = bc->bc1 / bc->bc0;
	float _rz2a = (bc->bc0 - bc->bc1) / bc->bc0;
	float z = interpolate_lin(bc->bc1, 0.f ,*rz1 * _rz2a, bc->bc0, *rz2 * _rz1a);
	*/
	float * old_z = zBuffer + *bi;

	if ( z < *old_z ) { return true; }	

	*old_z = z;
	
	return false;
}

static bool _world_to_raster_line(const vec3_t * _v, vec3_t * _ndc, vec3_t * _raster, float * _weight,
							 const float * imgW_h, const float * imgH_h, float * rz3, const mat4_t * _ct) {
	
	vec3_t * ndc = _ndc;
	const vec3_t * v = _v;
	vec3_t * raster = _raster;
	float * weight = _weight;
	const mat4_t * ct = _ct;
	
	ndc->x += (v->x * ct->_11) + (v->y * ct->_12) + (v->z * ct->_13);// + ct->_14;
	ndc->y += (v->x * ct->_21) + (v->y * ct->_22) + (v->z * ct->_23);// + ct->_24;
	ndc->z += (v->x * ct->_31) + (v->y * ct->_32) + (v->z * ct->_33);// + ct->_34;
	*weight += (v->x * ct->_41) + (v->y * ct->_42) + (v->z * ct->_43);// + ct->_44;
	
	//if (*weight < 0.f) return true;
	
	if (*weight != 1.f && *weight != 0.f){
		*weight = 1.f/(*weight); ndc->x *= *weight; ndc->y *= *weight; ndc->z *= *weight;
	}
	
	raster->x = (ndc->x + 1.f) * (*imgW_h);
	raster->y = (1.f-ndc->y) * (*imgH_h);
	raster->z = ndc->z;
	*rz3 = 1.f/raster->z;

	return false;
}

/**
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

static void render_line_in_point_mode(renderer_t * _renderer, const shape_t *  shape) {
	//VARS
	renderer_t * renderer = _renderer;
	const camera_t * cam = &renderer->camera;
	const mat4_t * ct = &cam->transformation;
	const vertex_t ** vertices = (const vertex_t **)shape->vertices;
	const vertex_t * v1 = (const vertex_t *)vertices[0]; 
	const vertex_t * v2 = (const vertex_t *)vertices[1];
	const cRGB_t * v1c = &v1->color;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	cRGB_t * frameBuffer = renderer->frameBuffer;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2, factor = 1.f;


	//const vec3_t * v1v =  &v1->vec, * v2v =  &v2->vec;

	vec3_t _v1v, _v2v;
	const vec3_t * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	vec3_t normR1 = { fmaxf(0.f, fminf(pRaster1.x, imgW)), fmaxf(0.f, fminf(pRaster1.y, imgH)), pRaster1.z };
	vec3_t normR2 = { fmaxf(0.f, fminf(pRaster2.x, imgW)), fmaxf(0.f, fminf(pRaster2.y, imgH)), pRaster2.z };

	unsigned int bi1 = (normR1.y * bufWidth + (normR1.x * used_samples));
	unsigned int bi2 = (normR2.y * bufWidth + (normR2.x * used_samples));

	for (unsigned int sample = used_samples; sample--;) {
		
		unsigned int bi = bi1 + sample;

		_set_color_to_fb_(frameBuffer,&bi ,&factor,v1c);

		bi = bi2 + sample;

		_set_color_to_fb_(frameBuffer,&bi ,&factor,v1c);

	}	
}

/*
static void gfx_draw_on_canvas(int32_t const * const x, int32_t const * const y, void *data) {
	cdCanvasPixel((cdCanvas *)data, *x, cdCanvasInvertYAxis((cdCanvas *)data, *y), 0);
}

static void _gfx_algo_test_draw_line_trigger(Ihandle *_ih) {
	Ihandle *ih = _ih;

	vec2_t start = { IupGetInt((Ihandle *)IupGetAttribute(ih, "lx0"), "SPINVALUE"), IupGetInt((Ihandle *)IupGetAttribute(ih, "ly0"), "SPINVALUE") };
	vec2_t end = { IupGetInt((Ihandle *)IupGetAttribute(ih, "lx1"), "SPINVALUE"), IupGetInt((Ihandle *)IupGetAttribute(ih, "ly1"), "SPINVALUE") };

	void * data = IupGetAttribute((Ihandle*)IupGetAttribute(ih, "gfx_canvas"), "GFX_TEST_CD_CANVAS_DBUFFER");
	geometry_line(&start, &end, gfx_draw_on_canvas, data);
}
*/
typedef struct {
	vec2_t *start; 
	vec2_t* end;
	renderer_t * renderer;
	float factor;
	const cRGB_t * color;
} renderer_2d_line_ctx_t;

//_set_color_to_fb_(frameBuffer,&bi ,&factor,v1c);
static void _2d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) {
	renderer_2d_line_ctx_t* ctx = (renderer_2d_line_ctx_t*)data;
	renderer_t *renderer = ctx->renderer;

	if ( *x >= renderer->imgWidth || *x < 0 || 
		*y >= renderer->imgHeight || *y < 0 ) return;

	for (unsigned int sample = renderer->used_samples; sample--;) {
		
		 unsigned int bi = ((unsigned int)*y * renderer->bufWidth ) + ( *x * renderer->used_samples ) + sample;
		_set_color_to_fb_(renderer->frameBuffer, &bi , &ctx->factor, ctx->color);

	}

}

static void _draw_2D_line_to_renderer(renderer_t * _renderer, vec2_t *start, vec2_t* end, const cRGB_t * color) {
	renderer_2d_line_ctx_t ctx = { start, end, _renderer, 1.f/_renderer->used_samples, color };
	geometry_line(start, end, _2d_line_to_framebuffer, &ctx);
}

static void render_line_in_line_mode(renderer_t * _renderer, const shape_t *  shape) {

	//VARS
	renderer_t * renderer = _renderer;
	const camera_t * cam = &renderer->camera;
	const mat4_t * ct = &cam->transformation;
	const vertex_t ** vertices = (const vertex_t **)shape->vertices;
	const vertex_t * v1 = (const vertex_t *)vertices[0]; 
	const vertex_t * v2 = (const vertex_t *)vertices[1];
	const cRGB_t * v1c = &v1->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2;


	//const vec3_t * v1v =  &v1->vec, * v2v =  &v2->vec;

	vec3_t _v1v, _v2v;
	const vec3_t * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	vec2_t start = { pRaster1.x, pRaster1.y };
	vec2_t end = { pRaster2.x, pRaster2.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v1c);
}

typedef struct {
	vec3_t *pRaster1; 
	vec3_t *pRaster2;
	renderer_t * renderer;
	const cRGB_t * color;
	float *rz1;
	float *rz2;
	unsigned int cntDeltaVecs;
	vec2_t *deltaVecs;
} renderer_3d_line_ctx_t;

static void _3d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) 
{
	renderer_3d_line_ctx_t* ctx = (renderer_3d_line_ctx_t*)data;
	renderer_t *renderer = ctx->renderer;

	const unsigned int _x = (const unsigned int)*x;
	const unsigned int _y = (const unsigned int)*y;

	if ( _x >= (unsigned int)(renderer->imgWidth - 1) || _y >= (unsigned int)(renderer->imgHeight - 1) ) return;

	const vec2_t * samples = renderer->samples;
	const vec2_t * cursample;
	const unsigned int used_samples = renderer->used_samples;
	const float *sample_factor = &renderer->sample_factor;
	const int bufWidth = renderer->bufWidth;

	vec3_t pixelSample;
	vec3_t *pRaster1 = ctx->pRaster1;
	vec3_t *pRaster2 = ctx->pRaster2;
	barycentric_t bc;
	float *rz1 = ctx->rz1;
	float *rz2 = ctx->rz2;

	float * zBuffer = renderer->zBuffer;
	cRGB_t * frameBuffer = renderer->frameBuffer;

	const cRGB_t *color = ctx->color; 

	for ( uint32_t curDeltaVec = 0; curDeltaVec < ctx->cntDeltaVecs; curDeltaVec++ )
	{
		vec2_t *deltaVec = &ctx->deltaVecs[curDeltaVec];
		const unsigned int curH = _y + (unsigned int)deltaVec->y;
		const unsigned int curW = _x + (unsigned int)deltaVec->x;

		unsigned int curHbufWidth = curH * bufWidth;

		cursample = samples;
		unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
		for (unsigned int sample = used_samples; sample--;) {

			if ( _compute_sample_bc_and_check_line(&pixelSample, &cursample,&curW, &curH, &bc,
										pRaster1, pRaster2)) { continue; }
			
			unsigned int bi = curWused_samples + sample;
			
			if ( _compute_and_set_z_line(rz1, rz2, &bc, &bi, zBuffer) )  { continue; }

			_set_color_to_fb_(frameBuffer,&bi ,sample_factor, color);
		}
	}

}

static void _draw_3D_line_to_renderer(renderer_t * _renderer, vec2_t *start, vec2_t* end, const cRGB_t * color,
	float *rz1, float *rz2, vec3_t *pRaster1, vec3_t *pRaster2) 
{
	vec2_t deltaVecs[5] = { {0.f , 0.f}, {-1.f , 0.f}, {1.f , 0.f}, {0.f , 1.f}, {0.f , -1.f} };
	renderer_3d_line_ctx_t ctx = { 
		pRaster1, pRaster2, _renderer, color, rz1, rz2, 5, &deltaVecs[0]		
	};
	geometry_line(start, end, _3d_line_to_framebuffer, &ctx);
}

static void render_line(renderer_t * _renderer, const shape_t * shape){
	//VARS
	renderer_t * renderer = _renderer;
	const camera_t * cam = &renderer->camera;
	const mat4_t * ct = &cam->transformation;
	const vertex_t ** vertices = (const vertex_t **)shape->vertices;
	const vertex_t * v1 = (const vertex_t *)vertices[0]; 
	const vertex_t * v2 = (const vertex_t *)vertices[1];
	const cRGB_t * v1c = &v1->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2;

	vec3_t _v1v, _v2v;
	const vec3_t * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	//_compute_min_max_w_h_line(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH, &pRaster1, &pRaster2);
	
	vec2_t start = { pRaster1.x, pRaster1.y };
	vec2_t end = { pRaster2.x, pRaster2.y };

	_draw_3D_line_to_renderer(renderer, &start, &end, v1c, &rz1, &rz2, &pRaster1, &pRaster2);
}

#if 0
	/**
		returns false if pixel sample is inside triangle otherwise false.
	*/
#endif
static bool _compute_sample_bc_and_check(vec3_t * _pixelSample, const vec2_t ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 barycentric_t * _bc,
										 const vec3_t * pRaster1, const vec3_t * pRaster2, const vec3_t * pRaster3) {
	vec3_t * pixelSample = _pixelSample;
	barycentric_t * bc = _bc;
	update_sample(pixelSample, _cursample, curW, curH);
	
	if(	(((bc->w0_12 = (pixelSample->x - pRaster2->x) * (pRaster3->y - pRaster2->y) - (pixelSample->y - pRaster2->y) * (pRaster3->x - pRaster2->x))<0.f) ||
		(( bc->w1_20  = (pixelSample->x - pRaster3->x) * (pRaster1->y - pRaster3->y) - (pixelSample->y - pRaster3->y) * (pRaster1->x - pRaster3->x))<0.f) ||	
		(( bc->w2_01  = (pixelSample->x - pRaster1->x) * (pRaster2->y - pRaster1->y) - (pixelSample->y - pRaster1->y) * (pRaster2->x - pRaster1->x))<0.f))) 
		{ return true;}
	
	bc->bc0 = bc->w0_12 * bc->area;
	bc->bc1 = bc->w1_20 * bc->area;
	bc->bc2 = bc->w2_01 * bc->area;
	
	return false;
}

#if 0
	/**
		returns true if pixel should skipped otherwise false.
	*/
#endif
static bool _compute_and_set_z(const float * rz1, const float * rz2, const float * rz3,
							   const barycentric_t *bc, const unsigned int * bi,
							   float * zBuffer) {
	float z = (*rz1 * bc->bc0);
	z += (*rz2 * bc->bc1);
	z += (*rz3 * bc->bc2); 
	
	float *old_z = zBuffer + *bi;

	//printf(" rz1: %f , rz2: %f, rz3: %f, bc->bc0: %f, bc->bc1: %f , bc->bc2: %f ,bc->area: %f ,old_z: %f new z: %f\n",
	//						*rz1, *rz2, *rz3, bc->bc0, bc->bc1, bc->bc2, bc->area,*old_z, z);

	if ( z < *old_z ) { return true; }

	*old_z = z;			
	
	//only for z buffer print 
	//renderer->min_z = fminf(renderer->min_z, z);
	//renderer->max_z = fmaxf(renderer->max_z, z);
	return false;
}

static void _compute_min_max_w_h(float *maxx, float *maxy, float *minx, float *miny,
								 unsigned int *curW, unsigned int *curH, const unsigned int *imgW, const unsigned int *imgH,
								 const vec3_t * pRaster1, const vec3_t * pRaster2, const vec3_t * pRaster3) {
	*maxx = fminf((float)*imgW, fmaxf(pRaster1->x, fmaxf(pRaster2->x, pRaster3->x)));
	*maxy = fminf((float)*imgH, fmaxf(pRaster1->y, fmaxf(pRaster2->y, pRaster3->y)));
	*minx = fmaxf(0.f, fminf(pRaster1->x, fminf(pRaster2->x, pRaster3->x)));
	*miny = fmaxf(0.f, fminf(pRaster1->y, fminf(pRaster2->y, pRaster3->y)));
	*curH = *miny;
	*curW = *minx;
}

#if 0
	/**
		returns false if transformation is high negative otherwise false. This is to prevent
		mirroring negative perspective depth projection.
	*/
#endif
static bool _world_to_raster(const vec3_t * _v, vec3_t * _ndc, vec3_t * _raster, float * _weight,
							 const float * imgW_h, const float * imgH_h, float * rz3, const mat4_t * _ct) {
	
	vec3_t * ndc = _ndc;
	const vec3_t * v = _v;
	vec3_t * raster = _raster;
	float * weight = _weight;
	const mat4_t * ct = _ct;
	
	ndc->x += (v->x * ct->_11) + (v->y * ct->_12) + (v->z * ct->_13);// + ct->_14;
	ndc->y += (v->x * ct->_21) + (v->y * ct->_22) + (v->z * ct->_23);// + ct->_24;
	ndc->z += (v->x * ct->_31) + (v->y * ct->_32) + (v->z * ct->_33);// + ct->_34;
	*weight += (v->x * ct->_41) + (v->y * ct->_42) + (v->z * ct->_43);// + ct->_44;
	
	if (*weight < 0.f) return true;
	
	if (*weight != 1.f && *weight != 0.f){
		*weight = 1.f/(*weight); ndc->x *= *weight; ndc->y *= *weight; ndc->z *= *weight;
	}
	
	raster->x = (ndc->x + 1.f) * (*imgW_h);
	raster->y = (1.f-ndc->y) * (*imgH_h);
	raster->z = ndc->z;
	*rz3 = 1.f/raster->z;
	
	return false;
}

static void render_point(renderer_t * renderer, const shape_t * shape){
		//VARS
	const camera_t * cam = &renderer->camera;
	const mat4_t * ct = &cam->transformation;
	const vertex_t ** vertices = (const vertex_t **)shape->vertices;
	const vertex_t * v1 = (const vertex_t *)vertices[0]; 
	const vec3_t * v1v = &v1->vec;
	const cRGB_t * v1c = &v1->color;
	const vec2_t * samples = renderer->samples;
	const vec2_t * cursample;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half, sample_factor = renderer->sample_factor;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, pRaster1, pixelSample;
	cRGB_t * frameBuffer = renderer->frameBuffer;
	unsigned int curW, curH;
	float maxx, maxy, minx, miny, weight1 = ct->_44, rz1;
	float * zBuffer = renderer->zBuffer;
	//EO VARS
	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return;

	maxx = 1.f; maxy = 1.f; minx = 0.f; miny = 0.f;
	curH = miny; curW = minx;
	//EO BOUNDING BOX
	for(; curH < maxy; ++curH) {
		for(curW = minx; curW < maxx; ++curW) {
			cursample = samples;
			for (unsigned int sample = used_samples; sample--;) {
				
				if ( _compute_sample_and_check_point(
							&pixelSample, &cursample,&curW, &curH, &imgW, &imgH, &pRaster1)) { continue; }
				
				unsigned int bi = curH * bufWidth + (curW * used_samples) + sample;		
				
				if ( _compute_and_set_z_point(&rz1, &bi, zBuffer) )  { continue; }
				
				_set_color_to_fb_(frameBuffer,&bi ,&sample_factor,v1c);
			}
		}
	}
}

static void render_triangle_in_point_mode(renderer_t *  renderer, const shape_t *  shape) {
	const camera_t *  cam = &renderer->camera;
	const mat4_t *  ct = &cam->transformation;
	const vertex_t **  vertices = (const vertex_t **)shape->vertices;
	const vertex_t *  v1 = (const vertex_t *)vertices[0]; 
	const vertex_t *  v2 = (const vertex_t *)vertices[1];
	const vertex_t *  v3 = (const vertex_t *)vertices[2];
	const vec3_t *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const cRGB_t *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const int bufWidth = renderer->bufWidth;
	const unsigned int used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3;
	cRGB_t *  frameBuffer = renderer->frameBuffer;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3, factor = 1.f;

	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	bool is1in = (pRaster1.x < renderer->imgWidth) && (pRaster1.x >= 0) && (pRaster1.y < renderer->imgHeight) && (pRaster1.y >= 0 );
	bool is2in = (pRaster2.x < renderer->imgWidth) && (pRaster2.x >= 0) && (pRaster2.y < renderer->imgHeight) && (pRaster2.y >= 0 );
	bool is3in = (pRaster3.x < renderer->imgWidth) && (pRaster3.x >= 0) && (pRaster3.y < renderer->imgHeight) && (pRaster3.y >= 0 );

	for (unsigned int sample = used_samples; sample--;) {
		
		
		if ( is1in )  { 
			unsigned int bi1 = (((unsigned int)pRaster1.y * bufWidth) + ((unsigned int)pRaster1.x * used_samples));
			unsigned int bi = bi1 + sample;
			_set_color_to_fb_(frameBuffer,&bi ,&factor,v1c); 
		}
		
		if ( is2in)  {
			unsigned int bi2 = (((unsigned int)pRaster2.y * bufWidth) + ((unsigned int)pRaster2.x * used_samples));
			unsigned int bi = bi2 + sample;
			_set_color_to_fb_(frameBuffer,&bi ,&factor,v2c); 
		}
		
		if ( is3in )  {
			unsigned int bi3 = (((unsigned int)pRaster3.y * bufWidth) + ((unsigned int)pRaster3.x * used_samples));
			unsigned int bi = bi3 + sample;
			_set_color_to_fb_(frameBuffer,&bi ,&factor,v3c); 
		}
		
	}	
}

static void render_triangle_in_line_mode(renderer_t *  renderer, const shape_t *  shape) {
	const camera_t *  cam = &renderer->camera;
	const mat4_t *  ct = &cam->transformation;
	const vertex_t **  vertices = (const vertex_t **)shape->vertices;
	const vertex_t *  v1 = (const vertex_t *)vertices[0]; 
	const vertex_t *  v2 = (const vertex_t *)vertices[1];
	const vertex_t *  v3 = (const vertex_t *)vertices[2];
	const vec3_t *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const cRGB_t *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3;

	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	vec2_t start = { pRaster1.x, pRaster1.y };
	vec2_t end = { pRaster2.x, pRaster2.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v1c);

	start = (vec2_t){ pRaster2.x, pRaster2.y };
	end = (vec2_t){ pRaster3.x, pRaster3.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v2c);

	start = (vec2_t){ pRaster3.x, pRaster3.y };
	end = (vec2_t){ pRaster1.x, pRaster1.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v3c);
}

static void render_triangle(renderer_t *  renderer, const shape_t *  shape){
	//VARS
	const camera_t *  cam = &renderer->camera;
	const mat4_t *  ct = &cam->transformation;
	const vertex_t **  vertices = (const vertex_t **)shape->vertices;
	const vertex_t *  v1 = (const vertex_t *)vertices[0]; 
	const vertex_t *  v2 = (const vertex_t *)vertices[1];
	const vertex_t *  v3 = (const vertex_t *)vertices[2];
	const vec3_t *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const cRGB_t *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const vec2_t *  v1t = &v1->texCoord, * v2t = &v2->texCoord, * v3t = &v3->texCoord;
	const vec2_t *  samples = renderer->samples;
	const vec2_t *  cursample;
	const int texId = shape->texId;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half, sample_factor = renderer->sample_factor;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3, pixelSample;
	cRGB_t *  frameBuffer = renderer->frameBuffer;
	barycentric_t bc;
	unsigned int curW, curH;
	float maxx, maxy, minx, miny, 
		  weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3;
	float *  zBuffer = renderer->zBuffer;

	cRGB_t curCol1;
	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	bc.area = 1.f/((pRaster3.x - pRaster1.x) * (pRaster2.y - pRaster1.y) - (pRaster3.y - pRaster1.y) * (pRaster2.x - pRaster1.x));

	_compute_min_max_w_h(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH,
						 &pRaster1, &pRaster2, &pRaster3);
	
	texture_cache_t * cache = renderer->texture_cache;
	texture_t *texture = texture_cache_get(cache, (unsigned int)shape->texId); 

	for(; curH < maxy; ++curH) {
		unsigned int curHbufWidth = curH * bufWidth;
		for(curW = minx; curW < maxx; ++curW) {
			cursample = samples;
			unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
			for (unsigned int sample = used_samples; sample--;) {

				if ( _compute_sample_bc_and_check(&pixelSample, &cursample,&curW, &curH, &bc,
										 &pRaster1, &pRaster2, &pRaster3)) { continue; }
				
				unsigned int bi = curWused_samples + sample;
				
				//if ( (curW >= low && curW < up) && (curH >= low && curH < up) ) {
				//	printf("----- CUBE w/h: %i/%i ", curW, curH);
					if ( _compute_and_set_z(&rz1, &rz2, &rz3, &bc, &bi, zBuffer) )  { continue; }
					
					if ( _compute_px_color(&curCol1, &bc, &weight1, &weight2, &weight3,
									texture, v1c, v2c, v3c, v1t, v2t, v3t, &texId))
					{
						_set_color_to_fb_(frameBuffer,&bi ,&sample_factor,&curCol1);
					}
					//EO COLOR AND TEX
				//}
			}
		}
	}
	
}

void render_shape(renderer_t *  renderer, const shape_t *  shape){
	const shape_t *  curshape = shape;
	const renderer_t * curRenderer = renderer;
	switch(curshape->cntVertex){
		case 3:
			curRenderer->TRIANGLE_RENDER_FUNC(renderer, curshape); break;
		case 2:
			curRenderer->LINE_RENDER_FUNC(renderer, curshape); break;
		case 1:
			curRenderer->POINT_RENDER_FUNC(renderer, curshape); break;
		default:
			printf("Unknown vertex count oO\n");
	}
}

void render_mesh(renderer_t * renderer, const mesh_t *  mesh){
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

void renderer_clear_frame(renderer_t * renderer){
	const int buffersize = renderer->imgWidth * 
						   renderer->imgHeight * 
						   renderer->samplestep * renderer->samplestep;
	memset(renderer->zBuffer, CHAR_MIN, buffersize * sizeof(float));
	memset(renderer->frameBuffer, 0, buffersize * sizeof(cRGB_t));
	renderer->max_z = RENDER_FLT_MAX;
	renderer->min_z = 0.f;
}


void renderer_set_vmode_solid(renderer_t * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle;
}

void renderer_set_vmode_point(renderer_t * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line_in_point_mode;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle_in_point_mode;
}

void renderer_set_vmode_line(renderer_t * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line_in_line_mode;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle_in_line_mode;
}

renderer_t * 
renderer_new(int imgWidth, int imgHeight, cRGB_t * bgColor, unsigned int samplestep){
	renderer_t * newrenderer = malloc(sizeof(renderer_t));
	newrenderer->projection = RP_ORTHOGRAPHIC;
	//newrenderer->texture = NULL;
	newrenderer->imgWidth = imgWidth;
	newrenderer->imgHeight = imgHeight;
	newrenderer->imgWidth_half = 0.5f * imgWidth;
	newrenderer->imgHeight_half = 0.5f * imgHeight;
	unsigned int us = samplestep*samplestep;
	unsigned int bw = imgWidth * us;
	newrenderer->bufWidth = bw;
	newrenderer->bufHeight = imgHeight;
	newrenderer->samplestep = samplestep;
	unsigned int buffersize = imgWidth * imgHeight * us;
	newrenderer->frameBuffer = malloc(buffersize * sizeof(cRGB_t));
	newrenderer->zBuffer = malloc(buffersize * sizeof(float));
	crgb_crgb_copy(&newrenderer->bgcolor, bgColor);
	renderer_clear_frame(newrenderer);
	newrenderer->min_z = RENDER_FLT_MAX;
	newrenderer->max_z = 0.f;
	#if 0
		//add samples logic
	#endif
	newrenderer->used_samples = us;
	newrenderer->sample_factor = 1.f/newrenderer->used_samples;
	float stepstart = 0.5f / (float)samplestep; //for st = 2 step is .25  for st = 4 0.125
	float step = 2.f*stepstart; //distance between 

	newrenderer->samples = malloc(newrenderer->used_samples * sizeof(vec2_t));

	for( unsigned int sy = 0; sy < samplestep ;++sy ){
		for( unsigned int sx = 0; sx < samplestep ;++sx ){
			vec2_t  *cursample = &newrenderer->samples[sy * samplestep + sx];
			cursample->x = stepstart + (sx * step);
			cursample->y = stepstart + (sy * step);
		}
	}
	
	renderer_set_vmode_solid(newrenderer);
	newrenderer->texture_cache = texture_cache_new();

	return newrenderer;
}

void 
renderer_free(renderer_t * renderer){
	free(renderer->frameBuffer);
	free(renderer->zBuffer);
	free(renderer->samples);
	//free(renderer->wh_index);
	texture_cache_free(&renderer->texture_cache);
	//if (renderer->texture != NULL) {
	//	texture_free(renderer->texture);
	//}
	free(renderer);
}


void 
renderer_output_ppm(renderer_t * renderer, const char * filename){
	unsigned int colcnt=0, bi=0, samplestart;
	int i, j, imgW = renderer->imgWidth, imgH = renderer->imgHeight;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", imgW, imgH);
	cRGB_t fc;
	unsigned char color[3*imgW*imgH];
    for (j = 0; j < imgH; ++j){
	  bi = j * renderer->bufWidth;
	  for (i = 0; i < imgW; ++i){
		fc.r = 0.f, fc.g = 0.f, fc.b = 0.f;
		samplestart = bi + (i*renderer->used_samples);
		for (unsigned int sample = renderer->used_samples; sample--;){
			cRGB_t * c = &renderer->frameBuffer[samplestart + sample];
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
renderer_output_z_buffer_ppm(renderer_t * renderer, const char * filename){
	unsigned int colcnt=0,bi=0,samplestart;
	int i, j, imgW = renderer->imgWidth, imgH = renderer->imgHeight;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", imgW, imgH);
	float _color, samplefactor = renderer->sample_factor;
	unsigned char color[3*imgW*imgH];
	
    for (j = 0; j < imgH; ++j){
	  bi = j * renderer->bufWidth;
	  for (i = 0; i < imgW; ++i){
		_color = 0.f;
		samplestart = bi + (i*renderer->used_samples);
		for (unsigned int sample = renderer->used_samples; sample--;){
			_color += renderer->zBuffer[samplestart + sample];
		}
		
		_color *= samplefactor ;
		
		if ( _color != RENDER_FLT_MAX ){
			_color = (unsigned char)interpolate_lin(_color, renderer->max_z, 0.f, renderer->min_z, 255.f);
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
