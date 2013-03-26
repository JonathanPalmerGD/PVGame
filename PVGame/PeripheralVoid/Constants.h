#pragma once

#include <map>
#include <vector>
#include "Common\d3dUtil.h"
#include "Common\LightHelper.h"


using std::vector;
using std::map;
using std::string;

#define NUM_LEVELS 1

#define USINGVLD 0
#if USINGVLD 
#include <vld.h>
#endif

const enum GAME_STATE { MENU, OPTION, PLAYING, END };

const float TARGET_FPS = 1000.0f/60.0f; //in milliseconds

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;

	//XMFLOAT3 Tangent;
	//XMFLOAT3 biTangent;  //BUMP MAPPING!
};

struct MeshData
{
	vector<Vertex> vertices;
	vector<UINT> indices;
	string bufferKey;
};

struct BufferPair
{
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;
};

struct MeshMaps
{
    static map<string, MeshData> create_map()
    {
		map<string, MeshData> m;
		// Statically defining first triangle to just get it to render.
		XMFLOAT3 a (2.0f, 1.0f, 10.0f); // Length = 105.
		XMFLOAT3 b (1.0f, 0.0f, 10.0f); // Length = 101.
		XMFLOAT3 c (0.0f, 1.0f, 10.0f); // Length = 101.

		XMFLOAT3 ua (a.x / 105.0f, a.y / 105.0f, a.z / 105.0f);
		XMFLOAT3 ub (b.x / 101.0f, b.y / 101.0f, b.z / 101.0f);
		XMFLOAT3 uc (c.x / 101.0f, c.y / 101.0f, c.z / 101.0f);

		Vertex triangleVertices[] =
		{
			// First triangle
			{ a, ua },
			{ b, ub },
			{ c, uc }
		};
		for (int i = 0; i < 3; i++)
		{
			m["Triangle"].vertices.push_back(triangleVertices[i]);
			m["Triangle"].indices.push_back(i);
		}

		m["Triangle"].bufferKey = "Triangle";

		// Create cube.
		Vertex vertices[] =
		{
			// Normals derived by hand - will want to be more efficient later.
			{ XMFLOAT3(-0.5, -0.5, -0.5), XMFLOAT3(-0.5773502691896258, -0.5773502691896258, -0.5773502691896258)	},
			{ XMFLOAT3(-0.5, +0.5, -0.5), XMFLOAT3(-0.5773502691896258, 0.5773502691896258, -0.5773502691896258)	},
			{ XMFLOAT3(+0.5, +0.5, -0.5), XMFLOAT3(0.5773502691896258, 0.5773502691896258, -0.5773502691896258)    },
			{ XMFLOAT3(+0.5, -0.5, -0.5), XMFLOAT3(0.5773502691896258, -0.5773502691896258, -0.5773502691896258)	},
			{ XMFLOAT3(-0.5, -0.5, +0.5), XMFLOAT3(-0.5773502691896258, -0.5773502691896258, 0.5773502691896258)    },
			{ XMFLOAT3(-0.5, +0.5, +0.5), XMFLOAT3(-0.5773502691896258, 0.5773502691896258, 0.5773502691896258)	},
			{ XMFLOAT3(+0.5, +0.5, +0.5), XMFLOAT3(0.5773502691896258, 0.5773502691896258, 0.5773502691896258)    },
			{ XMFLOAT3(+0.5, -0.5, +0.5), XMFLOAT3(0.5773502691896258, -0.5773502691896258, 0.5773502691896258)	}
		};
		m["Cube"].vertices.assign(vertices, vertices + 8);

		// Create the index buffer
		UINT indices[] = 
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3, 
			4, 3, 7
		};
		m["Cube"].indices.assign(indices, indices + 36);
		m["Cube"].bufferKey = "Cube";

		// Create Plane.
		Vertex planeVertices[] =
		{
			// Normals derived by hand - will want to be more efficient later.
			{ XMFLOAT3(-0.5, -0.0, 0.5), XMFLOAT3(0.0f, 1.0f, 0.0f)	},	//-1, 1
			{ XMFLOAT3(0.5, -0.0, 0.5), XMFLOAT3(0.0f, 1.0f, 0.0f)    },	//1, 1
			{ XMFLOAT3(0.5, -0.0, -0.5), XMFLOAT3(0.0f, 1.0f, 0.0f)	},	//1, -1
			{ XMFLOAT3(-0.5, -0.0, -0.5), XMFLOAT3(0.0f, 1.0f, 0.0f)	}	//-1,-1
		};
		m["Plane"].vertices.assign(planeVertices, planeVertices + 4);

		// Create the index buffer
		UINT planeIndices[] = 
		{
			// front face
			0, 1, 2,
			0, 2, 3
		};
		m["Plane"].indices.assign(planeIndices, planeIndices + 6);
		m["Plane"].bufferKey = "Plane";

		return m;
    }
    static const map<string, MeshData> MESH_MAPS;
};

//These structs were gathered from : http://www.braynzarsoft.net/Code/index.php?p=VC&code=Obj-Model-Loader
// Create material structure
struct SurfaceMaterial
{
    // Surface's colors
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;      
    XMFLOAT4 Specular;      // Specular power stored in 4th component
	XMFLOAT4 Reflect;

	/*
    // Texture ID's to look up texture in SRV array
    int DiffuseTextureID;
    int AmbientTextureID;
    int SpecularTextureID;
    int AlphaTextureID;
    int NormMapTextureID;

    // Booleans so we don't implement techniques we don't need
    bool HasDiffTexture;
    bool HasAmbientTexture;
    bool HasSpecularTexture;
    bool HasAlphaTexture;
    bool HasNormMap;
    bool IsTransparent;
	*/
};


//List our levels:
const char MAP_LEVEL_1[] = "Assets/level1.xml";

/*

// Model Structure
struct ObjModel
{
    int Subsets;                        // Number of subsets in obj model
    ID3D11Buffer* VertBuff;             // Models vertex buffer
    ID3D11Buffer* IndexBuff;            // Models index buffer
    std::vector<XMFLOAT3> Vertices;     // Models vertex positions list
    std::vector<DWORD> Indices;         // Models index list
    std::vector<int> SubsetIndexStart;  // Subset's index offset
    std::vector<int> SubsetMaterialID;  // Lookup ID for subsets surface material
    XMMATRIX World;                     // Models world matrix
    std::vector<XMFLOAT3> AABB;         // Stores models AABB (min vertex, max vertex, and center)
                                        // Where AABB[0] is the min Vertex, and AABB[1] is the max vertex
    XMFLOAT3 Center;                    // True center of the model
    float BoundingSphere;               // Model's bounding sphere
	~ObjModel() { VertBuff->Release(); IndexBuff->Release(); }
};

// Texture manager structure, So all the textures are nice and grouped together
struct TextureManager
{
    std::vector<ID3D11ShaderResourceView*> TextureList;
    std::vector<std::wstring> TextureNameArray;     // So we don't load in the same texture twice
};

*/