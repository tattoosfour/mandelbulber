		int3 cond1, cond2;
		
		cond1 = isgreater(z, foldingLimit);
		cond2 = isless(z, -foldingLimit);
		z = select(z,  foldingValue - z, cond1);
		z = select(z,  -foldingValue - z, cond2);
		
		//z = fabs(z + ones) - fabs(z - ones) - z;

		float rr = dot(z,z);
		float m = scale;
		if (rr < mr2)	m = native_divide(scale, mr2);
		else if (rr < fr2)	m = native_divide(scale,rr);
		
		z = Matrix33MulFloat3(fr->mandelbox.mainRot, z);
		
		z = z * m + c;
		tgladDE = tgladDE * fabs(m) + 1.0f;
		r = dot(z,z);
		
		colourMin += fabs(m);
		
		if(r>1024.0f) 
		{
			distance = native_sqrt(r) / fabs(tgladDE);
			out.colourIndex = colourMin / i * 300.0;
			break;
		}