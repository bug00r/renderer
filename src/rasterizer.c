#include "rasterizer.h"

/**

    PRIVATE INTERFACE

*/

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

	if ( z < *old_z ) { return true; }

	*old_z = z;			
	
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

/** #### POINT RASTERIZATION ####  */

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

static bool _compute_and_set_z_point(const float * rz1, const unsigned int * bi, float * zBuffer) {
		
	float * old_z = zBuffer + *bi;
             
	if ( *rz1 < *old_z ) { return true; }
	
	*old_z = *rz1;	
	
	return false;
}

static void raster_point(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state){
    
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t *ct = ctx->ct;
	const vec3_t * v1v = &obj->vec[0];
	const cRGB_t * v1c = &obj->color[0];

	const vec2_t * samples = ctx->samples;
	
	const int bufWidth = ctx->bufWidth;
	const unsigned int  imgW = ctx->imgWidth, 
                        imgH = ctx->imgHeight, 
                        used_samples = ctx->used_samples;

	const float imgW_h = ctx->imgWidth_half, 
                imgH_h = ctx->imgHeight_half, 
                sample_factor = ctx->sample_factor;

    vec3_t *ndc = &state->ndc[0], 
           *pRaster1 = &state->raster[0];

    ndc->x = ct->_14; ndc->y = ct->_24; ndc->z = ct->_34;
	
	float *weight1 = &state->weight[0]; *weight1 = ct->_44;
    float *rz1 = &state->rZ[0];
	
	if (_world_to_raster(v1v, ndc, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct)) return;

	float maxx = 1.f, maxy = 1.f, minx = 0.f, miny = 0.f;

    unsigned int curW = miny, 
                 curH = minx;
	//EO BOUNDING BOX
	for(; curH < maxy; ++curH) {
		for(curW = minx; curW < maxx; ++curW) {
			const vec2_t *cursample = samples;
			for (unsigned int sample = used_samples; sample--;) {
				
                vec3_t pixelSample;
				if ( _compute_sample_and_check_point(
							&pixelSample, &cursample,&curW, &curH, &imgW, &imgH, pRaster1)) { continue; }
				
				unsigned int bi = curH * bufWidth + (curW * used_samples) + sample;		
				
				if ( _compute_and_set_z_point(rz1, &bi, ctx->zBuffer) )  { continue; }
				
				_set_color_to_fb_(ctx->frameBuffer,&bi ,&sample_factor,v1c);
			}
		}
	}
}

/** #### LINE RASTERIZATION ####  */

static bool _compute_sample_bc_and_check_line(const float *_limit, const float *_raster_len, const float *_raster_len_inv, vec3_t * _pixelSample, const vec2_t ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 barycentric_t * _bc, const vec3_t * pRaster1, const vec3_t * pRaster2) { 
	vec3_t * pixelSample = _pixelSample;
	barycentric_t * bc = _bc;
	update_sample(pixelSample, _cursample, curW, curH);

	bc->bc0 = (pixelSample->x - pRaster1->x ) * (pRaster2->y - pRaster1->y) - (pixelSample->y - pRaster1->y) * (pRaster2->x - pRaster1->x);

	float edge = bc->bc0;

	const float limit = (const float)*_limit;
	const float raster_len_inv = (const float)*_raster_len_inv;
	const float raster_len = (const float)*_raster_len;

	if ( ( edge <= limit && edge >= -limit ) ) {

		vec2_t tmp;
		vec2_sub_dest(&tmp, (vec2_t*)pixelSample, (vec2_t*)pRaster1);
		float len2 = vec2_length(&tmp);

		bc->bc0 = len2 * raster_len_inv;
		bc->bc1 = (raster_len - len2) * raster_len_inv;
			
		return false;
	} 
	return true;								 
}


