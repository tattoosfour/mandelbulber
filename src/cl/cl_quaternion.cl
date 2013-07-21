				newz.x = z.x * z.x - z.y * z.y - z.z * z.z - w * w;
				newz.y = 2.0f* z.x * z.y;
				newz.z = 2.0f * z.x * z.z;
				neww = 2.0f * z.x * w;
				z = newz + c;
				w = neww;
				r = fast_length(z);
				r = sqrt(r*r + w*w);
				if (r < colourMin) colourMin = r;
				if(r>4000.0f || any(isinf(z))) 
				{
					distance = r;
					out.colourIndex = colourMin * 5000.0f;
					break;
				}