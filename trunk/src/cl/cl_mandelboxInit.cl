	float tgladDE = 1.0f;
	float4 ones = 1.0f;
	float4 foldingLimit = fr->mandelbox.foldingLimit;
	float4 foldingValue = fr->mandelbox.foldingValue;
	float mr2 = fr->mandelbox.minRadius * fr->mandelbox.minRadius;
	float fr2 = fr->mandelbox.fixedRadius * fr->mandelbox.fixedRadius;
	float scale = fr->mandelbox.scale;
	colourMin = 0.0;