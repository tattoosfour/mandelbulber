/*********************************************************
 /                   MANDELBULBER
 / function to calculate extimated distance
 / using "delta DE" method
 /
 / author: Krzysztof Marczak
 / contact: buddhi1980@gmail.com
 / licence: GNU GPL v3.0
 /
 ********************************************************/

formulaOut CalculateDistance(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam)
{
	formulaOut out;
	out.z = 0.0f;
	out.iters = 0;
	out.distance = 0.0f;
	out.colourIndex = 0.0f;
	out.distFromOrbitTrap = 0.0f;

#if _LimitsEnabled
	
	bool limit = false;
	float distance_a = 0;
	float distance_b = 0;
	float distance_c = 0;
	float distThresh = calcParam->distThresh;

	if (point.x < consts->fractal.amin - distThresh)
	{
		distance_a = fabs(consts->fractal.amin - point.x);
		limit = true;
	}
	else if (point.x > consts->fractal.amax + distThresh)
	{
		distance_a = fabs(consts->fractal.amax - point.x);
		limit = true;
	}

	if (point.y < consts->fractal.bmin - distThresh)
	{
		distance_a = fabs(consts->fractal.bmin - point.y);
		limit = true;
	}
	else if (point.y > consts->fractal.bmax + distThresh)
	{
		distance_b = fabs(consts->fractal.bmax - point.y);
		limit = true;
	}

	if (point.z < consts->fractal.cmin - distThresh)
	{
		distance_c = fabs(consts->fractal.cmin - point.z);
		limit = true;
	}
	else if (point.z > consts->fractal.cmax + distThresh)
	{
		distance_c = fabs(consts->fractal.cmax - point.z);
		limit = true;
	}

	if (limit)
	{
		out.distance = max(distance_a, max(distance_b, distance_c));
		return out;
	}
#endif 
	
	float distance;
	float delta = length(point)*1e-5f;
	float3 dr = 0.0;
	out = Fractal(consts, point, calcParam);

	if (out.iters == calcParam->N)
	{
		distance = 0.0f;
	}
	else
	{
		float r = length(out.z);
		float r11 = length(Fractal(consts, point + (float3)	{ delta, 0.0, 0.0}, calcParam).z);
		float r12 = length(Fractal(consts, point + (float3)	{ -delta, 0.0, 0.0}, calcParam).z);
		dr.x = min(fabs(r11 - r), fabs(r12 - r)) / delta;
		float r21 = length(Fractal(consts, point + (float3)	{ 0.0, delta, 0.0}, calcParam).z);
		float r22 = length(Fractal(consts, point + (float3)	{ 0.0, -delta, 0.0}, calcParam).z);
		dr.y = min(fabs(r21 - r), fabs(r22 - r)) / delta;
		float r31 = length(Fractal(consts, point + (float3)	{ 0.0, 0.0, delta}, calcParam).z);
		float r32 = length(Fractal(consts, point + (float3)	{ 0.0, 0.0, -delta}, calcParam).z);
		dr.z = min(fabs(r31 - r), fabs(r32 - r)) / delta;
		float d = length(dr);

		if (isinf(r) || isinf(d))
		{
			distance = 0.0f;
		}
		else
		{
			if(consts->fractal.linearDEmode)
			{
				distance = 0.5 * r  / d;
			}
			else
			{
				distance = 0.5 * r * native_log(r) / d;	
			}
		}
		if (distance < 0.0f) distance = 0.0f;
		if (distance > 10.0f) distance = 10.0f;
	}
	out.distance = distance;
	
#if _LimitsEnabled
	if (out.distance < calcParam->distThresh)
	{

		float distance_a1 = fabs(consts->fractal.amin - point.x);
		float distance_a2 = fabs(consts->fractal.amax - point.x);
		float distance_b1 = fabs(consts->fractal.bmin - point.y);
		float distance_b2 = fabs(consts->fractal.bmax - point.y);
		float distance_c1 = fabs(consts->fractal.cmin - point.z);
		float distance_c2 = fabs(consts->fractal.cmax - point.z);
		float min1 = min(distance_a1, min(distance_b1, distance_c1));
		float min2 = min(distance_a2, min(distance_b2, distance_c2));
		float min3 = min(min1, min2);
		if(min3 < calcParam->distThresh)
		{
			out.distance = min3;
		}
	}
#endif
	return out;
}