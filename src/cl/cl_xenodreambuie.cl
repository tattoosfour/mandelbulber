				float rp = native_powr(r, p);
				float th = atan2(z.y, z.x);
				float ph = acos(z.z / r);
				if (ph > 0.5f * M_PI)
				{
					ph = M_PI - ph;
				}
				else if (ph < -0.5f * M_PI)
				{
					ph = -M_PI - ph;
				}
				z.x = rp * native_cos(th * p) * native_sin(ph * p);
				z.y = rp * native_sin(th * p) * native_sin(ph * p);
				z.z = rp * native_cos(ph * p);
				z+=c;
				r = fast_length(z);
				if (r < colourMin) colourMin = r;
				if(r>4000.0f || any(isinf(z))) 
				{
					distance = r;
					out.colourIndex = colourMin * 5000.0f;
					break;
				}