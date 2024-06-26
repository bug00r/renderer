#include <stdlib.h>
#include <stdio.h>
#include "renderer.h"


static void _set_Coloro_fb_(ColorRGB * frameBuffer,
							  const unsigned int * bi , 
							  const float * sample_factor,
							  const ColorRGB * new_color) {
	ColorRGB * fbc = frameBuffer + *bi;
	const float sf = *sample_factor;
	fbc->r = new_color->r * sf;
	fbc->g = new_color->g * sf;
	fbc->b = new_color->b * sf;
}

static bool _compute_px_color(ColorRGB * color, 
							  const Barycentric *bc, 
							  const float * weight1, const float * weight2, const float * weight3,
							  const Texture * texture,
							  const ColorRGB * v1c, const ColorRGB * v2c, const ColorRGB * v3c,
							  const Vec2 * v1t, const Vec2 * v2t, const Vec2 * v3t,
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

				crgb_array2D_get(texture->buffer, (int)texx, (int)texy, color);

				if ( color->r == 1.f && color->g == 0.f && color->b == 1.f  )
				{
					return false;
				}
			}						
			break;
	}

	return true;
}


static void update_sample(Vec3 * _pixelSample, const Vec2 ** _cursample,
						  const unsigned int *curW, const unsigned int *curH
						  ) {
	const Vec2 * cursample = *_cursample;
	Vec3 * pixelSample = _pixelSample;
	
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

static bool _compute_sample_and_check_point(Vec3 * _pixelSample, const Vec2 ** _cursample,
										 unsigned int *curW, unsigned int *curH,
										 const unsigned int *imgW, const unsigned int *imgH,
										 const Vec3 * pRaster1) { 

	Vec3 * pixelSample = _pixelSample;
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

static bool _compute_sample_bc_and_check_line(const float *_limit, const float *_raster_len, const float *_raster_len_inv, Vec3 * _pixelSample, const Vec2 ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 Barycentric * _bc, const Vec3 * pRaster1, const Vec3 * pRaster2) { 
	Vec3 * pixelSample = _pixelSample;
	Barycentric * bc = _bc;
	update_sample(pixelSample, _cursample, curW, curH);

	bc->bc0 = (pixelSample->x - pRaster1->x ) * (pRaster2->y - pRaster1->y) - (pixelSample->y - pRaster1->y) * (pRaster2->x - pRaster1->x);

	float edge = bc->bc0;

	const float limit = (const float)*_limit;
	const float raster_len_inv = (const float)*_raster_len_inv;
	const float raster_len = (const float)*_raster_len;

	if ( ( edge <= limit && edge >= -limit ) ) {

		Vec2 tmp;
		vec2_sub_dest(&tmp, (Vec2*)pixelSample, (Vec2*)pRaster1);
		float len2 = vec2_length(&tmp);

		bc->bc0 = len2 * raster_len_inv;
		bc->bc1 = (raster_len - len2) * raster_len_inv;
			
		return false;
	} 
	return true;								 
}


static bool _compute_and_set_z_line(const float * rz1, const float * rz2, const Barycentric *bc, 
								    const unsigned int * bi, float * zBuffer) {

	float z = *rz1;
	if ( *rz1 != *rz2 ) {
		z =  *rz1 * bc->bc1; 
		z += *rz2 * bc->bc0; 
	}

	float * old_z = zBuffer + *bi;

	if ( z < *old_z ) { return true; }	

	*old_z = z;
	
	return false;
}

static bool _world_to_raster_line(const Vec3 * _v, Vec3 * _ndc, Vec3 * _raster, float * _weight,
							 const float * imgW_h, const float * imgH_h, float * rz3, const Mat4 * _ct) {
	
	Vec3 * ndc = _ndc;
	const Vec3 * v = _v;
	Vec3 * raster = _raster;
	float * weight = _weight;
	const Mat4 * ct = _ct;
	
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
static bool __line_filter_or_clip(Renderer* _renderer, Vec3* _dest_start, Vec3* _dest_end,
								  const Vec3* _line_start, const Vec3* _line_end ) {
	
	Renderer* renderer = _renderer;
	Plane *near = &renderer->camera.frustum.near;
	Vec3 *normal = &near->normal;

	Vec3	*line_start = (Vec3*)_line_start;
	Vec3	*line_end = (Vec3*)_line_end;

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

	Vec3 intersect;
	if (mu_line_plane_intersection_normal(&intersect, line_start, line_end, &near->lb, normal)) {
		Vec3 *to_clip = ( start_out ? _dest_start : _dest_end);
		vec3_copy_dest(to_clip, &intersect);
		//printf("line clip: ");
		//vec3_print(to_clip);
	}

	return true;
}

static void render_line_in_point_mode(Renderer * _renderer, const Shape *  shape) {
	//VARS
	Renderer * renderer = _renderer;
	const Camera * cam = &renderer->camera;
	const Mat4 * ct = &cam->transformation;
	const Vertex ** vertices = (const Vertex **)shape->vertices;
	const Vertex * v1 = (const Vertex *)vertices[0]; 
	const Vertex * v2 = (const Vertex *)vertices[1];
	const ColorRGB * v1c = &v1->color;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	ColorRGB * frameBuffer = renderer->frameBuffer;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2, factor = 1.f;

	Vec3 _v1v, _v2v;
	const Vec3 * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	Vec3 normR1 = { fmaxf(0.f, fminf(pRaster1.x, imgW)), fmaxf(0.f, fminf(pRaster1.y, imgH)), pRaster1.z };
	Vec3 normR2 = { fmaxf(0.f, fminf(pRaster2.x, imgW)), fmaxf(0.f, fminf(pRaster2.y, imgH)), pRaster2.z };

	unsigned int bi1 = (normR1.y * bufWidth + (normR1.x * used_samples));
	unsigned int bi2 = (normR2.y * bufWidth + (normR2.x * used_samples));

	for (unsigned int sample = used_samples; sample--;) {
		
		unsigned int bi = bi1 + sample;

		_set_Coloro_fb_(frameBuffer,&bi ,&factor,v1c);

		bi = bi2 + sample;

		_set_Coloro_fb_(frameBuffer,&bi ,&factor,v1c);

	}	
}

typedef struct {
	Vec2 *start; 
	Vec2* end;
	Renderer * renderer;
	float factor;
	const ColorRGB * color;
} renderer_2d_line_ctx_t;


static void _2d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) {
	renderer_2d_line_ctx_t* ctx = (renderer_2d_line_ctx_t*)data;
	Renderer *renderer = ctx->renderer;

	if ( *x >= renderer->imgWidth || *x < 0 || 
		*y >= renderer->imgHeight || *y < 0 ) return;

	for (unsigned int sample = renderer->used_samples; sample--;) {
		
		 unsigned int bi = ((unsigned int)*y * renderer->bufWidth ) + ( *x * renderer->used_samples ) + sample;
		_set_Coloro_fb_(renderer->frameBuffer, &bi , &ctx->factor, ctx->color);

	}

}

static void _draw_2D_line_to_renderer(Renderer * _renderer, Vec2 *start, Vec2* end, const ColorRGB * color) {
	renderer_2d_line_ctx_t ctx = { start, end, _renderer, 1.f/_renderer->used_samples, color };
	geometry_line(start, end, _2d_line_to_framebuffer, &ctx);
}

static void render_line_in_line_mode(Renderer * _renderer, const Shape *  shape) {

	//VARS
	Renderer * renderer = _renderer;
	const Camera * cam = &renderer->camera;
	const Mat4 * ct = &cam->transformation;
	const Vertex ** vertices = (const Vertex **)shape->vertices;
	const Vertex * v1 = (const Vertex *)vertices[0]; 
	const Vertex * v2 = (const Vertex *)vertices[1];
	const ColorRGB * v1c = &v1->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2;


	//const Vec3 * v1v =  &v1->vec, * v2v =  &v2->vec;

	Vec3 _v1v, _v2v;
	const Vec3 * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	Vec2 start = { pRaster1.x, pRaster1.y };
	Vec2 end = { pRaster2.x, pRaster2.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v1c);
}

typedef struct {
	Vec3 *pRaster1; 
	Vec3 *pRaster2;
	Renderer * renderer;
	const ColorRGB * color;
	float *rz1;
	float *rz2;
	unsigned int cntDeltaVecs;
	const Vec2 *deltaVecs;
	const float *limit;
	const float *raster_len;
	const float *raster_len_inv;
} renderer_3d_line_ctx_t;

static void _3d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) 
{
	renderer_3d_line_ctx_t* ctx = (renderer_3d_line_ctx_t*)data;
	Renderer *renderer = ctx->renderer;

	const unsigned int limitW = (unsigned int)(renderer->imgWidth - 1);
	const unsigned int limitH = (unsigned int)(renderer->imgHeight - 1);

	const unsigned int _x = (const unsigned int)*x;
	const unsigned int _y = (const unsigned int)*y;

	if ( _x > limitW || _y > limitH ) return;

	const Vec2 * samples = renderer->samples;
	const Vec2 * cursample;
	const unsigned int used_samples = renderer->used_samples;
	const float *sample_factor = &renderer->sample_factor;
	const int bufWidth = renderer->bufWidth;

	Vec3 pixelSample;
	Vec3 *pRaster1 = ctx->pRaster1;
	Vec3 *pRaster2 = ctx->pRaster2;
	Barycentric bc;
	float *rz1 = ctx->rz1;
	float *rz2 = ctx->rz2;

	float * zBuffer = renderer->zBuffer;
	ColorRGB * frameBuffer = renderer->frameBuffer;

	const ColorRGB *color = ctx->color; 

	const float *limit = ctx->limit;
	const float *raster_len = ctx->raster_len;
	const float *raster_len_inv = ctx->raster_len_inv;

	for ( uint32_t curDeltaVec = 0; curDeltaVec < ctx->cntDeltaVecs; curDeltaVec++ )
	{
		const Vec2 *deltaVec = &ctx->deltaVecs[curDeltaVec];
		const unsigned int curH = _y + (unsigned int)deltaVec->y;
		const unsigned int curW = _x + (unsigned int)deltaVec->x;

		if ( curW > limitW || curH > limitH ) continue;

		unsigned int curHbufWidth = curH * bufWidth;

		cursample = samples;
		unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
		for (unsigned int sample = used_samples; sample--;) {

			if ( _compute_sample_bc_and_check_line(limit, raster_len, raster_len_inv, &pixelSample, &cursample,&curW, &curH, &bc,
										pRaster1, pRaster2)) { continue; }
			
			unsigned int bi = curWused_samples + sample;
			
			if ( _compute_and_set_z_line(rz1, rz2, &bc, &bi, zBuffer) )  { continue; }

			_set_Coloro_fb_(frameBuffer,&bi ,sample_factor, color);
		}
	}

}

static void render_line(Renderer * _renderer, const Shape * shape){
	//VARS
	Renderer * renderer = _renderer;
	const Camera * cam = &renderer->camera;
	const Mat4 * ct = &cam->transformation;
	const Vertex ** vertices = (const Vertex **)shape->vertices;
	const Vertex * v1 = (const Vertex *)vertices[0]; 
	const Vertex * v2 = (const Vertex *)vertices[1];
	const ColorRGB * v1c = &v1->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2;

	Vec3 _v1v, _v2v;
	const Vec3 * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster_line(v1v, &pNDC1,  &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster_line(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);

	Vec3 limitvec;
	vec3_sub_dest(&limitvec, &pRaster2, &pRaster1);
	float limit = vec3_length(&limitvec);
	limit *= 0.5f;

	Vec2 start = { pRaster1.x, pRaster1.y };
	Vec2 end = { pRaster2.x, pRaster2.y };

	Vec2 tmp;
	vec2_sub_dest(&tmp, &end, &start);
	float raster_len = vec2_length(&tmp);
	float raster_len_inv = 1.f / raster_len;

	const Vec2 deltaVecs[5] = { {0.f , 0.f}, {-1.f , 0.f}, {1.f , 0.f}, {0.f , 1.f}, {0.f , -1.f} };
	renderer_3d_line_ctx_t ctx = { 
		&pRaster1, &pRaster2, _renderer, v1c, &rz1, &rz2, 5, &deltaVecs[0], 
		(const float*)&limit, (const float*)&raster_len, (const float*)&raster_len_inv		
	};
	geometry_line(&start, &end, _3d_line_to_framebuffer, &ctx);
}

#if 0
	/**
		returns false if pixel sample is inside triangle otherwise false.
	*/
#endif
static bool _compute_sample_bc_and_check(Vec3 * _pixelSample, const Vec2 ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 Barycentric * _bc,
										 const Vec3 * pRaster1, const Vec3 * pRaster2, const Vec3 * pRaster3) {
	Vec3 * pixelSample = _pixelSample;
	Barycentric * bc = _bc;
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
							   const Barycentric *bc, const unsigned int * bi,
							   float * zBuffer) {
	float z = (*rz1 * bc->bc0);
	z += (*rz2 * bc->bc1);
	z += (*rz3 * bc->bc2); 
	
	float *old_z = zBuffer + *bi;

	if ( z < *old_z ) { return true; }

	*old_z = z;			
	
	return false;
}

static void _compute_min_max_w_h(float *maxx, float *maxy, float *minx, float *miny,
								 unsigned int *curW, unsigned int *curH, const unsigned int *imgW, const unsigned int *imgH,
								 const Vec3 * pRaster1, const Vec3 * pRaster2, const Vec3 * pRaster3) {
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
static bool _world_to_raster(const Vec3 * _v, Vec3 * _ndc, Vec3 * _raster, float * _weight,
							 const float * imgW_h, const float * imgH_h, float * rz3, const Mat4 * _ct) {
	
	Vec3 * ndc = _ndc;
	const Vec3 * v = _v;
	Vec3 * raster = _raster;
	float * weight = _weight;
	const Mat4 * ct = _ct;
	
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

static void render_point(Renderer * renderer, const Shape * shape){
		//VARS
	const Camera * cam = &renderer->camera;
	const Mat4 * ct = &cam->transformation;
	const Vertex ** vertices = (const Vertex **)shape->vertices;
	const Vertex * v1 = (const Vertex *)vertices[0]; 
	const Vec3 * v1v = &v1->vec;
	const ColorRGB * v1c = &v1->color;
	const Vec2 * samples = renderer->samples;
	const Vec2 * cursample;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half, sample_factor = renderer->sample_factor;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, pRaster1, pixelSample;
	ColorRGB * frameBuffer = renderer->frameBuffer;
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
				
				_set_Coloro_fb_(frameBuffer,&bi ,&sample_factor,v1c);
			}
		}
	}
}

static bool __r_triangle_is_backface(Vec3 *pNDC1, Vec3 *pNDC2, Vec3 *pNDC3)
{
	Vec3 pNDC2_1;
	vec3_sub_dest(&pNDC2_1, pNDC2, pNDC1);
	Vec3 pNDC3_1;
	vec3_sub_dest(&pNDC3_1, pNDC3, pNDC1);
	

	Vec3 normal;
	vec3_cross_dest(&normal, &pNDC2_1, &pNDC3_1);
	
	if ( normal.z < 0.f ) return true;
	return false;
}

static void render_triangle_in_point_mode(Renderer *  renderer, const Shape *  shape) {
	const Camera *  cam = &renderer->camera;
	const Mat4 *  ct = &cam->transformation;
	const Vertex **  vertices = (const Vertex **)shape->vertices;
	const Vertex *  v1 = (const Vertex *)vertices[0]; 
	const Vertex *  v2 = (const Vertex *)vertices[1];
	const Vertex *  v3 = (const Vertex *)vertices[2];
	const Vec3 *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const ColorRGB *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const int bufWidth = renderer->bufWidth;
	const unsigned int used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3;
	ColorRGB *  frameBuffer = renderer->frameBuffer;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3, factor = 1.f;

	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	if (__r_triangle_is_backface(&pNDC1, &pNDC2, &pNDC3)) return;

	bool is1in = (pRaster1.x < renderer->imgWidth) && (pRaster1.x >= 0) && (pRaster1.y < renderer->imgHeight) && (pRaster1.y >= 0 );
	bool is2in = (pRaster2.x < renderer->imgWidth) && (pRaster2.x >= 0) && (pRaster2.y < renderer->imgHeight) && (pRaster2.y >= 0 );
	bool is3in = (pRaster3.x < renderer->imgWidth) && (pRaster3.x >= 0) && (pRaster3.y < renderer->imgHeight) && (pRaster3.y >= 0 );

	for (unsigned int sample = used_samples; sample--;) {
		
		
		if ( is1in )  { 
			unsigned int bi1 = (((unsigned int)pRaster1.y * bufWidth) + ((unsigned int)pRaster1.x * used_samples));
			unsigned int bi = bi1 + sample;
			_set_Coloro_fb_(frameBuffer,&bi ,&factor,v1c); 
		}
		
		if ( is2in)  {
			unsigned int bi2 = (((unsigned int)pRaster2.y * bufWidth) + ((unsigned int)pRaster2.x * used_samples));
			unsigned int bi = bi2 + sample;
			_set_Coloro_fb_(frameBuffer,&bi ,&factor,v2c); 
		}
		
		if ( is3in )  {
			unsigned int bi3 = (((unsigned int)pRaster3.y * bufWidth) + ((unsigned int)pRaster3.x * used_samples));
			unsigned int bi = bi3 + sample;
			_set_Coloro_fb_(frameBuffer,&bi ,&factor,v3c); 
		}
		
	}	
}

static void render_triangle_in_line_mode(Renderer *  renderer, const Shape *  shape) {
	const Camera *  cam = &renderer->camera;
	const Mat4 *  ct = &cam->transformation;
	const Vertex **  vertices = (const Vertex **)shape->vertices;
	const Vertex *  v1 = (const Vertex *)vertices[0]; 
	const Vertex *  v2 = (const Vertex *)vertices[1];
	const Vertex *  v3 = (const Vertex *)vertices[2];
	const Vec3 *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const ColorRGB *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3;
	float weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3;

	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	if (__r_triangle_is_backface(&pNDC1, &pNDC2, &pNDC3)) return;

	Vec2 start = { pRaster1.x, pRaster1.y };
	Vec2 end = { pRaster2.x, pRaster2.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v1c);

	start = (Vec2){ pRaster2.x, pRaster2.y };
	end = (Vec2){ pRaster3.x, pRaster3.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v2c);

	start = (Vec2){ pRaster3.x, pRaster3.y };
	end = (Vec2){ pRaster1.x, pRaster1.y };

	_draw_2D_line_to_renderer(renderer, &start, &end, v3c);
}

static void render_triangle(Renderer *  renderer, const Shape *  shape){
	//VARS
	const Camera *  cam = &renderer->camera;
	const Mat4 *  ct = &cam->transformation;
	const Vertex **  vertices = (const Vertex **)shape->vertices;
	const Vertex *  v1 = (const Vertex *)vertices[0]; 
	const Vertex *  v2 = (const Vertex *)vertices[1];
	const Vertex *  v3 = (const Vertex *)vertices[2];
	const Vec3 *  v1v = &v1->vec,* v2v = &v2->vec,* v3v = &v3->vec;
	const ColorRGB *  v1c = &v1->color, * v2c = &v2->color, * v3c = &v3->color;
	const Vec2 *  v1t = &v1->texCoord, * v2t = &v2->texCoord, * v3t = &v3->texCoord;
	const Vec2 *  samples = renderer->samples;
	const Vec2 *  cursample;
	const int texId = shape->texId;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half, sample_factor = renderer->sample_factor;
	Vec3 pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC3 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pRaster3, pixelSample;
	ColorRGB *  frameBuffer = renderer->frameBuffer;
	Barycentric bc;
	unsigned int curW, curH;
	float maxx, maxy, minx, miny, 
		  weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  weight3 = ct->_44, rz1, rz2, rz3;
	float *  zBuffer = renderer->zBuffer;

	ColorRGB curCol1;
	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	if (__r_triangle_is_backface(&pNDC1, &pNDC2, &pNDC3)) return;

	bc.area = 1.f/((pRaster3.x - pRaster1.x) * (pRaster2.y - pRaster1.y) - (pRaster3.y - pRaster1.y) * (pRaster2.x - pRaster1.x));

	_compute_min_max_w_h(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH,
						 &pRaster1, &pRaster2, &pRaster3);
	
	TextureCache * cache = renderer->texture_cache;
	Texture *texture = texture_cache_get(cache, (unsigned int)shape->texId); 

	for(; curH < maxy; ++curH) {
		unsigned int curHbufWidth = curH * bufWidth;
		for(curW = minx; curW < maxx; ++curW) {
			cursample = samples;
			unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
			for (unsigned int sample = used_samples; sample--;) {

				if ( _compute_sample_bc_and_check(&pixelSample, &cursample,&curW, &curH, &bc,
										 &pRaster1, &pRaster2, &pRaster3)) { continue; }
				
				unsigned int bi = curWused_samples + sample;
				
				if ( _compute_and_set_z(&rz1, &rz2, &rz3, &bc, &bi, zBuffer) )  { continue; }
				
				if ( _compute_px_color(&curCol1, &bc, &weight1, &weight2, &weight3,
								texture, v1c, v2c, v3c, v1t, v2t, v3t, &texId))
				{
					_set_Coloro_fb_(frameBuffer,&bi ,&sample_factor,&curCol1);
				}
			}
		}
	}
	
}

void render_shape(Renderer *  renderer, const Shape *  shape){
	const Shape *  curshape = shape;
	const Renderer * curRenderer = renderer;
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

/*

	p = (xmin,ymin,zmin)
	if (normal.x >= 0)
		p.x = xmax;
	if (normal.y >=0))
		p.y = ymax;
	if (normal.z >= 0)
		p.z = zmax:

The negative vertex n follows the opposite rule:


	n = (xmax,ymax,zmax)
	if (normal.x >= 0)
		n.x = xmin;
	if (normal.y >=0))
		n.y = ymin;
	if (normal.z >= 0)
		n.z = zmin:


*/

static void __r_update_pVertex(Vec3* _pVertex, Vec3 *_normal, BBox *_bbox)
{
	Vec3* pVertex = _pVertex;
	Vec3 *normal = _normal;
	BBox *bbox = _bbox;

	pVertex->x = ( normal->x >= 0.f ? bbox->max.x : bbox->min.x );
	pVertex->x = ( normal->y >= 0.f ? bbox->max.y : bbox->min.y );
	pVertex->x = ( normal->z >= 0.f ? bbox->max.z : bbox->min.z );
}

static void __r_update_nVertex(Vec3* _nVertex, Vec3 *_normal, BBox *_bbox)
{
	Vec3* nVertex = _nVertex;
	Vec3 *normal = _normal;
	BBox *bbox = _bbox;

	nVertex->x = ( normal->x >= 0.f ? bbox->min.x : bbox->max.x );
	nVertex->x = ( normal->y >= 0.f ? bbox->min.y : bbox->max.y );
	nVertex->x = ( normal->z >= 0.f ? bbox->min.z : bbox->max.z );
}

static int __r_check_plate_frustum(Plane* _plane, BBox *_bbox)
{
	BBox *bbox = _bbox;
	Plane* plane = _plane;
	
	uint32_t cntOutside = 0;
	
	Vec3 pVertex, nVertex;
	__r_update_nVertex(&nVertex, &plane->normal, bbox);
	__r_update_pVertex(&pVertex, &plane->normal, bbox);
	
	cntOutside += (mu_point_plane_distance_normal(&nVertex, &plane->lb, &plane->normal) < 0.f ? 1 : 0);
	cntOutside += (mu_point_plane_distance_normal(&pVertex, &plane->lb, &plane->normal) < 0.f ? 1 : 0);
	
	return cntOutside;
}

static void __r_frustum_as_vec3_array(Vec3 **_array, Plane* _nearPlane, Plane* _farPlane)
{
	Vec3 **array = _array;
	Plane* nearPlane = _nearPlane;
	Plane* farPlane = _farPlane;

	array[0] = &nearPlane->lb;
	array[1] = &nearPlane->rb;
	array[2] = &nearPlane->lt;
	array[3] = &nearPlane->rt;
	array[4] = &farPlane->lb;
	array[5] = &farPlane->rb;
	array[6] = &farPlane->lt;
	array[7] = &farPlane->rt;
}

static bool __r_bbox_in_frustum(Frustum *_frustum, BBox *_bbox)
{
	Frustum *frustum = _frustum;
	BBox *bbox = _bbox;

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
	Vec3*vecs[8];
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

void render_mesh(Renderer * renderer, const Mesh *  mesh){
	BBox * bbox = (BBox*)&mesh->bbox;
	if ( bbox->created  )
	{
		if ( !__r_bbox_in_frustum(&renderer->camera.frustum, bbox) ) return;
	}
	Shape ** shapes = mesh->shapes;
	for(unsigned int cntShape = mesh->cntShapes; cntShape-- ;) {		
		render_shape(renderer, *shapes);
		++shapes;
	} 
}

void 
render_scene(Renderer *  renderer, const Scene *  scene){	
	Mesh ** meshes = scene->meshes;
	for(int cntMesh = scene->cntMesh; cntMesh--; ) {
		render_mesh(renderer, *meshes);
		++meshes;
	}
}

void renderer_clear_frame(Renderer * renderer){
	const int buffersize = renderer->imgWidth * 
						   renderer->imgHeight * 
						   renderer->samplestep * renderer->samplestep;
	memset(renderer->zBuffer, 0.f, buffersize * sizeof(float));
	memset(renderer->frameBuffer, 0, buffersize * sizeof(ColorRGB));
	renderer->max_z = RENDER_FLT_MAX;
	renderer->min_z = 0.f;
}


void renderer_set_vmode_solid(Renderer * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle;
}

void renderer_set_vmode_point(Renderer * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line_in_point_mode;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle_in_point_mode;
}

void renderer_set_vmode_line(Renderer * renderer) {
	renderer->POINT_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_point;
	renderer->LINE_RENDER_FUNC		= (RENDERER_RENDER_FUNC)render_line_in_line_mode;
	renderer->TRIANGLE_RENDER_FUNC 	= (RENDERER_RENDER_FUNC)render_triangle_in_line_mode;
}

Renderer * 
renderer_new(int imgWidth, int imgHeight, ColorRGB * bgColor, unsigned int samplestep){
	Renderer * newrenderer = malloc(sizeof(Renderer));
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
	newrenderer->frameBuffer = malloc(buffersize * sizeof(ColorRGB));
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

	newrenderer->samples = malloc(newrenderer->used_samples * sizeof(Vec2));

	for( unsigned int sy = 0; sy < samplestep ;++sy ){
		for( unsigned int sx = 0; sx < samplestep ;++sx ){
			Vec2  *cursample = &newrenderer->samples[sy * samplestep + sx];
			cursample->x = stepstart + (sx * step);
			cursample->y = stepstart + (sy * step);
		}
	}
	
	renderer_set_vmode_solid(newrenderer);
	newrenderer->texture_cache = texture_cache_new();

	return newrenderer;
}

void 
renderer_free(Renderer * renderer){
	free(renderer->frameBuffer);
	free(renderer->zBuffer);
	free(renderer->samples);
	texture_cache_free(&renderer->texture_cache);
	free(renderer);
}


void 
renderer_output_ppm(Renderer * renderer, const char * filename){
	unsigned int colcnt=0, bi=0, samplestart;
	int i, j, imgW = renderer->imgWidth, imgH = renderer->imgHeight;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", imgW, imgH);
	ColorRGB fc;
	unsigned char color[3*imgW*imgH];
    for (j = 0; j < imgH; ++j){
	  bi = j * renderer->bufWidth;
	  for (i = 0; i < imgW; ++i){
		fc.r = 0.f, fc.g = 0.f, fc.b = 0.f;
		samplestart = bi + (i*renderer->used_samples);
		for (unsigned int sample = renderer->used_samples; sample--;){
			ColorRGB * c = &renderer->frameBuffer[samplestart + sample];
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
renderer_output_z_buffer_ppm(Renderer * renderer, const char * filename){
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
