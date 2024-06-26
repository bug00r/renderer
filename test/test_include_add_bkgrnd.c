void addBackgroundToRenderer(Renderer * renderer) {
	Texture * texture_fractals = texture_new(renderer->imgWidth,renderer->imgHeight);
	
	Mandelbrot *mb = mandelbrot_new(renderer->imgWidth, renderer->imgHeight);
	mb->minreal = -2.f;//-1.3f;
	mb->maxreal = 0.5f;//-1.f;
	mb->minimag = -1.f;//-.3f;
	mb->maximag = 1.f;//0.f;
	mb->cntiterations = 20;
	create_mandelbrot(mb);
	
	Mandelbroto_texture(mb, texture_fractals, mandelbrot_color_line_int_rgb);
	
	int bufsize = renderer->imgWidth*renderer->imgHeight*sizeof(ColorRGB);
	memcpy(renderer->frameBuffer, texture_fractals->buffer->entries, bufsize);
    
	mandelbrot_free(mb);
	texture_free(texture_fractals);
}