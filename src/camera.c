#include "camera.h"

void setviewport(camera_t *  camera, const float l,const float r,const float t,const float b,const float near,const float far){
	camera_t * curcam = camera;
	curcam->l = l;
	curcam->r = r;
	curcam->t = t;
	curcam->b = b;
	curcam->n = near;
	curcam->f = far;
}

void config_camera(camera_t *  camera, const vec3_t *  from, const vec3_t *  to, 
                   const float l,const float r,const float t,const float b,const float near,const float far) {
	camera_t *  curcam = camera;
	setviewport(curcam,l,r,t,b,near,far);
	camera_lookAt_ortho(curcam, from, to);
	createProjectionOrtho(curcam, l, r, t, b, near, far);
	//mat4_mul_dest(&curcam->transformation ,&curcam->view, &curcam->projection);
	mat4_mul_dest(&curcam->transformation ,&curcam->projection, &curcam->view);
}

void config_camera_perspective(camera_t *  camera, const vec3_t *  from, const vec3_t *  to, 
                   const float l,const float r,const float t,const float b,const float near,const float far) {
	camera_t *  curcam = camera;
	setviewport(curcam,l,r,t,b,near,far);
	camera_lookAt_perspective(curcam, from, to);
	createProjectionPerspective(curcam, l, r, t, b, near, far);
	//mat4_mul_dest(&curcam->transformation ,&curcam->view, &curcam->projection);
	mat4_mul_dest(&curcam->transformation ,&curcam->projection, &curcam->view);
}

void 
camera_lookAt_ortho(camera_t *  camera, const vec3_t *  from, const vec3_t *  to) {
	const vec3_t *  eye = from;
	camera_t *  cam = camera;
	mat4_t projection;
	
	vec3_sub_dest(&cam->forward, eye, to);
	vec3_normalize(&cam->forward);
	
	vec3_t tmp = { 0.f, 1.f, 0.f};
	vec3_normalize(&tmp);
	
	vec3_cross_dest(&cam->left, &tmp, &cam->forward);
	vec3_normalize(&cam->left);
	
	vec3_cross_dest(&cam->up, &cam->forward, &cam->left);
	vec3_normalize(&cam->up);
	
	projection._11 = cam->left.x;
	projection._21 = cam->left.y;
	projection._31 = cam->left.z;
	projection._41 = 0.f;//-vec3_vec3mul(right, from);
	
	projection._12 = cam->up.x;
	projection._22 = cam->up.y;
	projection._32 = cam->up.z;
	projection._42 = 0.f;//-vec3_vec3mul(up, from);//0.f;
	
	projection._13 = cam->forward.x;
	projection._23 = cam->forward.y;
	projection._33 = cam->forward.z;
	projection._43 = 0.f;//vec3_vec3mul(forward, from);//0.f;
	
	projection._14 = 0.f;//eye->x;
	projection._24 = 0.f;//eye->y;
	projection._34 = 0.f;//eye->z;
	projection._44 = 1.f;
	
	mat4_inverse_dest(&cam->projection, &projection);
}

