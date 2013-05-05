//***************************************************************************************
// Sky.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Sky.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "../Constants.h"

const D3D11_INPUT_ELEMENT_DESC Pos[1] = 
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

Sky::Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius, bool usingDX11)
{
	std::ifstream fin("fx/Sky.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &mFX));
	SkyTech = (usingDX11 ? mFX->GetTechniqueByName("SkyTech") : mFX->GetTechniqueByName("SkyTechDX10"));
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	HR(D3DX11CreateShaderResourceViewFromFile(device, cubemapFilename.c_str(), 0, 0, &mCubeMapSRV, 0));

	GeometryGenerator::MeshData sphere;
	GeometryGenerator geoGen;
	geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

	std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

	for(size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i] = sphere.Vertices[i].Position;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];

    HR(device->CreateBuffer(&vbd, &vinitData, &mVB));
	
	mIndexCount = sphere.Indices.size();

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * mIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;

	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices16[0];

    HR(device->CreateBuffer(&ibd, &iinitData, &mIB));

	D3DX11_PASS_DESC passDesc;
	SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(Pos, 1, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &posLayout));
}

Sky::~Sky()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mFX);
	ReleaseCOM(posLayout);
	ReleaseCOM(mCubeMapSRV);
}

ID3D11ShaderResourceView* Sky::CubeMapSRV()
{
	return mCubeMapSRV;
}

void Sky::Draw(ID3D11DeviceContext* dc, const Camera& camera)
{
	// center Sky about eye in world space
	XMFLOAT3 eyePos = camera.GetPosition();
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	XMMATRIX WVP = XMMatrixMultiply(T, camera.ViewProj());

	mfxWorldViewProj->SetMatrix(reinterpret_cast<const float*>(&WVP));
	CubeMap->SetResource(mCubeMapSRV);
	//Effects::SkyFX->SetWorldViewProj(WVP);
	//Effects::SkyFX->SetCubeMap(mCubeMapSRV);

	UINT stride = sizeof(XMFLOAT3);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	dc->IASetIndexBuffer(mIB, DXGI_FORMAT_R16_UINT, 0);
	dc->IASetInputLayout(posLayout);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	D3DX11_TECHNIQUE_DESC techDesc;
    SkyTech->GetDesc( &techDesc );

    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        ID3DX11EffectPass* pass = SkyTech->GetPassByIndex(p);
		pass->Apply(0, dc);
		dc->DrawIndexed(mIndexCount, 0, 0);
	}
}