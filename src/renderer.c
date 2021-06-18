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

static void _compute_px_color(cRGB_t * color, 
							  const barycentric_t *bc, 
							  const float * weight1, const float * weight2, const float * weight3,
							  const cRGB_t * texture, const unsigned int * imgW,
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
			int texx = (int)(( z0*v1t->x + z1*v2t->x + z2*v3t->x ) * z3 * 512.f);
			int texy = (int)(( z0*v1t->y + z1*v2t->y + z2*v3t->y ) * z3 * 512.f);
			
			const cRGB_t * txc = &texture[texy * (*imgW) + texx];
			color->r = txc->r;
			color->g = txc->g;
			color->b = txc->b;
			}						
			break;
	}
}

#if 0
	/**
		returns false if pixel should skipped otherwise false.
	*/
#endif
static bool _compute_and_set_z(const float * rz1, const float * rz2, const float * rz3,
							   const barycentric_t *bc, const unsigned int * bi,
							   float * zBuffer) {
	float z = (*rz1 * bc->bc0);
	z += (*rz2 * bc->bc1);
	z += (*rz3 * bc->bc2); 
	
	float * old_z = zBuffer + *bi;

	if ( z < *old_z ) { return true; }
	
	*old_z = z;			
	
	//only for z buffer print 
	//renderer->min_z = fminf(renderer->min_z, z);
	//renderer->max_z = fmaxf(renderer->max_z, z);
	return false;
}

static bool _compute_and_set_z_line(const float * rz1, const float * rz2, const barycentric_t *bc, 
								    const unsigned int * bi, float * zBuffer) {
	float z =  *rz1 * ( 1.f - bc->bc1 ); 
	z += *rz2 * bc->bc1; 
	
	float * old_z = zBuffer + *bi;

	if ( z > *old_z ) { return true; }	
	
	*old_z = z;
	
	//only for z buffer print 
	//renderer->min_z = fminf(renderer->min_z, z);
	//renderer->max_z = fmaxf(renderer->max_z, z);
	
	return false;
}

