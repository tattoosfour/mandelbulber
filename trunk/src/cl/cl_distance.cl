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
	formulaOut out=Fractal(consts, point, calcParam);
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
	return out;
}