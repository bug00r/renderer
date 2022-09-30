#include "rasterizer.h"

/**

    PRIVATE INTERFACE

*/

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
	
	float weight1 = ct->_44;
    float *rz1 = &state->rZ[0];
	
	if (_world_to_raster(v1v, ndc, pRaster1, &weight1, &imgW_h, &imgH_h, rz1, ct)) return;

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

void raster_precalc(raster_ctx_t *_ctx, const raster_obj_t * _obj, raster_state_t *_state)
{
    raster_ctx_t *ctx = _ctx;
    const raster_obj_t *obj = _obj;
    raster_state_t *state = _state;
}

void raster_set_vmode_solid(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	//ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line;
	//ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle;
}

void raster_set_vmode_point(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	//ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line_in_point_mode;
	//ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle_in_point_mode;
}

void raster_set_vmode_line(raster_ctx_t * _ctx) {
    raster_ctx_t *ctx = _ctx;
	ctx->POINT_RASTER_FUNC		= (RASTER_FUNC)raster_point;
	//ctx->LINE_RASTER_FUNC		= (RASTER_FUNC)raster_line_in_line_mode;
	//ctx->TRIANGLE_RASTER_FUNC 	= (RASTER_FUNC)raster_triangle_in_line_mode;
}
