#pragma once
#include "Constants.h"
#include "PhysicsManager.h"

class GameObject
{
	public:
		GameObject(void);
		GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix);

		void SetMeshKey(string aKey);
		void SetSurfaceMaterial(SurfaceMaterial aMaterial);
		void SetWorldMatrix(XMMATRIX* aMatrix);

		string GetMeshKey();
		SurfaceMaterial GetSurfaceMaterial();
		XMFLOAT4X4 GetWorldMatrix();

		virtual ~GameObject(void);

	private:
		string meshKey;
		SurfaceMaterial surfaceMaterial;
		btRigidBody* rigidBody;
		XMFLOAT4X4 worldMatrix;
};

