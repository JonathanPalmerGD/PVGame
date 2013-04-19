#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include "Common\d3dUtil.h"
#include "Common\LightHelper.h"
#include "Common\GeometryGenerator.h"

using std::vector;
using std::map;
using std::string;

#define USE_FRUSTUM_CULLING 1
#define DRAW_FRUSTUM 0 //Only Make 1 if USE_FRUSTUM_CULLING is 1
#define MOBILITY_MULTIPLIER 1.25f

#define USINGVLD 1
#if USINGVLD 
#include <vld.h>
#endif

#define MAX_LIGHTS 10
#define MAX_BLURS 2

const enum GAME_STATE { MENU, OPTION, PLAYING, END };
const enum CREST_TYPE { MEDUSA, MOBILITY, LEAP, UNLOCK};
const enum TURRET_TYPE { ALPHA, BETA, GAMMA };

const float TARGET_FPS = 1000.0f/60.0f; //in milliseconds

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
	XMFLOAT3 Tangent;
	//XMFLOAT3 biTangent;  //BUMP MAPPING!
};

struct MeshData
{
	vector<Vertex> vertices;
	vector<UINT> indices;
	string bufferKey;
	bool normalizeVertices;
};

struct BufferPair
{
	ID3D11Buffer* indexBuffer;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* instanceBuffer;
};

struct MeshMaps
{
    static map<string, MeshData> create_map()
    {
		map<string, MeshData> m;
		// Create cube.
		
		GeometryGenerator aGeometryGenerator;

		GeometryGenerator::MeshData quadMeshData;
		aGeometryGenerator.CreateFullscreenQuad(quadMeshData);

		m["Quad"].bufferKey = "Quad";
		m["Quad"].normalizeVertices = false;
		m["Quad"].indices = quadMeshData.Indices;

		for(UINT i = 0; i < quadMeshData.Vertices.size(); ++i)
		{
			Vertex aVertex;
			aVertex.Pos    = quadMeshData.Vertices[i].Position;
			aVertex.Normal = quadMeshData.Vertices[i].Normal;
			aVertex.TexC    = quadMeshData.Vertices[i].TexC;
			m["Quad"].vertices.push_back(aVertex);
		}

		GeometryGenerator::MeshData cubeMeshData;
		aGeometryGenerator.CreateBox(1.0f, 1.0f, 1.0f, cubeMeshData);
		m["Cube"].bufferKey = "Cube";
		m["Cube"].normalizeVertices = false;
		m["Cube"].indices = cubeMeshData.Indices;
		for (UINT i = 0; i < cubeMeshData.Vertices.size(); i++)
		{
			Vertex aVertex;
			aVertex.Pos = cubeMeshData.Vertices[i].Position;
			aVertex.Normal = cubeMeshData.Vertices[i].Normal;
			aVertex.TexC = cubeMeshData.Vertices[i].TexC;
			m["Cube"].vertices.push_back(aVertex);
		}
	
		// Create Plane.
		Vertex planeVertices[] =
		{
			// Normals derived by hand - will want to be more efficient later.
			{ XMFLOAT3(-0.5, -0.0, +0.5), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)	},	//-1, 1
			{ XMFLOAT3(+0.5, -0.0, +0.5), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f)    },	//1, 1
			{ XMFLOAT3(+0.5, -0.0, -0.5), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f)	},	//1, -1
			{ XMFLOAT3(-0.5, -0.0, -0.5), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f)	}	//-1,-1
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
		m["Plane"].normalizeVertices = false;

		
		GeometryGenerator::MeshData sphereMeshData;
		aGeometryGenerator.CreateSphere(1.0f, 14, 14, sphereMeshData);
		m["Sphere"].bufferKey = "Sphere";
		m["Sphere"].normalizeVertices = true;
		m["Sphere"].indices = sphereMeshData.Indices;
		for (UINT i = 0; i < sphereMeshData.Vertices.size(); i++)
		{
			Vertex aVertex;
			aVertex.Pos = sphereMeshData.Vertices[i].Position;
			aVertex.Normal = sphereMeshData.Vertices[i].Normal;
			aVertex.TexC = sphereMeshData.Vertices[i].TexC;
			m["Sphere"].vertices.push_back(aVertex);
		}
		

		#define AVERAGE_NORMALS 1
		#if AVERAGE_NORMALS
			map<string, MeshData>::iterator itr = m.begin();
			while (itr != m.end())
			{	
				if (itr->second.normalizeVertices)
				{
					// Averaging algorithim adopted from Luna.
					UINT numTriangles = itr->second.indices.size() / 3;
					for (UINT i = 0; i < numTriangles; i++)
					{
						// Indices of ith triangles.
						UINT i0 = itr->second.indices[i * 3 + 0];
						UINT i1 = itr->second.indices[i * 3 + 1];
						UINT i2 = itr->second.indices[i * 3 + 2];

						// Vertices of ith triangle.
						Vertex v0 = itr->second.vertices[i0];
						Vertex v1 = itr->second.vertices[i1];
						Vertex v2 = itr->second.vertices[i2];

						// Compute face normal.
						XMVECTOR e0 = (XMLoadFloat3(&v0.Pos) - XMLoadFloat3(&v1.Pos));
						XMVECTOR e1 = (XMLoadFloat3(&v2.Pos) - XMLoadFloat3(&v0.Pos));

						XMVECTOR faceNormal = XMVector3Cross(e0, e1);
						for (UINT normalIndex = i0; normalIndex <= i2; normalIndex++)
						{
							XMVECTOR normal = XMLoadFloat3(&itr->second.vertices[normalIndex].Normal);
							normal += faceNormal;
							XMStoreFloat3(&itr->second.vertices[normalIndex].Normal, normal);
						}
					}

					for (UINT i = 0; i < itr->second.vertices.size(); i++)
					{
						// Normalizes and sets the Normal at position i;
						XMStoreFloat3(&itr->second.vertices[i].Normal, XMVector3Normalize(XMLoadFloat3(&itr->second.vertices[i].Normal)));
					}
				}
				itr++;
			}
		#endif
		return m;
    }
    static map<string, MeshData> MESH_MAPS;
};

