//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	PointLight testLights[4];
	PointLight gPointLight;
	float3 gEyePosW;

	// Rift variables.
	float2 LensCenter;
	float2 ScreenCenter;
	float2 Scale;
	float2 ScaleIn;
	float4 HmdWarpParam;
	float4 ChromAbParam;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState Linear : register(s0);

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye); 

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	}
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{  
		// Start with a sum of zero. 
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]

		float4 A, D, S;
		for(int i = 0; i < gLightCount; ++i)
		{
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, 
				A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}

		ComputePointLight(gMaterial, gPointLight, pin.PosW, pin.NormalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		spec    += S;

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

float4 TestPS(VertexOut pin, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye); 

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	}
	 
	//
	// Lighting.
	//
	float4 litColor = texColor;

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	
	// Sum the light contribution from each light source.  
	[unroll]
	for(int i = 0; i < 4; ++i)
	{
		if (testLights[i].On.x == 1)
		{
			float4 A, D, S;
			ComputePointLight(gMaterial, testLights[i], pin.PosW, pin.NormalW, toEye, A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}
	}

	float4 A, D, S;
	ComputeDirectionalLight(gMaterial, gDirLights[0], pin.NormalW, toEye, 
				A, D, S);

	ambient += A;
	diffuse += D;
	spec    += S;

	// Modulate with late add.
	litColor = texColor*(ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

float4 OculusPS(VertexOut pin) : SV_Target
{
	float2 theta = (pin.Tex - LensCenter) * ScaleIn; // Scales to [-1, 1]
	float rSq = theta.x * theta.x + theta.y * theta.y;
	float2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
	
	// Detect whether blue texture coordinates are out of range
	// since these will scaled out the furthest.
	float2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);
	float2 tcBlue = LensCenter + Scale * thetaBlue;
	if (any(clamp(tcBlue, ScreenCenter -float2(0.25, 0.5), ScreenCenter+float2(0.25, 0.5)) - tcBlue))
		return 0;

	// Now do blue texture lookup.
	float blue = gDiffuseMap.Sample(Linear, tcBlue).b;

	// Do green lookup (no scaling).
	float2 tcGreen = LensCenter + Scale * theta1;
	float green = gDiffuseMap.Sample(Linear, tcGreen).g;
	// Do red scale and lookup.
	float2 thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);
	float2 tcRed = LensCenter + Scale * thetaRed;
	float red = gDiffuseMap.Sample(Linear, tcRed).r;

	return float4(red, green, blue, 1);
}

#pragma region DX11 Techniques
technique11 Light1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(1, false) ) );
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false) ) );
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true) ) );
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(1, true) ) );
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(2, true) ) );
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true) ) );
    }
}

technique11 TestLights
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, TestPS(true) ) );
    }
}

technique11 OculusTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, OculusPS() ) );
    }
}

#pragma endregion

#pragma region DX10 Techniques
technique11 Light1DX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(1, false) ) );
    }
}

technique11 Light2DX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(2, false) ) );
    }
}

technique11 Light3DX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(3, false) ) );
    }
}

technique11 Light0TexDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(0, true) ) );
    }
}

technique11 Light1TexDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(1, true) ) );
    }
}

technique11 Light2TexDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(2, true) ) );
    }
}

technique11 Light3TexDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(3, true) ) );
    }
}

technique11 TestLightsDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, TestPS(true) ) );
    }
}

technique11 OculusTechDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, OculusPS() ) );
    }
}
#pragma endregion