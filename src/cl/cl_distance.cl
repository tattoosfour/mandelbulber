/*********************************************************
 /                   MANDELBULBER
 / function to calculate extimated distance
 / using Fractal() function
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

	out = Fractal(consts, point, calcParam);
	if (out.iters == calcParam->N)
	{
		out.distance = 0.0f;
	}
	else
	{
		if(isinf(out.distance)) out.distance = 0.0f;
		if(out.distance<0.0f) out.distance = 0.0f;
		if(out.distance>10.0f) out.distance = 10.0f;
	}
	
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