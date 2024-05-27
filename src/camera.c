#include "camera.h"

void setviewport(Camera *  camera, const float l,const float r,const float t,const float b,const float near,const float far){
	Camera * curcam = camera;
	curcam->l = l;
	curcam->r = r;
	curcam->t = t;
	curcam->b = b;
	curcam->n = near;
	curcam->f = far;
}

void config_camera(Camera *  camera, const Vec3 *  from, const Vec3 *  to, 
                   const float l,const float r,const float t,const float b,const float near,const float far) {
	Camera *  curcam = camera;
	setviewport(curcam,l,r,t,b,near,far);
	camera_lookAt(curcam, from, to);
	createProjectionOrtho(curcam, l, r, t, b, near, far);
	
	mat4_mul_dest(&curcam->transformation ,&curcam->view, &curcam->projection);

	//mat4_mul_dest(&curcam->transformation ,&curcam->projection, &curcam->view);
}

void config_camera_perspective(Camera *  camera, const Vec3 *  from, const Vec3 *  to, 
                   const float l,const float r,const float t,const float b,const float near,const float far) {
	Camera *  curcam = camera;
	setviewport(curcam,l,r,t,b,near,far);
	camera_lookAt(curcam, from, to);
	createProjectionPerspective(curcam, l, r, t, b, near, far);
	mat4_mul_dest(&curcam->transformation ,&curcam->view, &curcam->projection);
	//mat4_mul_dest(&curcam->transformation ,&curcam->projection, &curcam->view);
	
	//mat4_copy(&curcam->transformation ,&curcam->projection);
}

static void __calc_normal(Plane *plane) {
	Plane *p = plane;
	Vec3 tmp, tmp2;
	vec3_sub_dest(&tmp, &p->rb, &p->lb);
	vec3_sub_dest(&tmp2, &p->lt, &p->lb);

	vec3_cross_dest(&p->normal, &tmp, &tmp2);
	vec3_normalize(&p->normal);
}

static void __calc_frustum(Camera *  camera) {
	Camera *cam = camera;
	Frustum *frustum = &cam->frustum;
	Plane *near = &frustum->near;
	Plane *far = &frustum->far;
	Plane *left = &frustum->left;
	Plane *right = &frustum->right;
	Plane *top = &frustum->top;
	Plane *bottom = &frustum->bottom;

	Vec3 tmp;
	Vec3 tmp2;

	Vec3 centerNear;
	vec3_negate_dest(&centerNear, &cam->forward);
	vec3_mul_dest(&tmp, &centerNear, cam->n);
	vec3_add_dest(&centerNear, &cam->from, &tmp);

	Vec3 nearUpDir;
	vec3_mul_dest(&nearUpDir, &cam->up, cam->t);

	Vec3 nearDownDir;
	vec3_negate_dest(&nearDownDir, &nearUpDir);

	Vec3 nearTopRight;
	vec3_mul_dest(&tmp, &cam->left, cam->r); //cam->left means right :D..don't ask yet :P
	vec3_add_n_dest(&nearTopRight, 3,&centerNear, &nearUpDir, &tmp);
	
	Vec3 nearTopLeft;
	vec3_negate_dest(&tmp2, &tmp);
	vec3_add_n_dest(&nearTopLeft, 3, &centerNear, &nearUpDir, &tmp2);

	Vec3 nearBottomRight;
	vec3_mul_dest(&tmp, &cam->left, cam->r); //cam->left means right :D..don't ask yet :P
	vec3_add_n_dest(&nearBottomRight, 3, &centerNear, &nearDownDir, &tmp);

	Vec3 nearBottomLeft;
	vec3_negate_dest(&tmp2, &tmp);
	vec3_add_n_dest(&nearBottomLeft, 3, &centerNear, &nearDownDir, &tmp2);

	//near plane ready
	vec3_copy_dest(&near->lb, &nearBottomRight);
	vec3_copy_dest(&near->rb, &nearBottomLeft);
	vec3_copy_dest(&near->lt, &nearTopRight);
	vec3_copy_dest(&near->rt, &nearTopLeft);

	float farFactor = cam->f / cam->n;

	Vec3 centerFar;
	vec3_negate_dest(&centerFar, &cam->forward);
	vec3_mul_dest(&tmp, &centerFar, cam->f);
	vec3_add_dest(&centerFar, &cam->from, &tmp);

	Vec3 farUpDir;
	vec3_mul_dest(&farUpDir, &cam->up, cam->t * farFactor);

	Vec3 farDownDir;
	vec3_negate_dest(&farDownDir, &farUpDir);

	Vec3 farTopRight;
	vec3_mul_dest(&tmp, &cam->left, cam->r * farFactor); //cam->left means right :D..don't ask yet :P
	vec3_add_n_dest(&farTopRight, 3,&centerFar, &farUpDir, &tmp);
	
	Vec3 farTopLeft;
	vec3_negate_dest(&tmp2, &tmp);
	vec3_add_n_dest(&farTopLeft, 3, &centerFar, &farUpDir, &tmp2);

	Vec3 farBottomRight;
	vec3_mul_dest(&tmp, &cam->left, cam->r * farFactor); //cam->left means right :D..don't ask yet :P
	vec3_add_n_dest(&farBottomRight, 3, &centerFar, &farDownDir, &tmp);

	Vec3 farBottomLeft;
	vec3_negate_dest(&tmp2, &tmp);
	vec3_add_n_dest(&farBottomLeft, 3, &centerFar, &farDownDir, &tmp2);

	//far plane ready
	vec3_copy_dest(&far->lb, &farBottomLeft);
	vec3_copy_dest(&far->rb, &farBottomRight);
	vec3_copy_dest(&far->lt, &farTopLeft);
	vec3_copy_dest(&far->rt, &farTopRight);

	//set left plane
	vec3_copy_dest(&left->lb, &near->rb);
	vec3_copy_dest(&left->lt, &near->rt);
	vec3_copy_dest(&left->rb, &far->lb);
	vec3_copy_dest(&left->rt, &far->lt);

	//set right plane
	vec3_copy_dest(&right->lb, &far->rb);
	vec3_copy_dest(&right->lt, &far->rt);
	vec3_copy_dest(&right->rb, &near->lb);
	vec3_copy_dest(&right->rt, &near->lt);

	//set top plane
	vec3_copy_dest(&top->lb, &far->lt);
	vec3_copy_dest(&top->lt, &near->rt);
	vec3_copy_dest(&top->rb, &far->rt);
	vec3_copy_dest(&top->rt, &near->lt);

	//set top plane
	vec3_copy_dest(&bottom->lb, &near->rb);
	vec3_copy_dest(&bottom->lt, &far->lb);
	vec3_copy_dest(&bottom->rb, &near->lb);
	vec3_copy_dest(&bottom->rt, &far->rb);

	//normals are going to center of frustum
	//near
	__calc_normal(near);
	__calc_normal(far);
	__calc_normal(left);
	__calc_normal(right);
	__calc_normal(top);
	__calc_normal(bottom);

}