static bool _compute_and_set_z_line(const float * rz1, const float * rz2, const barycentric_t *bc, 
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

static void raster_line_in_point_mode(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t * ct = ctx->ct;

	const cRGB_t *v1c = &obj->color[0];
	const int bufWidth = ctx->bufWidth;
	const unsigned int imgW = ctx->imgWidth, imgH = ctx->imgHeight, used_samples = ctx->used_samples;
	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half;

	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34;

	float *weight1 = &state->weight[0], 
		  *weight2 = &state->weight[1], 
		  *rz1 = &state->rZ[0], 
		  *rz2 = &state->rZ[1];

	*weight1 = ct->_44; *weight2 = ct->_44;

	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1];

	_world_to_raster_line(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct);
	_world_to_raster_line(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct);

	vec3_t normR1 = { fmaxf(0.f, fminf(pRaster1->x, imgW)), fmaxf(0.f, fminf(pRaster1->y, imgH)), pRaster1->z };
	vec3_t normR2 = { fmaxf(0.f, fminf(pRaster2->x, imgW)), fmaxf(0.f, fminf(pRaster2->y, imgH)), pRaster2->z };

	unsigned int bi1 = (normR1.y * bufWidth + (normR1.x * used_samples));
	unsigned int bi2 = (normR2.y * bufWidth + (normR2.x * used_samples));

	float factor = 1.f;
	cRGB_t * frameBuffer = ctx->frameBuffer;
	for (unsigned int sample = used_samples; sample--;) {
		
		unsigned int bi = bi1 + sample;

		_set_color_to_fb_(frameBuffer,&bi ,&factor, v1c);

		bi = bi2 + sample;

		_set_color_to_fb_(frameBuffer,&bi ,&factor, v1c);

	}	
}

typedef struct {
	vec2_t *start; 
	vec2_t* end;
	raster_ctx_t * ctx;
	float factor;
	const cRGB_t * color;
} renderer_2d_line_ctx_t;


static void _2d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) {
	renderer_2d_line_ctx_t* ctx = (renderer_2d_line_ctx_t*)data;
	raster_ctx_t * rasterCtx = ctx->ctx;

	if ( *x >= rasterCtx->imgWidth || *x < 0 || 
		*y >= rasterCtx->imgHeight || *y < 0 ) return;

	for (unsigned int sample = rasterCtx->used_samples; sample--;) {
		
		 unsigned int bi = ((unsigned int)*y * rasterCtx->bufWidth ) + ( *x * rasterCtx->used_samples ) + sample;
		_set_color_to_fb_(rasterCtx->frameBuffer, &bi , &ctx->factor, ctx->color);

	}

}

static void _draw_2D_line_to_renderer(raster_ctx_t * rasterCtx, vec2_t *start, vec2_t* end, const cRGB_t * color) {
	renderer_2d_line_ctx_t ctx = { start, end, rasterCtx, 1.f/rasterCtx->used_samples, color };
	geometry_line(start, end, _2d_line_to_framebuffer, &ctx);
}

static void raster_line_in_line_mode(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t * ct = ctx->ct;

	const cRGB_t * v1c = &obj->color[0];
	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half;

	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34;

	float *weight1 = &state->weight[0], 
		  *weight2 = &state->weight[1], 
		  *rz1 = &state->rZ[0], 
		  *rz2 = &state->rZ[1];

	*weight1 = ct->_44; *weight2 = ct->_44;
	
	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1];

	_world_to_raster_line(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct);
	_world_to_raster_line(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct);

	vec2_t start = { pRaster1->x, pRaster1->y };
	vec2_t end = { pRaster2->x, pRaster2->y };

	_draw_2D_line_to_renderer(ctx, &start, &end, v1c);
}


typedef struct {
	vec3_t *pRaster1; 
	vec3_t *pRaster2;
	raster_ctx_t * ctx;
	const cRGB_t * color;
	float *rz1;
	float *rz2;
	unsigned int cntDeltaVecs;
	const vec2_t *deltaVecs;
	const float *limit;
	const float *raster_len;
	const float *raster_len_inv;
} renderer_3d_line_ctx_t;

static void _3d_line_to_framebuffer(int32_t const * const x, int32_t const * const y, void *data) 
{
	renderer_3d_line_ctx_t* ctx = (renderer_3d_line_ctx_t*)data;
	raster_ctx_t *rasterCtx = ctx->ctx;

	const unsigned int limitW = (unsigned int)(rasterCtx->imgWidth - 1);
	const unsigned int limitH = (unsigned int)(rasterCtx->imgHeight - 1);

	const unsigned int _x = (const unsigned int)*x;
	const unsigned int _y = (const unsigned int)*y;

	if ( _x > limitW || _y > limitH ) return;

	const vec2_t * samples = rasterCtx->samples;
	const vec2_t * cursample;
	const unsigned int used_samples = rasterCtx->used_samples;
	const float *sample_factor = &rasterCtx->sample_factor;
	const int bufWidth = rasterCtx->bufWidth;

	vec3_t pixelSample;
	vec3_t *pRaster1 = ctx->pRaster1;
	vec3_t *pRaster2 = ctx->pRaster2;
	barycentric_t bc;
	float *rz1 = ctx->rz1;
	float *rz2 = ctx->rz2;

	float * zBuffer = rasterCtx->zBuffer;
	cRGB_t * frameBuffer = rasterCtx->frameBuffer;

	const cRGB_t *color = ctx->color; 

	const float *limit = ctx->limit;
	const float *raster_len = ctx->raster_len;
	const float *raster_len_inv = ctx->raster_len_inv;

	for ( uint32_t curDeltaVec = 0; curDeltaVec < ctx->cntDeltaVecs; curDeltaVec++ )
	{
		const vec2_t *deltaVec = &ctx->deltaVecs[curDeltaVec];
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

			_set_color_to_fb_(frameBuffer,&bi ,sample_factor, color);
		}
	}

}

