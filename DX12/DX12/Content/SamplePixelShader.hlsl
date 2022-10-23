cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};


struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 color = {view._m00, view._m01, view._m02};
	

	return float4(color, 1.0f);
}