void 
camera_lookAt(Camera *  camera, const Vec3 *  from, const Vec3 *  to) {
	
	const Vec3 *  eye = from;
	Camera *  cam = camera;
	Vec3 * f = &cam->forward;
	Vec3 * l = &cam->left;
	Vec3 * u = &cam->up;

	vec3_copy_dest(&cam->from, from);
	vec3_copy_dest(&cam->to, to);

	vec3_sub_dest(f, eye, to);
	vec3_normalize(f);

	Vec3 tmp = { 0.f, 1.f, 0.f};
	vec3_normalize(&tmp);
	
	vec3_cross_dest(l, &tmp, f);
	vec3_normalize(l);
	
	vec3_cross_dest(u, f, l);
	vec3_normalize(u);
	
	__calc_frustum(cam);

	Mat4 m = { l->x	,u->x,	f->x	,eye->x,	
				 l->y	,u->y,	f->y	,eye->y,	
				 l->z	,u->z,	f->z	,eye->z, 	
				 0.f	,0.f ,	0.f		,1.f };
	
	//inverse: Base matrix, below complex, could be => mat4_inverse_dest(&cam->projection, &projection);
	Mat4 * dest = &cam->projection;
	Mat3 t = { m._22, m._23, m._24, m._32, m._33, m._34, m._42, m._43, m._44};

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
createProjectionOrtho(Camera *  camera, const float l,const float r,const float t,const float b,const float near,const float far) {
	Camera *  cam = camera;
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
createProjectionPerspective(Camera *  camera, const float l,const float r,const float t,const float b,const float near,const float far) {
	Camera *  cam = camera;
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
	cam->view._43 = -1.f;
	cam->view._44 = 0.f;

}

static void print_plane(const Plane *plane) {
	const Plane *p = plane;
	printf("lb: ");vec3_print(&p->lb);
	printf("rb: ");vec3_print(&p->rb);
	printf("lt: ");vec3_print(&p->lt);
	printf("rt: ");vec3_print(&p->rt);
	printf("normal: ");vec3_print(&p->normal);
}

static void print_frustum(const Frustum *frustum) {
	const Frustum *fu = frustum;
	printf("near plane:\n");
	print_plane(&fu->near);
	printf("far plane:\n");
	print_plane(&fu->far);
	printf("left plane:\n");
	print_plane(&fu->left);
	printf("right plane:\n");
	print_plane(&fu->right);
	printf("top plane:\n");
	print_plane(&fu->top);
	printf("bottom plane:\n");
	print_plane(&fu->bottom);
}

void 
print_camera(const Camera *  camera) {
	const Camera *  cam = camera;
	printf("from.:\t"); vec3_print(&cam->from);
	printf("to:\t"); vec3_print(&cam->to);
	printf("forw.:\t"); vec3_print(&cam->forward);
	printf("left:\t"); vec3_print(&cam->left);
	printf("up:\t"); vec3_print(&cam->up);
	print_frustum(&cam->frustum);
	printf("camera:\t"); mat4_print(&cam->view);
	printf("projection:\t"); mat4_print(&cam->projection);
	printf("transformation:\t"); mat4_print(&cam->transformation);
}