static void raster_line(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;
	
	const mat4_t * ct = ctx->ct;

	const cRGB_t * v1c = &obj->color[0];

	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34;

	float *weight1 = &state->weight[0], 
		  *weight2 = &state->weight[1], 
		  *rz1 = &state->rZ[0], 
		  *rz2 = &state->rZ[1];

	*weight1 = ct->_44; *weight2 = ct->_44;
	
	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1];

	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half;

	_world_to_raster_line(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct);
	_world_to_raster_line(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct);

	vec3_t limitvec;
	vec3_sub_dest(&limitvec, pRaster2, pRaster1);
	float limit = vec3_length(&limitvec);
	limit *= 0.5f;

	vec2_t start = { pRaster1->x, pRaster1->y };
	vec2_t end = { pRaster2->x, pRaster2->y };

	vec2_t tmp;
	vec2_sub_dest(&tmp, &end, &start);
	float raster_len = vec2_length(&tmp);
	float raster_len_inv = 1.f / raster_len;

	const vec2_t deltaVecs[5] = { {0.f , 0.f}, {-1.f , 0.f}, {1.f , 0.f}, {0.f , 1.f}, {0.f , -1.f} };
	renderer_3d_line_ctx_t ctx3DLine = { 
		pRaster1, pRaster2, ctx, v1c, rz1, rz2, 5, &deltaVecs[0], 
		(const float*)&limit, (const float*)&raster_len, (const float*)&raster_len_inv		
	};
	geometry_line(&start, &end, _3d_line_to_framebuffer, &ctx3DLine);
}

/** #### TRIANGLE RASTERIZATION ####  */

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


static bool __r_triangle_is_backface(vec3_t *pNDC1, vec3_t *pNDC2, vec3_t *pNDC3)
{
	vec3_t pNDC2_1;
	vec3_sub_dest(&pNDC2_1, pNDC2, pNDC1);
	vec3_t pNDC3_1;
	vec3_sub_dest(&pNDC3_1, pNDC3, pNDC1);
	

	vec3_t normal;
	vec3_cross_dest(&normal, &pNDC2_1, &pNDC3_1);
	
	if ( normal.z < 0.f ) return true;
	return false;
}

static void raster_triangle_in_point_mode(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t *  ct = ctx->ct;

	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1], *v3v = &obj->vec[2];
	const cRGB_t *v1c = &obj->color[0], *v2c = &obj->color[1], *v3c = &obj->color[2];

	const int bufWidth = ctx->bufWidth;
	const unsigned int used_samples = ctx->used_samples;
	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half;

	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1], *pRaster3 = &state->raster[2],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1], *pNDC3 = &state->ndc[2];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14; pNDC3->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24; pNDC3->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34; pNDC3->z = ct->_34;

	float *weight1 = &state->weight[0], *weight2 = &state->weight[1], *weight3 = &state->weight[2],
		  *rz1 = &state->rZ[0], *rz2 = &state->rZ[1], *rz3 = &state->rZ[2];

	*weight1 = ct->_44; *weight2 = ct->_44; *weight3 = ct->_44;

	if (_world_to_raster(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct)) return; 
	if (_world_to_raster(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct)) return; 
	if (_world_to_raster(v3v, pNDC3, pRaster3, weight3, &imgW_h, &imgH_h, rz3, ct)) return; 

	// TODO backface Culling outsourcing to 3D Engine
	if (__r_triangle_is_backface(pNDC1, pNDC2, pNDC3)) return;

	bool is1in = (pRaster1->x < ctx->imgWidth) && (pRaster1->x >= 0) && (pRaster1->y < ctx->imgHeight) && (pRaster1->y >= 0 );
	bool is2in = (pRaster2->x < ctx->imgWidth) && (pRaster2->x >= 0) && (pRaster2->y < ctx->imgHeight) && (pRaster2->y >= 0 );
	bool is3in = (pRaster3->x < ctx->imgWidth) && (pRaster3->x >= 0) && (pRaster3->y < ctx->imgHeight) && (pRaster3->y >= 0 );

	float factor = 1.f;
	cRGB_t *frameBuffer = ctx->frameBuffer;
	for (unsigned int sample = used_samples; sample--;) {
		
		
		if ( is1in )  { 
			unsigned int bi1 = (((unsigned int)pRaster1->y * bufWidth) + ((unsigned int)pRaster1->x * used_samples));
			unsigned int bi = bi1 + sample;
			_set_color_to_fb_(frameBuffer, &bi, &factor, v1c); 
		}
		
		if ( is2in)  {
			unsigned int bi2 = (((unsigned int)pRaster2->y * bufWidth) + ((unsigned int)pRaster2->x * used_samples));
			unsigned int bi = bi2 + sample;
			_set_color_to_fb_(frameBuffer, &bi, &factor, v2c); 
		}
		
		if ( is3in )  {
			unsigned int bi3 = (((unsigned int)pRaster3->y * bufWidth) + ((unsigned int)pRaster3->x * used_samples));
			unsigned int bi = bi3 + sample;
			_set_color_to_fb_(frameBuffer, &bi, &factor, v3c); 
		}
		
	}	
}

