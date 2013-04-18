#pragma once

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
//The code for this OBJLoader was taken from : http://www.braynzarsoft.net/Code/index.php?p=VC&code=Obj-Model-Loader

class FileLoader
{

public:
	FileLoader(void);
	~FileLoader(void);

	bool LoadFile(ID3D11Device* device, 
	std::wstring Filename, 
    ObjModel& Model,
    std::vector<GameMaterial>& material, 
    TextureManager& TexMgr,
    bool IsRHCoordSys,
    bool ComputeNormals,
	bool flipFaces);
};
