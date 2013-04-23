//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
#define MAX_LIGHTS 10
#define MAX_GLOW_RANGE 27 // How many "units" away from the eye an object must be to achieve full glow.
#define GLOW_RANGE_POWER (0.333333333)
#define GLOW_ANGLE_POWER (0.666666666)

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

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
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
	float2 AtlasCoord			: ATLASCOORD;
	float4 GlowColor			: GLOWCOLOR;
};

struct VertexOut
{
	float4 PosH			: SV_POSITION;
    float3 PosW			: POSITION;
    float3 NormalW		: NORMAL;
	float2 Tex			: TEXCOORD;
	Material Material	: MATERIAL;
	float2 AtlasCoord	: ATLASCOORD;
	float4 GlowColor	: GLOWCOLOR;
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
	float2 texCoord = (isUsingAtlas) ? float2((vin.Tex.x / 2.0f) + (1.0f / 2.0f * vin.AtlasCoord.x),
							(vin.Tex.y / 2.0f) + (1.0f / 2.0f * vin.AtlasCoord.y))
							: vin.Tex;

	// Output vertex attributes for interpolation across triangle.
	vout.Tex   = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	vout.Material = vin.Material;
	vout.AtlasCoord = vin.AtlasCoord;
	vout.GlowColor = vin.GlowColor;
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
		texColor = gDiffuseMap.Sample( samAnisotropic, (frac(pin.Tex) * 0.5f) + (pin.AtlasCoord * 0.5f));
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

float4 TexturePS(VertexOut pin) : SV_Target
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

	if ( texAboutOrigin.x * texAboutOrigin.x + texAboutOrigin.y * texAboutOrigin.y > 0.05f )
	{
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
	}

	
	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
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
        SetPixelShader( CompileShader( ps_5_0, TexturePS() ) );
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
        SetPixelShader( CompileShader( ps_4_0, TexturePS() ) );
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

#pragma endregion