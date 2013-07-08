formulaOut Fractal(float3 point, sClFractal *fr)
{	
	float distance = 0.0f;
	int N = fr->N;
	float3 z = point;
	float3 c = fr->julia;
	int i;
	formulaOut out;
	float r = 0.0f;
	float colourMin = 1e8;
	
	
