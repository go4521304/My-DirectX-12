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

ConstantBuffer<CB_PLAYER_INFO> gcbPlayerObjectInfo : register(b0);
ConstantBuffer<CB_CAMERA_INFO> gcbCameraInfo : register(b1);
ConstantBuffer<CB_GAMEOBJECT_INFO> gcbGameObjectInfo : register(b2);
ConstantBuffer< CB_BILLBOARD_INDEX> gcbTextureType : register(b3);
ConstantBuffer< CB_BILLBOARD_INDEX> gcbBombTexture : register(b4);


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
	/*
		float4 cColor;
		if (nPrimitiveID < 2)
			cColor = gtxtTextures[0].Sample(gWrapSamplerState, input.uv);
		else if (nPrimitiveID < 4)
			cColor = gtxtTextures[1].Sample(gWrapSamplerState, input.uv);
		else if (nPrimitiveID < 6)
			cColor = gtxtTextures[2].Sample(gWrapSamplerState, input.uv);
		else if (nPrimitiveID < 8)
			cColor = gtxtTextures[3].Sample(gWrapSamplerState, input.uv);
		else if (nPrimitiveID < 10)
			cColor = gtxtTextures[4].Sample(gWrapSamplerState, input.uv);
		else
			cColor = gtxtTextures[5].Sample(gWrapSamplerState, input.uv);
	*/
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
	uint typeID : BILL_TYPE;
};

struct VS_OUT {
	float3 centerW : POSITION;
	float2 sizeW : SIZE;
	uint typeID : BILL_TYPE;
};

struct GS_OUT {
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
	uint typeID : BILL_TYPE;
};

VS_OUT VS_Billboard(VS_IN input)
{
	VS_OUT output;
	output.centerW = input.posW;
	output.sizeW = input.sizeW;
	output.typeID = input.typeID;
	return output;
}

[maxvertexcount(4)]
void GS_Billboard(point VS_OUT input[1], inout TriangleStream<GS_OUT> outStream)
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
		output.typeID = input[0].typeID;
		outStream.Append(output);
	}
	outStream.RestartStrip();
}

float4 PS_Billboard(GS_OUT input) : SV_TARGET
{
	float4 cColor;
	cColor = gtxtBillboardTexture[input.typeID].Sample(gWrapSamplerState, input.uv);
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