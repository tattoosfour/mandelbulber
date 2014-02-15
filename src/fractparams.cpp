/*********************************************************
 /                   MANDELBULBER
 / definition of structures for image and effect
 / parameters
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 ********************************************************/
#include "fractparams.hpp"

void InitParams(parameters::container *par)
{
	par->addParam("image_width", 800);
	par->addParam("image_height", 600);
	par->addParam("tiles", 1);
	par->addParam("limit_min", CVector3(-10.0, -10.0, -10.0));
	par->addParam("limit_max", CVector3(10.0, 10.0, 10.0));
	par->addParam("view_point", CVector3(0.0, 0.0, 0.0));
	par->addParam("angle_alpha", -20.0);
	par->addParam("angle_beta", 30.0);
	par->addParam("angle_gamma", 0.0);
	par->addParam("zoom", 2.5);
	par->addParam("perspective", 0.5);
	par->addParam("formula", (int)trig_optim);
	par->addParam("power", 9.0);
	par->addParam("N", 250);
	par->addParam("minN", 1);
	par->addParam("fractal_constant_factor", 1.0);
	par->addParam("quality", 1.0);
	par->addParam("smoothness", 1.0);
	par->addParam("julia_mode", false);
	par->addParam("julia", CVector3(0.0, 0.0, 0.0));
	par->addParam("tglad_folding_mode", false);
	par->addParam("folding_limit", 1.0);
	par->addParam("folding_value", 2.0);
	par->addParam("spherical_folding_mode", false);
	par->addParam("spherical_folding_fixed", 1.0);
	par->addParam("spherical_folding_min", 0.5);
	par->addParam("IFS_folding_mode", false);
	par->addParam("iteration_threshold_mode", false);
	par->addParam("analityc_DE_mode", true);
	par->addParam("DE_factor", 1.0);
	par->addParam("brightness", 1.0);
	par->addParam("contrast", 1.0);
	par->addParam("gamma", 1.0);
	par->addParam("hdr", false);
	par->addParam("ambient", 0.0);
	par->addParam("reflect", 0.0);
	par->addParam("shadows_intensity", 0.7);
	par->addParam("shadows_cone_angle", 1.0);
	par->addParam("ambient_occlusion", 1.0);
	par->addParam("ambient_occlusion_quality", 4);
	par->addParam("ambient_occlusion_fast_tune", 1.0);
}