static void raster_triangle_in_line_mode(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t *  ct = ctx->ct;

	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1], *v3v = &obj->vec[2];
	const cRGB_t *v1c = &obj->color[0], *v2c = &obj->color[1], *v3c = &obj->color[2];
	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half;

	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1], *pRaster3 = &state->raster[2],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1], *pNDC3 = &state->ndc[2];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14; pNDC3->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24; pNDC3->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34; pNDC3->z = ct->_34;

	float *weight1 = &state->weight[0], *weight2 = &state->weight[1], *weight3 = &state->weight[2],
		  *rz1 = &state->rZ[0], *rz2 = &state->rZ[1], *rz3 = &state->rZ[2];

	*weight1 = ct->_44; *weight2 = ct->_44; *weight3 = ct->_44;

	if (_world_to_raster(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct)) return; 
	if (_world_to_raster(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct)) return; 
	if (_world_to_raster(v3v, pNDC3, pRaster3, weight3, &imgW_h, &imgH_h, rz3, ct)) return; 

	//TODO Backface Culling should be OUT to 3D Engine
	if (__r_triangle_is_backface(pNDC1, pNDC2, pNDC3)) return;

	vec2_t start = { pRaster1->x, pRaster1->y };
	vec2_t end = { pRaster2->x, pRaster2->y };

	_draw_2D_line_to_renderer(ctx, &start, &end, v1c);

	start = (vec2_t){ pRaster2->x, pRaster2->y };
	end = (vec2_t){ pRaster3->x, pRaster3->y };

	_draw_2D_line_to_renderer(ctx, &start, &end, v2c);

	start = (vec2_t){ pRaster3->x, pRaster3->y };
	end = (vec2_t){ pRaster1->x, pRaster1->y };

	_draw_2D_line_to_renderer(ctx, &start, &end, v3c);
}


static bool _compute_px_color(cRGB_t * color, 
							  const barycentric_t *bc, 
							  const float * weight1, const float * weight2, const float * weight3,
							  const texture_t * texture,
							  const cRGB_t * v1c, const cRGB_t * v2c, const cRGB_t * v3c,
							  const vec2_t * v1t, const vec2_t * v2t, const vec2_t * v3t) {

	const float z0 = bc->bc0*(*weight1);
	const float z1 = bc->bc1*(*weight2);
	const float z2 = bc->bc2*(*weight3);
	const float z3 = 1.f/(z0 + z1 + z2);
	
	if (texture == NULL)
	{
		color->r = (z0*v1c->r + z1*v2c->r + z2*v3c->r ) * z3;
		color->g = (z0*v1c->g + z1*v2c->g + z2*v3c->g ) * z3;
		color->b = (z0*v1c->b + z1*v2c->b + z2*v3c->b ) * z3;
	}
	else
	{
		float texx = ( z0*v1t->x + z1*v2t->x + z2*v3t->x ) * z3 * texture->width;
		float texy = ( z0*v1t->y + z1*v2t->y + z2*v3t->y ) * z3 * texture->height;

		crgb_array2D_get(texture->buffer, (int)texx, (int)texy, color);

		//TODO Transparency color: should be outsourced as opt
		if ( color->r == 1.f && color->g == 0.f && color->b == 1.f  )
		{
			return false;
		}
	}

	return true;
}

