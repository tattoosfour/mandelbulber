				newz.x = z.x * z.x - z.y * z.y - z.z * z.z - z.w * z.w;
				newz.y = 2.0 * z.x * z.y;
				newz.z = 2.0 * z.x * z.z;
				newz.w = 2.0 * z.x * z.w;
				z = newz + c;
				r = fast_length(z);
				if (r < colourMin) colourMin = r;
				if(r>4000.0f || any(isinf(z))) 
				{
					distance = r;
					out.colourIndex = colourMin * 5000.0;
					break;
				}