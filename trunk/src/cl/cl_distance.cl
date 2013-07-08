formulaOut CalculateDistance(float3 point, sClFractal *fractal)
{
	formulaOut out=Fractal(point, fractal);
	if (out.iters == fractal->N)
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