#include "GameObject.h"

GameObject::GameObject(void)
{
	meshKey = "None";
	rigidBody = NULL;
	audioSource = new AudioSource();
	visionAffected = false;
}

GameObject::GameObject(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, bool visionAff)
{
	visionAffected = visionAff;
	meshKey = aMeshKey;
	materialKey = aMaterialKey;
	XMStoreFloat4x4(&worldMatrix, *aWorldMatrix);
	rigidBody = NULL;
	localScale = XMFLOAT3(1.0,1.0,1.0);
	this->physicsMan = physicsMan;
	mass = 0.0;
	audioSource = new AudioSource();
}

GameObject::GameObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, float mass, bool visionAff)
{
	visionAffected = visionAff;
	meshKey = aMeshKey;
	materialKey = aMaterialKey;
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	rigidBody = rB;
	localScale = XMFLOAT3(1.0,1.0,1.0);
	this->physicsMan = physicsMan;
	physicsMan->addRigidBodyToWorld(rigidBody);
	this->mass = mass;
	audioSource = new AudioSource();
	CalculateWorldMatrix();
}

void GameObject::translate(float x, float y, float z)
{
	if(rigidBody != NULL)
	{
		rigidBody->translate(btVector3(x, y, z));
		rigidBody->getMotionState()->setWorldTransform(rigidBody->getWorldTransform());
		CalculateWorldMatrix();
	}
}

void GameObject::setPosition(float x, float y, float z)
{
	if(rigidBody != NULL)
	{
		btTransform t = rigidBody->getWorldTransform();
		t.setOrigin(btVector3(x,y,z));
		rigidBody->setWorldTransform(t);
		rigidBody->getMotionState()->setWorldTransform(rigidBody->getWorldTransform());
		CalculateWorldMatrix();
	}
}

void GameObject::scale(float x, float y, float z)
{
	if(rigidBody != NULL)
	{
		localScale = XMFLOAT3(x, y, z);
		btTransform t;
		rigidBody->getMotionState()->getWorldTransform(t);
		btVector3 position = t.getOrigin();

		physicsMan->removeRigidBodyFromWorld(rigidBody);
		rigidBody = physicsMan->createRigidBody(meshKey, position.getX(), position.getY(), position.getZ(), x, y, z, mass);
		physicsMan->addRigidBodyToWorld(rigidBody);
		CalculateWorldMatrix();
	}
}

void GameObject::rotate(float x, float y, float z, float w)
{
	if(rigidBody!= NULL)
	{
		btTransform t = rigidBody->getWorldTransform();
		t.setRotation(btQuaternion(x, y , z, w));
		rigidBody->setWorldTransform(t);
		rigidBody->getMotionState()->setWorldTransform(rigidBody->getWorldTransform());
		CalculateWorldMatrix();
	}
}

void GameObject::setLinearVelocity(float x, float y, float z)
{
	rigidBody->setLinearVelocity(btVector3(x, y, z));
}

void GameObject::SetMeshKey(string aKey) { meshKey = aKey; }
void GameObject::SetMaterialKey(string aKey) { materialKey = aKey; }
void GameObject::SetWorldMatrix(XMMATRIX* aMatrix) { XMStoreFloat4x4(&worldMatrix, *aMatrix); }

bool GameObject::GetVisionAffected() { return visionAffected; }
string GameObject::GetMeshKey() const { return meshKey; }
string GameObject::GetMaterialKey() const { return materialKey; }
btRigidBody* GameObject::getRigidBody() const { return rigidBody; }

void GameObject::SetRigidBody(btRigidBody* rBody)
{
	if(rigidBody != NULL)
	{
		physicsMan->removeRigidBodyFromWorld(rigidBody);
	}
	rigidBody = rBody;
	physicsMan->addRigidBodyToWorld(rigidBody);

	CalculateWorldMatrix();
}

void GameObject::Update()
{
	if(rigidBody != NULL && (rigidBody->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) != btCollisionObject::CF_STATIC_OBJECT)
	{
		CalculateWorldMatrix();
	}
}

void GameObject::CalculateWorldMatrix()
{
	btTransform t;
	rigidBody->getMotionState()->getWorldTransform(t);
	btScalar* mat = new btScalar[16];
	t.getOpenGLMatrix(mat);
		
	worldMatrix = XMFLOAT4X4(mat[0 ] * localScale.x, mat[1 ]                , mat[2 ]               , mat[3 ], //NOT Transposed Matrix  
								mat[4 ]               , mat[5 ] * localScale.y , mat[6 ]               , mat[7 ], //DO NOT TRANSPOSE MATRIX
							    mat[8 ]               , mat[9 ]                , mat[10] * localScale.z, mat[11], //ITS IN THE CORRECT ROW-COLUMN ORDER
							    mat[12]               , mat[13]                , mat[14]               , mat[15]);

	audioSource->setPosition(t.getOrigin().getX(),t.getOrigin().getY(), t.getOrigin().getZ()); //THIS IS AWEFULL 
	delete[] mat;
}

/* GetWorldMatrix()
 *
 * returns the transformation matrix from the rigid body if it has one.
 *         if it does not have a rigid body, it uses the default XMMatrix it was constructed with
 */
XMFLOAT4X4 GameObject::GetWorldMatrix() const
{ 
	return worldMatrix; 
}

void GameObject::initAudio(string file)
{
	audioSource->initialize(file.c_str(), AudioSource::WAV);
}

void GameObject::playAudio()
{
	if(audioSource->Initialized())
		audioSource->play();
}

void GameObject::setAudioLooping(bool looping)
{
	if(audioSource->Initialized())
		audioSource->setLooping(looping);
}

void GameObject::stopAudio()
{
	if(audioSource->Initialized())
		audioSource->stop();
}

void GameObject::pauseAudio()
{
	if(audioSource->Initialized())
		audioSource->pause();
}

void GameObject::resumeAudio()
{
	if(audioSource->Initialized())
		audioSource->resume();
}

void GameObject::restartAudio()
{
	if(audioSource->Initialized())
		audioSource->restart();
}

void GameObject::restartAndPlayAudio()
{
	if(audioSource->Initialized())
		audioSource->restartAndPlay();
}

GameObject::~GameObject(void)
{
	if (rigidBody)
		physicsMan->removeRigidBodyFromWorld(rigidBody);
	delete audioSource;
}
