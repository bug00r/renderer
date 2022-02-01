void test_render_cylinder(renderer_t * renderer, bool isperspective) {
	//createcylinder(float radius, float height, unsigned int longs, unsigned lats, bool showtop, bool showbottom)
	mesh_t * cylinder = createcylinder(0.5f, 1.5f, 30, 30, true, true);
	
	render_mesh(renderer, cylinder);
	
	#ifdef output
		char * filename = create_string("build/__%u_cylinder_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cylinder_%uxMSAA.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#else
		(void)(isperspective);
	#endif
	
	mat3_t * rotx_mat = create_rot_x_mat(-45.f);
	mat3_t * roty_mat = create_rot_y_mat(0.f);
	mat3_mul(rotx_mat, roty_mat);
	
	mat_mul_mesh(cylinder, rotx_mat);
	free(rotx_mat);
	free(roty_mat);
	
	
	renderer_clear_frame(renderer);
	render_mesh(renderer, cylinder);
	
	#ifdef output
		filename = create_string("build/__%u_cylinder_%uxMSAA_rot45.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_ppm(renderer, filename);
		free(filename);
		
		filename = create_string("build/_z__%u_cylinder_%uxMSAA_rot45.ppm", 
										isperspective, renderer->samplestep);
		renderer_output_z_buffer_ppm(renderer, filename);
		free(filename);
	#endif
	
	free_mesh(cylinder);
}