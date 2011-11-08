		int4 cond1, cond2;
		
		/*
		cond1.x = isgreater(z.x, foldingLimit);
		cond1.y = isgreater(z.y, foldingLimit);
		cond1.z = isgreater(z.z, foldingLimit);
		cond2.x = isless(z.x, -foldingLimit);
		cond2.y = isless(z.y, -foldingLimit);
		cond2.z = isless(z.z, -foldingLimit);
		z.x = select(z.x,  foldingValue - z.x, cond1.x);
		z.y = select(z.y,  foldingValue - z.y, cond1.y);
		z.z = select(z.z,  foldingValue - z.z, cond1.z);
		z.x = select(z.x,  -foldingValue - z.x, cond2.x);
		z.y = select(z.y,  -foldingValue - z.y, cond2.y);
		z.z = select(z.z,  -foldingValue - z.z, cond2.z);
		*/
		
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
		float r = dot(z,z);
		
		if(r>1024.0f) 
		{
			distance = native_sqrt(r) / fabs(tgladDE);
			break;
		}