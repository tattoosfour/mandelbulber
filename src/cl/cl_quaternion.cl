				newz.x = z4.x * z4.x - z4.y * z4.y - z4.z * z4.z - z4.w * z4.w;
				newz.y = 2.0f* z4.x * z4.y;
				newz.z = 2.0f * z4.x * z4.z;
				newz.w = 2.0f * z4.x * z4.w;
				z4 = newz + c4;
				r = fast_length(z4);
				if (r < colourMin) colourMin = r;
				if(r>4000.0f || any(isinf(z4))) 
				{
					distance = r;
					out.colourIndex = colourMin * 5000.0f;
					break;
				}