void 
camera_lookAt_perspective(camera_t *  camera, const vec3_t *  from, const vec3_t *  to) {
	
	const vec3_t *  eye = from;
	camera_t *  cam = camera;
	vec3_t * f = &cam->forward;
	vec3_t * l = &cam->left;
	vec3_t * u = &cam->up;

	vec3_sub_dest(f, eye, to);
	vec3_normalize(f);
	
	vec3_t tmp = { 0.f, 1.f, 0.f};
	vec3_normalize(&tmp);
	
	vec3_cross_dest(l, &tmp, f);
	vec3_normalize(l);
	
	vec3_cross_dest(u, f, l);
	vec3_normalize(u);
	
	mat4_t m = { l->x	,u->x,	-f->x	,eye->x,	
				 l->y	,u->y,	-f->y	,eye->y,	
				 l->z	,u->z,	-f->z	,eye->z, 	
				 0.f	,0.f ,	0.f		,1.f };
	
	//inverse: Base matrix
	mat4_t * dest = &cam->projection;
	mat3_t t = { m._22, m._23, m._24, m._32, m._33, m._34, m._42, m._43, m._44};

	dest->_11 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&temp);

	t._11 = m._21; t._21 = m._31; t._31 = m._41;
	dest->_21 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				  (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._12 = m._22; t._22 = m._32; t._32 = m._42;
	dest->_31 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._13 = m._23; t._23 = m._33; t._33 = m._43;
	dest->_41 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				  (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._11 = m._12; t._12 = m._13; t._13 = m._14; 
	t._21 = m._32; t._22 = m._33; t._23 = m._34; 
	t._31 = m._42; t._32 = m._43; t._33 = m._44;
	dest->_12 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				  (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._11 = m._11; t._21 = m._31; t._31 = m._41;
	dest->_22 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
			     (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._12 = m._12; t._22 = m._32; t._32 = m._42;
	dest->_32 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				  (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._13 = m._13; t._23 = m._33; t._33 = m._43;
	dest->_42 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);
	
	t._11 = m._12; t._12 = m._13; t._13 = m._14; 
	t._21 = m._22; t._22 = m._23; t._23 = m._24; 
	t._31 = m._42; t._32 = m._43; t._33 = m._44;
	dest->_13 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._11 = m._11; t._21 = m._21; t._31 = m._41;
	dest->_23 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				(t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._12 = m._12; t._22 = m._22; t._32 = m._42;
	dest->_33 = ((t._11*t._22*t._33) +(t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._13 = m._13; t._23 = m._23; t._33 = m._43;
	dest->_43 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				(t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);
	
	t._11 = m._12; t._12 = m._13; t._13 = m._14; 
	t._21 = m._22; t._22 = m._23; t._23 = m._24; 
	t._31 = m._32; t._32 = m._33; t._33 = m._34;
	dest->_14 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) - 
	              (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._11 = m._11; t._21 = m._21; t._31 = m._31;
	dest->_24 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._12 = m._12; t._22 = m._22; t._32 = m._32;
	dest->_34 = -((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				  (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);

	t._13 = m._13; t._23 = m._23; t._33 = m._33;
	dest->_44 = ((t._11*t._22*t._33) + (t._12*t._23*t._31) + (t._13*t._21*t._32) -
				 (t._13*t._22*t._31) - (t._12*t._21*t._33) - (t._11*t._23*t._32));//mat3_determinant(&t);
	
	//inverse: determinant inside inverse calc
	float det = (m._11 * dest->_11) + 
				(m._12 * -dest->_21 ) + 
				(m._13 * dest->_31) +
				(m._14 * -dest->_41);
	
	//inverse: mul at with 1/det
	dest->_11 *= det; dest->_12 *= det; dest->_13 *= det; dest->_14 *= det;
	dest->_21 *= det; dest->_22 *= det; dest->_23 *= det; dest->_24 *= det;
	dest->_31 *= det; dest->_32 *= det; dest->_33 *= det; dest->_34 *= det;
	dest->_41 *= det; dest->_42 *= det; dest->_43 *= det; dest->_44 *= det;
}

void 
createProjectionOrtho(camera_t *  camera, const float l,const float r,const float t,const float b,const float near,const float far) {
	camera_t *  cam = camera;
	cam->view._11 = 2.f/(r-l);
	cam->view._12 = 0.f;
	cam->view._13 = 0.f;
	cam->view._14 = -((r+l)/(r-l));

	cam->view._21 = 0.f;
	cam->view._22 = 2.f/(t-b);
	cam->view._23 = 0.f;
	cam->view._24 = -((t+b)/(t-b));

	cam->view._31 = 0.f;
	cam->view._32 = 0.f;
	cam->view._33 = -2.f/(far-near);
	cam->view._34 = -((far+near)/(far-near));

	cam->view._41 = 0.f;
	cam->view._42 = 0.f;
	cam->view._43 = 0.f;
	cam->view._44 = 1.f;
}

#if 0
	//This is the openGL
#endif
void
createProjectionPerspective(camera_t *  camera, const float l,const float r,const float t,const float b,const float near,const float far) {
	camera_t *  cam = camera;
	float scale = 1 / tan(90.f * 0.5f * M_PI / 180.f); 
	cam->view._11 = scale;//(2.f*near)/(r-l);//scale;//
	cam->view._12 = 0.f;
	cam->view._13 = (r+l)/(r-l);
	cam->view._14 = 0.f;
	   
	cam->view._21 = 0.f;
	cam->view._22 = scale;//(2.f*near)/(t-b); //scale;//
	cam->view._23 = (t+b)/(t-b);
	cam->view._24 = 0.f;
	   
	cam->view._31 = 0.f;
	cam->view._32 = 0.f;
	cam->view._33 = -(far+near)/(far-near);
	cam->view._34 = -(2.f*far*near)/(far-near);
	  
	cam->view._41 = 0.f;
	cam->view._42 = 0.f;
	cam->view._43 = 1.f;
	cam->view._44 = 0.f;
}

void 
print_camera(const camera_t *  camera) {
	const camera_t *  cam = camera;
	printf("forw.:\t"); vec3_print(&cam->forward);
	printf("left:\t"); vec3_print(&cam->left);
	printf("up:\t"); vec3_print(&cam->up);
	printf("camera:\t"); mat4_print(&cam->view);
	printf("projection:\t"); mat4_print(&cam->projection);
	printf("transformation:\t"); mat4_print(&cam->transformation);
}