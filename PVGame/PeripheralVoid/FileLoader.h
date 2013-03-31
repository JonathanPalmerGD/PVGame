#pragma once

/*
#include <Windows.h>
#include <vector>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "Constants.h"


class FileLoader
{
	
//These structs were gathered from : http://www.braynzarsoft.net/Code/index.php?p=VC&code=Obj-Model-Loader
struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT4 color;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;  //BUMP MAPPING!
};

public:
	FileLoader(void);
	~FileLoader(void);

	bool loadFile(ID3D11Device* device, 
	std::wstring Filename, 
    ObjModel& Model,
    std::vector<SurfaceMaterial>& material, 
    TextureManager& TexMgr,
    bool IsRHCoordSys,
    bool ComputeNormals,
    bool flipFaces);
};

*/