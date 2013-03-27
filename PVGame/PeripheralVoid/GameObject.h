#pragma once
#include "Constants.h"
#include "PhysicsManager.h"

//Only GameObjects that need to be affected by physics, aka the player not walking through them need a rigid body
class GameObject
{
	public:
		GameObject(void);
		GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix);
		GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix, const btRigidBody& rB);

		void translate(float x, float y, float z);
		void scale(float x, float y, float z);
		void rotate(float x, float y, float z, float w);

		void SetMeshKey(string aKey);
		void SetSurfaceMaterial(SurfaceMaterial aMaterial);
		void SetWorldMatrix(XMMATRIX* aMatrix);
		void SetRigidBody(btRigidBody* rigidBody);

		string GetMeshKey();
		SurfaceMaterial GetSurfaceMaterial();
		XMFLOAT4X4 GetWorldMatrix();
		btRigidBody* getRigidBody();

		virtual ~GameObject(void);

	private:
		string meshKey;
		SurfaceMaterial surfaceMaterial;
		btRigidBody* rigidBody;
		XMFLOAT4X4 worldMatrix;
		XMFLOAT3 localScale;
};

