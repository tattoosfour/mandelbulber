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
	float distance_a = max(point.x - consts->fractal.amax, -(point.x - consts->fractal.amin));
	float distance_b = max(point.y - consts->fractal.bmax, -(point.y - consts->fractal.bmin));
	float distance_c = max(point.z - consts->fractal.cmax, -(point.z - consts->fractal.cmin));
	float limitBoxDist = max(distance_a, max(distance_b, distance_c));

	if(limitBoxDist > calcParam->distThresh)
	{
		out.distance = limitBoxDist;
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
	
	if (consts->fractal.interiorMode && !calcParam->normalCalculationMode)
	{
		if (out.distance < 0.5f * calcParam->distThresh)
		{
			out.distance = calcParam->distThresh;
		}
	}
	else if(consts->fractal.interiorMode && calcParam->normalCalculationMode)
	{
		if (out.distance < 0.9f * calcParam->distThresh)
		{
			out.distance = calcParam->distThresh - out.distance;
		}
	}
	
	if (limitBoxDist < calcParam->distThresh)
	{
		out.distance = max(out.distance, limitBoxDist);
	}
	
#endif
	return out;
}