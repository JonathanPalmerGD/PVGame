#include "GameObject.h"


GameObject::GameObject(void)
{
	meshKey = "None";
}

GameObject::GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix)
{
	meshKey = aKey;
	surfaceMaterial = aMaterial;
	XMStoreFloat4x4(&worldMatrix, *aWorldMatrix);
}

void GameObject::SetMeshKey(string aKey) { meshKey = aKey; }
void GameObject::SetSurfaceMaterial(SurfaceMaterial aMaterial) { surfaceMaterial = aMaterial; }
void GameObject::SetWorldMatrix(XMMATRIX* aMatrix) { XMStoreFloat4x4(&worldMatrix, *aMatrix); }

string GameObject::GetMeshKey() { return meshKey; }
SurfaceMaterial GameObject::GetSurfaceMaterial() { return surfaceMaterial; }
XMFLOAT4X4 GameObject::GetWorldMatrix() { return worldMatrix; }

GameObject::~GameObject(void)
{
	if(rigidBody)
		delete rigidBody;
}
