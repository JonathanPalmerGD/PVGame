//***************************************************************************************
// Sky.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple class that renders a sky using a cube map.
//***************************************************************************************

#ifndef SKY_H
#define SKY_H

#include "d3dUtil.h"

class Camera;

class Sky
{
public:
	Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius, bool usingDX11);
	~Sky();

	ID3D11ShaderResourceView* CubeMapSRV();
	
	void Draw(ID3D11DeviceContext* dc, const Camera& camera, const float* viewproj);
	void Draw(ID3D11DeviceContext* dc, const Camera& camera);

private:
	Sky(const Sky& rhs);
	Sky& operator=(const Sky& rhs);

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	ID3DX11Effect* mFX;
	ID3D11InputLayout* posLayout;
	ID3DX11EffectTechnique* SkyTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3D11ShaderResourceView* mCubeMapSRV;

	UINT mIndexCount;
};

#endif // SKY_H