static void raster_triangle(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	const mat4_t *ct = ctx->ct;

	const vec3_t *v1v = &obj->vec[0], *v2v = &obj->vec[1], *v3v = &obj->vec[2];
	const cRGB_t *v1c = &obj->color[0], *v2c = &obj->color[1], *v3c = &obj->color[2];
	const vec2_t *v1t = &obj->texCoord[0], *v2t = &obj->texCoord[1], *v3t = &obj->texCoord[2];
	const vec2_t *samples = ctx->samples;
	
	const int bufWidth = ctx->bufWidth;
	const unsigned int imgW = ctx->imgWidth, imgH = ctx->imgHeight, used_samples = ctx->used_samples;
	const float imgW_h = ctx->imgWidth_half, imgH_h = ctx->imgHeight_half, sample_factor = ctx->sample_factor;
	
	vec3_t *pRaster1 = &state->raster[0], *pRaster2 = &state->raster[1], *pRaster3 = &state->raster[2],
		   *pNDC1 = &state->ndc[0], *pNDC2 = &state->ndc[1], *pNDC3 = &state->ndc[2];
	pNDC1->x = ct->_14; pNDC2->x = ct->_14; pNDC3->x = ct->_14;
	pNDC1->y = ct->_24; pNDC2->y = ct->_24; pNDC3->y = ct->_24;
	pNDC1->z = ct->_34; pNDC2->z = ct->_34; pNDC3->z = ct->_34;

	float *weight1 = &state->weight[0], *weight2 = &state->weight[1], *weight3 = &state->weight[2],
		  *rz1 = &state->rZ[0], *rz2 = &state->rZ[1], *rz3 = &state->rZ[2];

	*weight1 = ct->_44; *weight2 = ct->_44; *weight3 = ct->_44;

	if (_world_to_raster(v1v, pNDC1, pRaster1, weight1, &imgW_h, &imgH_h, rz1, ct)) return; 
	if (_world_to_raster(v2v, pNDC2, pRaster2, weight2, &imgW_h, &imgH_h, rz2, ct)) return; 
	if (_world_to_raster(v3v, pNDC3, pRaster3, weight3, &imgW_h, &imgH_h, rz3, ct)) return; 

	//TODO backface culling moving out to 3D ENGINE
	if (__r_triangle_is_backface(pNDC1, pNDC2, pNDC3)) return;

	barycentric_t bc;
	bc.area = 1.f/((pRaster3->x - pRaster1->x) * (pRaster2->y - pRaster1->y) - 
				   (pRaster3->y - pRaster1->y) * (pRaster2->x - pRaster1->x));

	unsigned int curW, curH;
	float maxx, maxy, minx, miny;
	_compute_min_max_w_h(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH,
						 pRaster1, pRaster2, pRaster3);
	
	vec3_t pixelSample;
	for(; curH < maxy; ++curH) {
		unsigned int curHbufWidth = curH * bufWidth;
		for(curW = minx; curW < maxx; ++curW) {
			const vec2_t *cursample = samples;
			unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
			for (unsigned int sample = used_samples; sample--;) {

				if ( _compute_sample_bc_and_check(&pixelSample, &cursample, &curW, &curH, &bc,
										 pRaster1, pRaster2, pRaster3)) { continue; }
				
				unsigned int bi = curWused_samples + sample;
				
				float *zBuffer = ctx->zBuffer;
				if ( _compute_and_set_z(rz1, rz2, rz3, &bc, &bi, zBuffer) )  { continue; }
				
				cRGB_t curCol1;
				if ( _compute_px_color(&curCol1, &bc, weight1, weight2, weight3,
								obj->texture, v1c, v2c, v3c, v1t, v2t, v3t))
				{
					cRGB_t *  frameBuffer = ctx->frameBuffer;
					_set_color_to_fb_(frameBuffer,&bi ,&sample_factor, &curCol1);
				}
			}
		}
	}
	
}


/**

    PUBLIC INTERFACE

*/

void raster(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

    switch(obj->cntVertex){
		case 3:
			ctx->TRIANGLE_RASTER_FUNC(ctx, obj, state); break;
		case 2:
			ctx->LINE_RASTER_FUNC(ctx, obj, state); break;
		case 1:
			ctx->POINT_RASTER_FUNC(ctx, obj, state); break;
		default:
			printf("Unknown vertex count oO\n");
	}
}

void raster_precalc_weight(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;


}

void raster_precalc_ndc(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;
}

void raster_precalc_raster(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;
}

void raster_precalc(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;

	//TODO
}

void raster_set_vmode_solid(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line;
	ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle;
}

void raster_set_vmode_point(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line_in_point_mode;
	ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle_in_point_mode;
}

void raster_set_vmode_line(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line_in_line_mode;
	ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle_in_line_mode;
}