struct Wall
{
	float row;
	float col;
	float xLength;
	float zLength;
	float centerX;
	float centerY;
	float centerZ;
	string direction;
	string file;
	CREST_TYPE effect;
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
};

struct GameMaterial
{
	string Name;
	string SurfaceKey;
	string DiffuseKey;
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;      
	XMFLOAT4 Specular;
	XMFLOAT4 Reflect;
	int DiffuseTextureID;
	int AmbientTextureID;
	int SpecularTextureID;
	int AlphaTextureID;
	int NormMapTextureID;

	std::wstring MatName;   // So we can match the subset with it's material

	// Booleans so we don't implement techniques we don't need
	bool HasDiffTexture;
	bool HasAmbientTexture;
	bool HasSpecularTexture;
	bool HasAlphaTexture;
	bool HasNormMap;
	bool IsTransparent;
};

static std::map<string, GameMaterial> GAME_MATERIALS;
static std::map<string, SurfaceMaterial> SURFACE_MATERIALS;

// For hardware instancing.
struct InstancedData
{
	XMFLOAT4X4 World;
	SurfaceMaterial SurfMaterial;
	XMFLOAT2 AtlasC;
	bool isRendered;
};

//List our levels:
const char MAP_LEVEL_1[] = "Assets/level1.xml";
const char TEXTURES_FILE[] = "Assets/Textures.xml";
const char SURFACE_MATERIALS_FILE[] = "Assets/SurfaceMaterials.xml";
const char MATERIALS_FILE[] = "Assets/Materials.xml";

const enum PostProcessingEffects
{
	BlurEffect = 0x01,
	WireframeEffect = 0x02
};

// http://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode
static std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

#define DBOUT( s )					\
{									\
	std::wostringstream os;			\
	os << s << endl;				\
	OutputDebugString(os.str().c_str());\
}

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