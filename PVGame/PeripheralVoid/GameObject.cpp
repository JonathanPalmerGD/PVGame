#include "GameObject.h"


GameObject::GameObject(void)
{
	meshKey = "None";
	rigidBody = NULL;
}

GameObject::GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan)
{
	meshKey = aKey;
	surfaceMaterial = aMaterial;
	XMStoreFloat4x4(&worldMatrix, *aWorldMatrix);
	rigidBody = NULL;
	localScale = XMFLOAT3(1.0,1.0,1.0);
	this->physicsMan = physicsMan;
}

GameObject::GameObject(string aKey, SurfaceMaterial aMaterial, XMMATRIX* aWorldMatrix, const btRigidBody& rB, PhysicsManager* physicsMan)
{
	meshKey = aKey;
	surfaceMaterial = aMaterial;
	XMStoreFloat4x4(&worldMatrix, *aWorldMatrix);
	rigidBody = new btRigidBody(rB);
	localScale = XMFLOAT3(1.0,1.0,1.0);
	this->physicsMan = physicsMan;
	physicsMan->addRigidBodyToWorld(rigidBody);
}

void GameObject::translate(float x, float y, float z)
{
	if(rigidBody != NULL)
	{
		rigidBody->translate(btVector3(x, y, z));
	}
}

void GameObject::scale(float x, float y, float z)
{
	if(rigidBody != NULL)
	{
		rigidBody->getCollisionShape()->setLocalScaling(btVector3(x, y, z));
		localScale = XMFLOAT3(x, y, z);
	}
}

void GameObject::rotate(float x, float y, float z, float w)
{
	if(rigidBody!= NULL)
	{
		btTransform t = rigidBody->getWorldTransform();
		t.setRotation(btQuaternion(x, y , z, w));
		rigidBody->setWorldTransform(t);
	}
}

void GameObject::SetMeshKey(string aKey) { meshKey = aKey; }
void GameObject::SetSurfaceMaterial(SurfaceMaterial aMaterial) { surfaceMaterial = aMaterial; }
void GameObject::SetWorldMatrix(XMMATRIX* aMatrix) { XMStoreFloat4x4(&worldMatrix, *aMatrix); }

string GameObject::GetMeshKey() const { return meshKey; }
SurfaceMaterial GameObject::GetSurfaceMaterial() const { return surfaceMaterial; }
btRigidBody* GameObject::getRigidBody() const { return rigidBody; }

void GameObject::SetRigidBody(btRigidBody* rBody)
{
	rigidBody = rBody;
	physicsMan->addRigidBodyToWorld(rigidBody);
}

/* GetWorldMatrix()
 *
 * returns the transformation matrix from the rigid body if it has one.
 *         if it does not have a rigid body, it uses the default XMMatrix it was constructed with
 */
XMFLOAT4X4 GameObject::GetWorldMatrix() 
{ 
	if(rigidBody != NULL)
	{
		btTransform t = rigidBody->getWorldTransform();
		btScalar* mat = new btScalar[16];
		t.getOpenGLMatrix(mat);
		
		worldMatrix = XMFLOAT4X4(mat[0 ] * localScale.x, mat[1 ]                , mat[2 ]               , mat[3 ], //NOT Transposed Matrix  
								 mat[4 ]               , mat[5 ] * localScale.y , mat[6 ]               , mat[7 ], //DO NOT TRANSPOSE MATRIX
							     mat[8 ]               , mat[9 ]                , mat[10] * localScale.z, mat[11], //ITS IN THE CORRECT ROW-COLUMN ORDER
							     mat[12]               , mat[13]                , mat[14]               , mat[15]);
		delete[] mat;
	}
	return worldMatrix; 
}
GameObject::~GameObject(void)
{
	if (rigidBody)
		physicsMan->removeRigidBodyFromWorld(rigidBody);
	//delete rigidBody;
}
