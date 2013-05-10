#include "GameObject.h"

GameObject::GameObject(void)
{
	meshKey = "None";
	rigidBody = NULL;
	audioSource = new AudioSource();
	visionAffected = false;
	collisionLayer = 0;

	if(USE_FRUSTUM_CULLING && (collisionLayer & COL_VISION_AFFECTED))
		seen = false;
	else
		seen = true;
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
	//this->collisionLayer = ObjectType::WORLD;
	audioSource = new AudioSource();
	if(USE_FRUSTUM_CULLING && (collisionLayer & COL_VISION_AFFECTED))
		seen = false;
	else
		seen = true;
}

GameObject::GameObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, short collisionLayer, float mass, bool visionAff)
{
	visionAffected = visionAff;
	meshKey = aMeshKey;
	materialKey = aMaterialKey;
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	rigidBody = rB;
	btVector3 s = rigidBody->getCollisionShape()->getLocalScaling();
	localScale = XMFLOAT3(1.0,1.0,1.0);
	this->physicsMan = physicsMan;
	physicsMan->addRigidBodyToWorld(rigidBody, collisionLayer);
	rigidBody->setUserPointer(this);
	this->mass = mass;
	this->collisionLayer = collisionLayer;
	audioSource = new AudioSource();
	CalculateWorldMatrix();
	if(USE_FRUSTUM_CULLING && (collisionLayer & COL_VISION_AFFECTED))
		seen = false;
	else
		seen = true;
}

void GameObject::setSeen(bool s)
{
	seen = s;
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

bool GameObject::isSeen()
{
	return seen;
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
		rigidBody->setUserPointer(this);
		physicsMan->addRigidBodyToWorld(rigidBody, collisionLayer);
		CalculateWorldMatrix();
	}
}

void GameObject::SetTexScale(float x, float y, float z, float w)
{
	texScale = XMFLOAT4(x, y, z, w);
}

void GameObject::rotate(float x, float y, float z, float w)
{
	if(rigidBody!= NULL)
	{
		/*btTransform t = rigidBody->getWorldTransform();
		t.setRotation(btQuaternion(x, y , z, w));
		rigidBody->setWorldTransform(t);
		rigidBody->getMotionState()->setWorldTransform(rigidBody->getWorldTransform());*/
		btTransform t = rigidBody->getWorldTransform();
		btQuaternion rotation(x, y, z, w);
		t.setRotation(rotation);

		rigidBody->getMotionState()->setWorldTransform(t);
		CalculateWorldMatrix();
	}
}

void GameObject::rotate(float yaw, float pitch, float roll)
{
	if(rigidBody!= NULL)
	{
		/*btTransform t = rigidBody->getWorldTransform();
		t.setRotation(btQuaternion(yaw, pitch, roll));
		rigidBody->setWorldTransform(t);
		rigidBody->getMotionState()->setWorldTransform(rigidBody->getWorldTransform());*/

		btTransform t = rigidBody->getWorldTransform();
		btQuaternion rotation(yaw, pitch, roll);
		t.setRotation(rotation);

		rigidBody->getMotionState()->setWorldTransform(t);
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

void GameObject::SetRigidBody(btRigidBody* rBody, short layer)
{
	if(rigidBody != NULL)
	{
		physicsMan->removeRigidBodyFromWorld(rigidBody);
	}
	rigidBody = rBody;
	rigidBody->setUserPointer(this);
	physicsMan->addRigidBodyToWorld(rigidBody, layer);

	CalculateWorldMatrix();
}

void GameObject::changeCollisionLayer(short layer)
{
	if(rigidBody != NULL)
	{
		btTransform t;
		rigidBody->getMotionState()->getWorldTransform(t);
		btVector3 position = t.getOrigin();

		physicsMan->removeRigidBodyFromWorld(rigidBody);
		rigidBody = physicsMan->createRigidBody(meshKey, position.getX(), position.getY(), position.getZ(), localScale.x, localScale.y, localScale.z, mass);
		rigidBody->setUserPointer(this);
		physicsMan->addRigidBodyToWorld(rigidBody, layer);
	}
}

void GameObject::Update()
{
	if(rigidBody != NULL)
	{
		if(USE_FRUSTUM_CULLING && (collisionLayer & COL_VISION_AFFECTED))
		{
			seen = false;
		}
		CalculateWorldMatrix();
		rigidBody->setUserPointer(this);
	}
}

void GameObject::CalculateWorldMatrix()
{
	btTransform t;
	rigidBody->getMotionState()->getWorldTransform(t);
	btScalar* mat = new btScalar[16];
	t.getOpenGLMatrix(mat);
		
	worldMatrix = XMFLOAT4X4(mat[0 ] * localScale.x, mat[1 ]                , mat[2 ]               , mat[3 ],    //NOT Transposed Matrix  
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
	if (rigidBody && physicsMan)
		physicsMan->removeRigidBodyFromWorld(rigidBody);
	
	if (audioSource)
		delete audioSource;
}