static bool _compute_and_set_z_point(const float * rz1, const unsigned int * bi, float * zBuffer) {
		
	float * old_z = zBuffer + *bi;
             
	if ( *rz1 > *old_z ) { return true; }
	
	*old_z = *rz1;	
	
	//only for z buffer print 
	//renderer->min_z = fminf(renderer->min_z, *rz1);
	//renderer->max_z = fmaxf(renderer->max_z, *rz1);
	
	return false;
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

static bool _compute_sample_bc_and_check_line(vec3_t * _pixelSample, const vec2_t ** _cursample,
										 const unsigned int *curW, const unsigned int *curH,
										 barycentric_t * _bc, const vec3_t * pRaster1, const vec3_t * pRaster2) { 
	vec3_t * pixelSample = _pixelSample;
	barycentric_t * bc = _bc;
	update_sample(pixelSample, _cursample, curW, curH);

	//bc->inside = false;
	bc->bc1 = (pixelSample->x - pRaster1->x ) / (pRaster2->x - pRaster1->x);

	float edge = place_of_vec3(pRaster1, pRaster2, pixelSample);
	
	vec3_t limitvec;
	vec3_sub_dest(&limitvec, pRaster2, pRaster1);
	float limit = vec3_length(&limitvec) * 0.5f;
	if ( ( edge <= limit && edge >= -limit ) ) {		 
		return false;
	}	

	return true;								 
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

static void _compute_min_max_w_h_line(float *maxx, float *maxy, float *minx, float *miny,
								 unsigned int *curW, unsigned int *curH, const unsigned int *imgW, const unsigned int *imgH,
								 const vec3_t * pRaster1, const vec3_t * pRaster2 ) {
	*maxx = fminf((float)*imgW, fmaxf(pRaster1->x, pRaster2->x));
	*maxy = fminf((float)*imgH, fmaxf(pRaster1->y, pRaster2->y));
	*minx = fmaxf(0.f, fminf(pRaster1->x, pRaster2->x));
	*miny = fmaxf(0.f, fminf(pRaster1->y, pRaster2->y));
	
	if ( *minx == *maxx ) {
		*minx -= 2.f; *maxx+=2.f;
	}
	
	if ( *miny == *maxy ) {
		*miny -= 2.f; *maxy+=2.f;
	}
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
	const vec2_t * v1t = &v1->texCoord;
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

static float place_of_vec3_z(const vec3_t *  s, const vec3_t *  e, const vec3_t *  p) {
	return (p->z - s->z) * (e->x - s->x) - (p->x - s->x) * (e->z - s->z);
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
	bool start_out = (mu_point_plane_distance_normal(line_start, &near->lb, normal) < 0);
	bool end_out = (mu_point_plane_distance_normal(line_end, &near->lb, normal) < 0);

	filtered = (start_out && end_out);

	printf("start out: %i  end out: %i\n", start_out, end_out);
	printf("line: ");
	vec3_print(line_start);
	vec3_print(line_end);
	print_camera(&renderer->camera);

	if (filtered)  {	
		return false;
	} else if ( !start_out && !end_out ) {
		return true;
	}

	vec3_copy_dest(_dest_start, line_start);
	vec3_copy_dest(_dest_end, line_end);

	vec3_t intersect;
	if (mu_line_plane_intersection_normal(&intersect, line_start, line_end, &near->lb, normal)) {
		vec3_t *to_clip = ( start_out ? _dest_start : _dest_end);
		vec3_copy_dest(to_clip, &intersect);
		printf("line: ");
		vec3_print(line_start);
		vec3_print(line_end);
		printf("line clip: ");
		vec3_print(to_clip);
		print_camera(&renderer->camera);
	}


	//bool mu_line_plane_intersection_normal(vec3_t *intersect, vec3_t *_line_p1, vec3_t *_line_p2, vec3_t * _point, vec3_t *_normal) 

	//clipping NEW

		//bool mu_line_plane_intersection(vec3_t *_intersect, vec3_t * _line_p1, vec3_t * _line_p2, vec3_t * _plane_p1, vec3_t * _plane_p2, vec3_t * _plane_p3)
	/*vec3_t inter_pt;
	vec3_t line_p1 = {0.5f, 0.5f, 0.5f};
	vec3_t line_p2 = {0.5f, -0.5f, 0.5f};
	bool intersects = mu_line_plane_intersection(&inter_pt, &line_p1, &line_p2, &plane_p1, &plane_p2, &plane_p3);

	assert(intersects == true);
	vec3_t expected_intersection = {0.5f, 0.f, 0.5f};
	assert(vec3_equals(&inter_pt, &expected_intersection));*/



	/*vec3_t	*v_start = &renderer->camera.clip_line.start;
	vec3_t	*v_end = &renderer->camera.clip_line.end;

	printf("horizontal clip line:\n");
	vec3_print(v_start);
	vec3_print(v_end);

	const vec3_t	*line_start = _line_start;
	const vec3_t	*line_end = _line_end;

	printf("to check line:\n");
	vec3_print(line_start);
	vec3_print(line_end);

	bool filtered = false;
	bool start_out = (place_of_vec3_z(v_start, v_end, line_start) < 0);
	bool end_out = (place_of_vec3_z(v_start, v_end, line_end) < 0);

	filtered = (start_out && end_out);

	printf("start out: %i  end out: %i\n", start_out, end_out);

	if (filtered) return false;

	vec2_t inter_p;
	vec2_t v2_start = {v_start->z, v_start->x};
	vec2_t v2_end = {v_end->z, v_end->x};

	vec2_t p2_s2 = {line_start->z, line_start->x};
	vec2_t p2_e2 = {line_end->z, line_end->x};

	lines_intersect_pt( &inter_p, &v2_start, &v2_end, &p2_s2, &p2_e2);

	printf("First Z/X (x/y) intersection:\n");
	vec2_print(&inter_p);

	//TODO MOVE UP VEC TO this intersection line and recalC
	vec3_t *up_start = &renderer->camera.clip_line.up_start;
	vec3_t *up_end = &renderer->camera.clip_line.up_end;
	
	printf("vertical y proofline:\n");
	vec3_print(up_start);
	vec3_print(up_end);
	
	vec2_t up_mov_s = { inter_p.y, up_start->y};
	vec2_t up_mov_e = { inter_p.y, up_end->y};

	float saved_z = inter_p.x;

	p2_s2 = (vec2_t){line_start->x, line_start->y};
	p2_e2 = (vec2_t){line_end->x, line_end->y};
	lines_intersect_pt( &inter_p, &up_mov_s, &up_mov_e, &p2_s2, &p2_e2);
	
	printf("First X/Y intersection:\n");
	vec2_print(&inter_p);
	vec2_print(&inter_p);

	//printf("3D intersection:\n");
	//vec3_t interception = {inter_p.x, inter_p.y, saved_z};
	vec3_copy_dest(_dest_start, line_start);
	vec3_copy_dest(_dest_end, line_end);

	vec3_t *to_clip = ( start_out ? _dest_start : _dest_end);
	vec3_set_values(to_clip, inter_p.x, inter_p.y, saved_z);

	printf("clipped point(the outer):\n");
	vec3_print(to_clip);
	*/

	return true;
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
	const vec2_t * samples = renderer->samples;
	const vec2_t * cursample;
	const int bufWidth = renderer->bufWidth;
	const unsigned int imgW = renderer->imgWidth, imgH = renderer->imgHeight, used_samples = renderer->used_samples;
	const float imgW_h = renderer->imgWidth_half, imgH_h = renderer->imgHeight_half, sample_factor = renderer->sample_factor;
	vec3_t pNDC1 = {ct->_14, ct->_24, ct->_34}, 
		   pNDC2 = {ct->_14, ct->_24, ct->_34}, pRaster1, pRaster2, pixelSample;
	cRGB_t * frameBuffer = renderer->frameBuffer;
	unsigned int curW, curH;
	float maxx, maxy, minx, miny, 
		  weight1 = ct->_44, 
		  weight2 = ct->_44, 
		  rz1, rz2;
	barycentric_t bc;
	float * zBuffer = renderer->zBuffer;

	//const vec3_t * v1v =  &v1->vec, * v2v =  &v2->vec;

	vec3_t _v1v, _v2v;
	const vec3_t * v1v = &_v1v, * v2v = &_v2v;
	if ( !__line_filter_or_clip(renderer, &_v1v, &_v2v, &v1->vec, &v2->vec) ) {
		return;
	}
	
	_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct);
	_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct);
	
	_compute_min_max_w_h_line(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH, &pRaster1, &pRaster2);
	
	for(; curH < maxy; ++curH) {
		unsigned int curHbufWidth = curH * bufWidth;
		for(curW = minx; curW < maxx; ++curW) {
			cursample = samples;
			unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
			for (unsigned int sample = used_samples; sample--;) {

				if ( _compute_sample_bc_and_check_line(&pixelSample, &cursample,&curW, &curH, &bc,
										 &pRaster1, &pRaster2)) { continue; }
				
				unsigned int bi = curWused_samples + sample;
				
				if ( _compute_and_set_z_line(&rz1, &rz2, &bc, &bi, zBuffer) )  { continue; }
								
				_set_color_to_fb_(frameBuffer,&bi ,&sample_factor,v1c);

			}
		}
	}
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

	cRGB_t curCol1, *fbc, *txc;
	//EO VARS

	if (_world_to_raster(v1v, &pNDC1, &pRaster1, &weight1, &imgW_h, &imgH_h, &rz1, ct)) return; 
	if (_world_to_raster(v2v, &pNDC2, &pRaster2, &weight2, &imgW_h, &imgH_h, &rz2, ct)) return; 
	if (_world_to_raster(v3v, &pNDC3, &pRaster3, &weight3, &imgW_h, &imgH_h, &rz3, ct)) return; 

	bc.area = 1.f/((pRaster3.x - pRaster1.x) * (pRaster2.y - pRaster1.y) - (pRaster3.y - pRaster1.y) * (pRaster2.x - pRaster1.x));

	_compute_min_max_w_h(&maxx, &maxy, &minx, &miny, &curW, &curH, &imgW, &imgH,
						 &pRaster1, &pRaster2, &pRaster3);
	
	
	//EO BOUNDING BOX
	//old vERSION JUST WORKING FINE. see below improoved version
	for(; curH < maxy; ++curH) {
		unsigned int curHbufWidth = curH * bufWidth;
		for(curW = minx; curW < maxx; ++curW) {
			cursample = samples;
			unsigned int curWused_samples = curHbufWidth + (curW * used_samples); 
			for (unsigned int sample = used_samples; sample--;) {

				if ( _compute_sample_bc_and_check(&pixelSample, &cursample,&curW, &curH, &bc,
										 &pRaster1, &pRaster2, &pRaster3)) { continue; }
				
				unsigned int bi = curWused_samples + sample;
				
				if ( _compute_and_set_z(&rz1, &rz2, &rz3,&bc, &bi, zBuffer) )  { continue; }
				
				_compute_px_color(&curCol1, &bc, &weight1, &weight2, &weight3,
								  renderer->texture, &imgW, v1c, v2c, v3c, v1t, v2t, v3t, &texId);

				_set_color_to_fb_(frameBuffer,&bi ,&sample_factor,&curCol1);
				//EO COLOR AND TEX
			}
		}
	}
	
}

void render_shape(renderer_t *  renderer, const shape_t *  shape){
	const shape_t *  curshape = shape;
	switch(curshape->cntVertex){
		case 3:
			render_triangle(renderer, curshape); break;
		case 2:
			render_line(renderer, curshape); break;
		case 1:
			render_point(renderer, curshape); break;
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

renderer_t * 
renderer_new(int imgWidth, int imgHeight, cRGB_t * bgColor, unsigned int samplestep){
	renderer_t * newrenderer = malloc(sizeof(renderer_t));
	newrenderer->projection = RP_ORTHOGRAPHIC;
	newrenderer->texture = NULL;
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
	
	return newrenderer;
}

void 
renderer_free(renderer_t * renderer){
	free(renderer->frameBuffer);
	free(renderer->zBuffer);
	free(renderer->samples);
	//free(renderer->wh_index);
	if (renderer->texture != NULL) {
		free(renderer->texture);
	}
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
