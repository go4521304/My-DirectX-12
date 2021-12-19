#define _WITH_CONSTANT_BUFFER_SYNTAX

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
struct CB_PLAYER_INFO
{
	matrix		mtxWorld;
};

struct CB_GAMEOBJECT_INFO
{
	matrix		mtxWorld;
};

struct CB_CAMERA_INFO
{
	matrix		mtxView;
	matrix		mtxProjection;
	float3		position;
};
struct CB_BILLBOARD_INDEX
{
	uint		type;
};

struct CB_FRAMEWORK_INFO
{
	float					m_fCurrentTime;
	float					m_fElapsedTime;
};

ConstantBuffer<CB_PLAYER_INFO> gcbPlayerObjectInfo : register(b0);
ConstantBuffer<CB_CAMERA_INFO> gcbCameraInfo : register(b1);
ConstantBuffer<CB_GAMEOBJECT_INFO> gcbGameObjectInfo : register(b2);
ConstantBuffer< CB_BILLBOARD_INDEX> gcbTextureType : register(b3);
ConstantBuffer< CB_BILLBOARD_INDEX> gcbBombTexture : register(b4);
ConstantBuffer<CB_FRAMEWORK_INFO> gcbFrameInfo : register(b5);

#else
cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxWorld : packoffset(c0);
};
cbuffer cbTextureType : register(b3)
{
	uint		billType : packoffset(c0);
};
cbuffer cbBombTexture : register(b4)
{
	uint		bombIndex : packoffset(c0);
};

