//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
#define MAX_LIGHTS 10
#define MAX_GLOW_RANGE 27 // How many "units" away from the eye an object must be to achieve full glow.
#define MAX_ENVIRONMENTS 1 // How many different texture atlas's we are using.
#define GLOW_RANGE_POWER (0.000000000)
#define GLOW_ANGLE_POWER (1.000000000)

cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	PointLight testLights[MAX_LIGHTS];
	PointLight gPointLight;
	float3 gEyePosW;
	float4 gBlurColor;

	float gTexelWidth;
	float gTexelHeight;
	float2 gScreenSize;

	// Rift variables.
	float2 LensCenter;
	float2 ScreenCenter;
	float2 Scale;
	float2 ScaleIn;
	float4 HmdWarpParam;
	float4 ChromAbParam;
};

cbuffer cbSettings
{
	float gWeights[21] = 
	{
		 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.75f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 2;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gViewProj;
	float4x4 gTexTransform;
	float4x4 gWorldViewProj;
	float4x4 gWorldInvTranspose;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
Texture2D environmentAtlas[MAX_ENVIRONMENTS];

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState Linear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VertexIn
{
	float3 PosL					: POSITION;
	float3 NormalL				: NORMAL;
	float2 Tex					: TEXCOORD;
	row_major float4x4 World	: WORLD;
	Material Material			: MATERIAL;
	uint InstanceId				: SV_InstanceID;
	float4 GlowColor			: GLOWCOLOR;
	float4 TexScale				: TEXSCALE;
	float2 AtlasCoord			: ATLASCOORD;
};

struct VertexOut
{
	float4 PosH			: SV_POSITION;
	float4 GlowColor	: GLOWCOLOR;
	Material Material	: MATERIAL;
	float2 Tex			: TEXCOORD;
	float2 AtlasCoord	: ATLASCOORD;
	float AtlasIndex	: ATLASINDEX;
    float3 PosW			: POSITION;
    float3 NormalW		: NORMAL;
};

struct BlurVertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex	: TEXCOORD;
};

VertexOut VS(VertexIn vin, uniform bool isUsingAtlas)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), vin.World).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)vin.World);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex   = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	if (vin.TexScale.w == 1.0f)
	{
		vout.Tex.x *= vin.TexScale.x;
		vout.Tex.y *= vin.TexScale.y;
	}
	vout.Material = vin.Material;
	vout.AtlasCoord = vin.AtlasCoord;
	vout.GlowColor = vin.GlowColor;

	if (isUsingAtlas)
		vout.AtlasIndex = vin.TexScale.z;
	return vout;
}

VertexOut TextureVS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW    = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorld);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	vout.Material = vin.Material;
	vout.AtlasCoord = vin.AtlasCoord;

	return vout;
}

BlurVertexOut BlurVS(VertexIn vin)
{
	BlurVertexOut vout;

	// Already in normalized device space.
	vout.PosH = float4(vin.PosL, 1.0f);

	// Pass onto pixel shader.
	vout.Tex = vin.Tex;

	return vout;
}

float4x4 gOcView;
VertexOut OculusVS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gOcView);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	//vout.PosH = mul(gOcView, float4(vin.PosL, 1.0f));
	//vout.PosW = mul(gOcView, float4(vin.PosL, 1.0f));
	//vout.Tex = mul(gTexTransform, float4(vin.Tex, 0, 1)).xy;

	vout.Material = vin.Material;
	vout.AtlasCoord = vin.AtlasCoord;

	return vout;
}

