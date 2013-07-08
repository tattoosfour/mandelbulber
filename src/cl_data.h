
typedef struct
{
	cl_float3 m1;
	cl_float3 m2;
	cl_float3 m3;
} matrix33;

typedef struct
{
	cl_float scale;
	cl_float foldingLimit;
	cl_float foldingValue;
	cl_float fixedRadius;
	cl_float minRadius;
	matrix33 mainRot;
} sClMandelbox;

typedef struct
{
	cl_int absX;
	cl_int absY;
	cl_int absZ;
	cl_int enabled[9];
	cl_int mengerSpongeMode;
	matrix33 mainRot;
	matrix33 rot[9];
	cl_float rotationAlpha;
	cl_float rotationBeta;
	cl_float rotationGamma;
	cl_float scale;
	cl_float distance[9];
	cl_float alpha[9];
	cl_float beta[9];
	cl_float gamma[9];
	cl_float intensity[9];
	cl_float3 offset;
	cl_float3 direction[9];
	cl_float3 edge;
} sClIFS;

typedef struct
{
	cl_uint N;
	cl_float power;
	cl_int formula;
	cl_float3 julia;
	sClMandelbox mandelbox;
	sClIFS ifs;
	cl_int juliaMode;
	cl_float opacity;
	cl_float opacityTrim;
	cl_int constantDEThreshold;
} sClFractal;

typedef struct
{
	cl_int width;
	cl_int height;
	cl_float alpha;
	cl_float beta;
	cl_float gamma;
	cl_float zoom;
	cl_float persp;
	cl_float DEfactor;
	cl_float quality;
	cl_float3 vp;
	cl_float mainLightAlfa;
	cl_float mainLightBeta;
	cl_int AmbientOcclusionNoOfVectors;
	cl_int colouringEnabled;
	cl_int fastAmbientOcclusionEnabled;
	cl_int slowAmbientOcclusionEnabled;
	cl_int DOFEnabled;
	cl_float colouringSpeed;
	cl_float colouringOffset;
	cl_float ambientOcclusionIntensity;
	cl_float specularIntensity;
	cl_float mainLightIntensity;
	cl_float glowIntensity;
	cl_float fogColour1Distance;
	cl_float fogColour2Distance;
	cl_float fogDistanceFactor;
	cl_float fogDensity;
	cl_float DOFFocus;
	cl_float DOFRadius;
	cl_float3 glowColour1;
	cl_float3 glowColour2;
	cl_float3 backgroundColour1;
	cl_float3 backgroundColour2;
	cl_float3 backgroundColour3;
	cl_float3 mainLightColour;
	cl_float3 fogColour1;
	cl_float3 fogColour2;
	cl_float3 fogColour3;
} sClParams;

typedef struct
{
	cl_ushort R;
	cl_ushort G;
	cl_ushort B;
	cl_float zBuffer;
} sClPixel;

typedef struct
{
	sClParams params;
	sClFractal fractal;
	cl_float3 palette[256];
	cl_float3 vectorsAround[10000];
	cl_float3 vectorsAroundColours[10000];
} sClInBuff;