cbuffer cbFrameworkInfo : register(b5)
{
	float		gfCurrentTime : packoffset(c0.x);
	float		gfElapsedTime : packoffset(c0.y);
};
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSDiffused(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	output.position = mul(mul(mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
#endif
	output.color = input.color;

	return(output);
}

float4 PSDiffused(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	output.position = mul(mul(mul(float4(input.position, 1.0f), gcbPlayerObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxPlayerWorld), gmtxView), gmtxProjection);
#endif
	output.color = input.color;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTextures[6] : register(t0);

SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	output.position = mul(mul(mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
#endif
	output.uv = input.uv;

	return(output);
}

float4 PSTextured(VS_TEXTURED_OUTPUT input, uint nPrimitiveID : SV_PrimitiveID) : SV_TARGET
{
	float4 cColor = gtxtTextures[NonUniformResourceIndex(nPrimitiveID / 2)].Sample(gWrapSamplerState, input.uv);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTerrainBaseTexture : register(t6);
Texture2D gtxtTerrainDetailTexture : register(t7);

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	output.position = mul(mul(mul(float4(input.position, 1.0f), gcbGameObjectInfo.mtxWorld), gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxView), gmtxProjection);
#endif
	output.color = input.color;
	output.uv0 = input.uv0;
	output.uv1 = input.uv1;

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv0);
	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gWrapSamplerState, input.uv1);
	float4 cColor = input.color * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtSkyBox : register(t8);

float4 PSSkyBox(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyBox.Sample(gClampSamplerState, input.uv);

	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture2D gtxtBillboardTexture[2] : register(t9);

struct VS_IN {
	float3 posW : POSITION;
	float2 sizeW : SIZE;
};

struct VS_OUT {
	float3 centerW : POSITION;
	float2 sizeW : SIZE;
};

struct GS_OUT {
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
	uint primID : SV_PrimitiveID;
};

VS_OUT VS_Billboard(VS_IN input)
{
	VS_OUT output;
	output.centerW = input.posW;
	output.sizeW = input.sizeW;
	return output;
}

[maxvertexcount(4)]
void GS_Billboard(point VS_OUT input[1], uint primID : SV_PrimitiveID, inout TriangleStream<GS_OUT> outStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook;
#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	vLook = gcbCameraInfo.position.xyz - input[0].centerW;
#else
	vLook = gvCameraPosition.xyz - input[0].centerW;
#endif
	vLook = normalize(vLook);
	float3 vRight = cross(vUp, vLook);
	float fHalfW = input[0].sizeW.x * 0.5f;
	float fHalfH = input[0].sizeW.y * 0.5f;
	float4 pVertices[4];
	pVertices[0] = float4(input[0].centerW + fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[1] = float4(input[0].centerW + fHalfW * vRight + fHalfH * vUp, 1.0f);
	pVertices[2] = float4(input[0].centerW - fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[3] = float4(input[0].centerW - fHalfW * vRight + fHalfH * vUp, 1.0f);
	float2 pUVs[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };
	GS_OUT output;
	for (int i = 0; i < 4; ++i)
	{
		output.posW = pVertices[i].xyz;
#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
		output.posH = mul(mul(pVertices[i], gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
#else
		output.posH = mul(mul(pVertices[i], gmtxView), gmtxProjection);
#endif
		output.normalW = vLook;
		output.uv = pUVs[i];
		output.primID = primID;
		outStream.Append(output);
	}
	outStream.RestartStrip();
}

float4 PS_Billboard(GS_OUT input) : SV_TARGET
{
	float4 cColor;
#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	cColor = gtxtBillboardTexture[gcbTextureType.type].Sample(gWrapSamplerState, input.uv);
#else
	cColor = gtxtBillboardTexture[billType].Sample(gWrapSamplerState, input.uv);
#endif
	clip(cColor.a - 0.15f);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Texture2D gtxtBombTexture[21] : register(t11);

float4 PSBullet(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor;
#ifdef _WITH_CONSTANT_BUFFER_SYNTAX
	cColor = gtxtBombTexture[gcbBombTexture.type].Sample(gWrapSamplerState, input.uv);
#else
	cColor = gtxtBombTexture[bombIndex].Sample(gWrapSamplerState, input.uv);
#endif
	clip(cColor.a - 0.15f);
	return(cColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define PARTICLE_TYPE_EMITTER	0
#define PARTICLE_TYPE_FLARE		0x0ff


struct VS_PARTICLE_INPUT
{
	float3 position : POSITION;
	float3 velocity : VELOCITY;
	float2 size : SIZE;
	float age : LIFETIME;
	uint type : PARTICLETYPE;
};

VS_PARTICLE_INPUT VSParticleStreamOutput(VS_PARTICLE_INPUT input)
{
	return(input);
}

Texture2D gtxtParticleTexture : register(t32);

float3 GetParticleColor(float fAge, float fLifetime)
{
	float3 cColor = float3(1.0f, 1.0f, 1.0f);

	if (fAge == 0.0f) cColor = float3(0.0f, 1.0f, 0.0f);
	else if (fLifetime == 0.0f) 
		cColor = float3(1.0f, 1.0f, 0.0f);
	else
	{
		float t = fAge / fLifetime;
		cColor = lerp(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), t * 1.0f);
	}

	return(cColor);
}

void GetBillboardCorners(float3 position, float2 size, out float4 pf4Positions[4])
{
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = normalize(gcbCameraInfo.position - position);
	float3 f3Right = normalize(cross(f3Up, f3Look));

	pf4Positions[0] = float4(position + size.x * f3Right - size.y * f3Up, 1.0f);
	pf4Positions[1] = float4(position + size.x * f3Right + size.y * f3Up, 1.0f);
	pf4Positions[2] = float4(position - size.x * f3Right - size.y * f3Up, 1.0f);
	pf4Positions[3] = float4(position - size.x * f3Right + size.y * f3Up, 1.0f);
}

void GetPositions(float3 position, float2 f2Size, out float3 pf3Positions[8])
{
	float3 f3Right = float3(1.0f, 0.0f, 0.0f);
	float3 f3Up = float3(0.0f, 1.0f, 0.0f);
	float3 f3Look = float3(0.0f, 0.0f, 1.0f);

	float3 f3Extent = normalize(float3(1.0f, 1.0f, 1.0f));

	pf3Positions[0] = position + float3(-f2Size.x, 0.0f, -f2Size.y);
	pf3Positions[1] = position + float3(-f2Size.x, 0.0f, +f2Size.y);
	pf3Positions[2] = position + float3(+f2Size.x, 0.0f, -f2Size.y);
	pf3Positions[3] = position + float3(+f2Size.x, 0.0f, +f2Size.y);
	pf3Positions[4] = position + float3(-f2Size.x, 0.0f, 0.0f);
	pf3Positions[5] = position + float3(+f2Size.x, 0.0f, 0.0f);
	pf3Positions[6] = position + float3(0.0f, 0.0f, +f2Size.y);
	pf3Positions[7] = position + float3(0.0f, 0.0f, -f2Size.y);
}

[maxvertexcount(9)]
void GSParticleStreamOutput(point VS_PARTICLE_INPUT input[1], inout PointStream<VS_PARTICLE_INPUT> output)
{
	VS_PARTICLE_INPUT particle = input[0];

	particle.age.x += gcbFrameInfo.m_fElapsedTime;
	if (particle.age.x <= particle.age.y)
	{
		if (particle.type == PARTICLE_TYPE_EMITTER)
		{
			particle.color = float3(1.0f, 0.0f, 0.0f);
//			particle.age.x = 0.0f;

			output.Append(particle);
			
			float4 f4Random = gRandomBuffer.Load(int(fmod(gcbFrameInfo.m_fCurrentTime - floor(gcbFrameInfo.m_fCurrentTime) * 1000.0f, 1000.0f)));

			float3 pf3Positions[8];
			GetPositions(particle.position, float2(particle.size.x * 1.25f, particle.size.x * 1.25f), pf3Positions);

			particle.color = float3(0.0f, 0.0f, 1.0f);
			particle.age.x = 0.0f;

			for (int j = 0; j < 8; j++)
			{
				particle.type = (j >= 4) ? PARTICLE_TYPE_EMITTER : PARTICLE_TYPE_FLARE;
				particle.position = pf3Positions[j].xyz;
				particle.velocity = float3(0.0f, particle.size.x * particle.age.y * 4.0f, 0.0f) * 2.0f;
				particle.acceleration = float3(0.0f, 250.125f, 0.0f) * abs(f4Random.x);
				particle.age.y = (particle.type == PARTICLE_TYPE_EMITTER) ? 0.25f : 1.5f + (abs(f4Random.w) * 0.75f * abs(j - 4));

				output.Append(particle);
			}
		}
		else
		{
			particle.color = GetParticleColor(particle.age.x, particle.age.y);
			particle.position += (0.5f * particle.acceleration * gcbFrameInfo.m_fElapsedTime * gcbFrameInfo.m_fElapsedTime) + (particle.velocity * gcbFrameInfo.m_fElapsedTime);

			output.Append(particle);
		}
	}
}

VS_PARTICLE_INPUT VSParticleDraw(VS_PARTICLE_INPUT input)
{
	return(input);
}

struct GS_PARTICLE_OUTPUT
{
	float4 position : SV_Position;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
	float age : LIFETIME;
	uint type : PARTICLETYPE;
};

static float2 gf2QuadUVs[4] = { float2(0.0f,1.0f), float2(0.0f,0.0f), float2(1.0f,1.0f), float2(1.0f,0.0f) };

[maxvertexcount(4)]
void GSParticleDraw(point VS_PARTICLE_INPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> outputStream)
{
	float4 pVertices[4];
	//GetBillboardCorners(input[0].position, input[0].size * 0.5f, pVertices);
	GetBillboardCorners(mul(float4(input[0].position, 1.0f), gcbGameObjectInfo.mtxWorld).xyz, input[0].size * 0.5f, pVertices);

	GS_PARTICLE_OUTPUT output = (GS_PARTICLE_OUTPUT)0;
	output.color = input[0].color;
	output.age = input[0].age;
	output.type = input[0].type;
	for (int i = 0; i < 4; i++)
	{
		output.position = mul(mul(pVertices[i], gcbCameraInfo.mtxView), gcbCameraInfo.mtxProjection);
		output.uv = gf2QuadUVs[i];

		outputStream.Append(output);
	}
}

float4 PSParticleDraw(GS_PARTICLE_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtParticleTexture.Sample(gWrapSamplerState, input.uv);
	clip(cColor.a - 0.15f);
	return(cColor); 
}