float4 PS(VertexOut pin, uniform bool gUseTexure) : SV_Target
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
	//fixed4 tex = tex2D(_MainTex, (frac(IN.uv_MainTex) * 0.25f) + offset);
    if(gUseTexure)
	{
		// Sample texture.
		//float2 texCoord = (frac(pin.Tex) * 0.25f) + (pin.AtlasCoord * 0.25f); // Original
		float2 texCoord = (frac(pin.Tex) * 0.125f) + (pin.AtlasCoord * 0.25f) + float2(0.0625f, 0.0625f);
		texColor = environmentAtlas[pin.AtlasIndex].Sample( samAnisotropic, texCoord);
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
	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		if (testLights[i].On.x == 1)
		{
			float4 A, D, S;
			ComputePointLight(pin.Material, testLights[i], pin.PosW, pin.NormalW, toEye, A, D, S);

			ambient += A;
			diffuse += D;
			spec    += S;
		}
	}

	float4 A, D, S;
	ComputeDirectionalLight(pin.Material, gDirLights[0], pin.NormalW, toEye, 
				A, D, S);

	ambient += A;
	diffuse += D;
	spec    += S;

	// Modulate with late add.
	litColor = texColor*(ambient + diffuse) + spec;

	distToEye = clamp(distToEye, 0.1f, MAX_GLOW_RANGE);

	// Only glow if certain conditions met. Position check is to discard pixels not on screen.
	if (pin.GlowColor[3] > 0.0f && pin.PosH.x >= 0 && pin.PosH.x <= gScreenSize.x && pin.PosH.y >= 0 && pin.PosH.y <= gScreenSize.y)
	{
		litColor += (pin.GlowColor * GLOW_RANGE_POWER / (MAX_GLOW_RANGE / distToEye));

		float halfWidth = gScreenSize.x / 2;
		float halfHeight = gScreenSize.y / 2;
		float xDif = pin.PosH.x - halfWidth;
		float yDif = pin.PosH.y - halfHeight;
		float dist = xDif * xDif + yDif * yDif;
		float range = halfWidth * halfWidth + halfHeight * halfHeight;

		if ( dist <= range )
		{
			litColor += (pin.GlowColor * GLOW_ANGLE_POWER * dist / range);
		}
	}
	
	// Common to take alpha from diffuse material and texture.
	litColor.a = pin.Material.Diffuse.a * texColor.a;

    return litColor;
}

float4 TexturePS(VertexOut pin, uniform bool drawCursor) : SV_Target
{
	if (drawCursor && pin.Tex.x >= 0.495f && pin.Tex.x <= 0.505f
				&& pin.Tex.y >= 0.495f && pin.Tex.y <= 0.505f)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);

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

	// Sample texture.
	texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );
	 
	float4 litColor = texColor;

	// Common to take alpha from diffuse material and texture.
	litColor.a = 0.5f * texColor.a;

    return litColor;
}

float4 BlurPS(BlurVertexOut pin, uniform bool gHorizontalBlur) : SV_TARGET
{
	float2 texOffset;
	if(gHorizontalBlur)
	{
		texOffset = float2(gTexelWidth, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gTexelHeight);
	}

	// The center value always contributes to the sum.
	float4 color = gWeights[10] * gDiffuseMap.SampleLevel(samInputImage, pin.Tex, 0.0f);
	float totalWeight = gWeights[10];

	float2 texAboutOrigin = float2(pin.Tex.x - 0.5f, pin.Tex.y - 0.5f);

	//if ( texAboutOrigin.x * texAboutOrigin.x + texAboutOrigin.y * texAboutOrigin.y > 0.05f )
	//{
		
	//}

	for (float i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		// We aklready added in the center weight.
		if (i == 0)
			continue;

		float2 tex = pin.Tex + i * texOffset;

		float weight = gWeights[i + gBlurRadius];

		// Add neighbor pixel to blur.
		color += weight * gDiffuseMap.SampleLevel(samInputImage, tex, 0.0f);

		totalWeight += weight;
	}

		color += gBlurColor;
	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
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
technique11 LightsWithAtlas
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS(true) ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}

technique11 LightsWithoutAtlas
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS(false) ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}

technique11 TexturePassThrough
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, TexturePS(false) ) );
	}
}

technique11 TexturePassThroughWithCursor
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, TexturePS(true) ) );
	}
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BlurPS(true) ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BlurPS(false) ) );
    }
}

technique11 Blur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BlurPS(false) ) );
    }

	pass P1
    {
		SetVertexShader( CompileShader( vs_5_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, BlurPS(true) ) );
    }
}

technique11 OculusTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, OculusVS() ) );
		//SetVertexShader( CompileShader( vs_5_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, OculusPS() ) );
    }
}

#pragma endregion

#pragma region DX10 Techniques
technique11 LightsWithAtlasDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS(true) ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(true) ) );
    }
}

technique11 LightsWithoutAtlasDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS(false) ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS(true) ) );
    }
}

technique11 TexturePassThroughDX10
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, TexturePS(false) ) );
	}
}

technique11 TexturePassThroughWithCursorDX10
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, TexturePS(true) ) );
	}
}

technique11 HorzBlurDX10
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, BlurPS(true) ) );
    }
}

technique11 VertBlurDX10
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, BlurPS(false) ) );
    }
}
 
technique11 BlurDX10
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_4_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, BlurPS(false) ) );
    }

	pass P1
    {
		SetVertexShader( CompileShader( vs_4_0, BlurVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, BlurPS(true) ) );
    }
}

technique11 OculusTechDX10
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, OculusVS() ) );
		//SetVertexShader( CompileShader( vs_4_0, TextureVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, OculusPS() ) );
    }
}
#pragma endregion