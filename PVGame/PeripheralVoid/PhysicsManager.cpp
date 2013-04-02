#include "PhysicsManager.h"

PhysicsManager::PhysicsManager(void)
{
	collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher      = new btCollisionDispatcher(collisionConfig);
    broadphase      = new btDbvtBroadphase();
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    solver          = new btSequentialImpulseConstraintSolver();
    world           = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
    world->setGravity(btVector3(0.0f,-9.81f,0.0f));

	pStepSize = 1.0f / 60.0f; //Update at 60 fps
	pAccumulator = 0.0f;
}

PhysicsManager::~PhysicsManager()
{
	delete dispatcher;
	delete collisionConfig;
	delete solver;
	delete broadphase;
	delete world;
}

/* update()
 *
 * steps the simulation at a rate of 60 fps
 *
 * param: the time passed in the game in milliseconds
 */
void PhysicsManager::update(float dt)
{
	 pAccumulator += dt;

	 if(pAccumulator >= pStepSize)
	 {
		 world->stepSimulation(pStepSize);
		 pAccumulator-= pStepSize;
	 }
}

/* createPlane()
 *
 * creates a plane at that position facing with (0,1,0) up vector
 *
 * param x y and z position of the center of the plane
 *
 * returns a pointer to the rigid body of the plane
 */
btRigidBody* PhysicsManager::createPlane(float x, float y, float z)
{
	btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(x,y,z));
	btCollisionShape* plane = new btStaticPlaneShape(btVector3(0,1,0),0);
    btMotionState* motion=new btDefaultMotionState(t);
    btRigidBody::btRigidBodyConstructionInfo info(0.0,motion, plane); //0 Mass means that this is a static object
    btRigidBody* body=new btRigidBody(info);
	return body;
}

/* createRigidBody()
 *
 * Creates a rigid body from saved Triangle Mesh Data at position (0,0,0) with scale (1,1,1)
 *
 * params: handle   - the string that will be used to access the the mesh data
 *         mass     - the mass of the rigid body. 0 mass means that the rigid body is static
 */
btRigidBody* PhysicsManager::createRigidBody(string handle, float mass)
{
	map<string, btTriangleMesh*>::const_iterator ptr = TRIANGLE_MESHES.find(handle);
	if(ptr != TRIANGLE_MESHES.end())
	{
		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(0, 0, 0));

		btCollisionShape* triMeshShape;
		if(handle.compare("Cube") == 0)
			triMeshShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
		else if(handle.compare("Sphere") == 0)
			triMeshShape = new btSphereShape(3);
		else
			triMeshShape = new btConvexTriangleMeshShape(ptr->second);

		triMeshShape->setLocalScaling(btVector3(1,1,1));
		btDefaultMotionState* motionState = new btDefaultMotionState(t);

		btVector3 inertia(0,0,0);
		if(mass != 0.0)
			triMeshShape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, triMeshShape, inertia);
		btRigidBody* rigidBody = new btRigidBody(rbInfo);
		return rigidBody;
	}
	return NULL;
}

/* createRigidBody()
 *
 * Creates a rigid body from saved Triangle Mesh Data at position (xPos,yPos,zPos) with scale (1,1,1)
 *
 * params: handle   - the string that will be used to access the the mesh data
 *         Pos      - the position the rigid body will start at
 *         mass     - the mass of the rigid body. 0 mass means that the rigid body is static
 */
btRigidBody* PhysicsManager::createRigidBody(string handle, float xPos, float yPos, float zPos, float mass)
{
	map<string, btTriangleMesh*>::const_iterator ptr = TRIANGLE_MESHES.find(handle);
	if(ptr != TRIANGLE_MESHES.end())
	{
		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(xPos, yPos, zPos));

		btCollisionShape* triMeshShape;
		if(handle.compare("Cube") == 0)
			triMeshShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
		else if(handle.compare("Sphere") == 0)
			triMeshShape = new btSphereShape(3);
		else
			triMeshShape = new btConvexTriangleMeshShape(ptr->second);

		triMeshShape->setLocalScaling(btVector3(1,1,1));
		btDefaultMotionState* motionState = new btDefaultMotionState(t);

		btVector3 inertia(0,0,0);
		if(mass != 0.0)
			triMeshShape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, triMeshShape, inertia);
		btRigidBody* rigidBody = new btRigidBody(rbInfo);
		return rigidBody;
	}
	return NULL;
}

/* createRigidBody()
 *
 * Creates a rigid body from saved Triangle Mesh Data at position (xPos,yPos,zPos) with scale (xScale, yScale, zScale)
 *
 * params: handle   - the string that will be used to access the the mesh data
 *         Pos      - the position the rigid body will start at
 *         Scale    - the scaling of the rigid body
 *         mass     - the mass of the rigid body. 0 mass means that the rigid body is static
 */
btRigidBody* PhysicsManager::createRigidBody(string handle, float xPos, float yPos, float zPos, float xScale, float yScale, float zScale, float mass)
{
	map<string, btTriangleMesh*>::const_iterator ptr = TRIANGLE_MESHES.find(handle);
	if(ptr != TRIANGLE_MESHES.end())
	{
		btTransform t;
		t.setIdentity();
		t.setOrigin(btVector3(xPos, yPos, zPos));

		btCollisionShape* triMeshShape;
		if(handle.compare("Cube") == 0)
			triMeshShape = new btBoxShape(btVector3(0.5, 0.5, 0.5));
		else if(handle.compare("Sphere") == 0)
			triMeshShape = new btSphereShape(3);
		else
			triMeshShape = new btConvexTriangleMeshShape(ptr->second);

		triMeshShape->setLocalScaling(btVector3(1,1,1));
		triMeshShape->setLocalScaling(btVector3(xScale, yScale, zScale));

		btDefaultMotionState* motionState = new btDefaultMotionState(t);

		btVector3 inertia(0,0,0);
		if(mass != 0.0)
			triMeshShape->calculateLocalInertia(mass, inertia);

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, triMeshShape, inertia);
		btRigidBody* rigidBody = new btRigidBody(rbInfo);
		return rigidBody;
	}
	return NULL;
}

/* addTriangleMesh
 *
 * Cooks a btTriangleMesh from a bunch of MeshData. This
 * does not add it to the world, it just makes a kind of prefab
 * that can be used to build rigid bodies later much much faster.
 *
 * params: handle   - the string that will be used to access the rigid body later
 *         meshData - the data that will be used to create the btTriangleMesh
 */
void PhysicsManager::addTriangleMesh(string handle, MeshData meshData)
{
	btTriangleMesh* tMesh = new btTriangleMesh();

	//OH GOD WHY
	//Convert a mesh from our Vertex format to Bullet's btVector3 format
	//This is what makes this method slow
	for(UINT i = 0; i < meshData.indices.size(); i+=3)
	{
		 tMesh->addTriangle(btVector3(meshData.vertices[meshData.indices[i    ]].Pos.x, meshData.vertices[meshData.indices[i    ]].Pos.y, meshData.vertices[meshData.indices[i    ]].Pos.z), 
							btVector3(meshData.vertices[meshData.indices[i + 1]].Pos.x, meshData.vertices[meshData.indices[i + 1]].Pos.y, meshData.vertices[meshData.indices[i + 1]].Pos.z),
							btVector3(meshData.vertices[meshData.indices[i + 2]].Pos.x, meshData.vertices[meshData.indices[i + 2]].Pos.y, meshData.vertices[meshData.indices[i + 2]].Pos.z));
	}

	TRIANGLE_MESHES.insert(map<string, btTriangleMesh*>::value_type(handle,tMesh));
}

/* addRigidBodyToWorld()
 *
 * adds a rigid body to the world so it can be updated in the simulation
 *
 * params: rigidBody - the rigid body to be added to the world
 */
void PhysicsManager::addRigidBodyToWorld(btRigidBody* rigidBody)
{
	if(rigidBody != NULL)
		world->addRigidBody(rigidBody);
}

/* removeRigidBodyFromWorld()
 *
 * removes a rigid body from the world
 */
void PhysicsManager::removeRigidBodyFromWorld(btRigidBody* rigidBody)
{
	if(rigidBody!= NULL)
		world->removeRigidBody(rigidBody);
}

/* broadPhase()
 *
 * Takes the player's view info and the object to check.
 * Returns true if the dot product of the player's view and the objects position is positive.
 */
bool PhysicsManager::broadPhase(Camera* playCamera, btVector3* targetV3)
{
	//btVector3* playerV3 = new btVector3(playCamera->GetPosition().x, playCamera->GetPosition().y, playCamera->GetPosition().z);
	btVector3* playerV3 = new btVector3(playCamera->GetLook().x, playCamera->GetLook().y, playCamera->GetLook().z);
	btVector3 targetRelPosV3 = *targetV3 - *playerV3;
	btScalar pDT= playerV3->dot(targetRelPosV3);
	float angle = acos( pDT / (playerV3->length() * targetRelPosV3.length()));
	playCamera->GetLook();
	angle = angle * 180 / 3.14;
	DBOUT(angle);

	if(angle < 120 && angle > 30  )
		return true;
	else
 		return false;
}

btKinematicCharacterController* PhysicsManager::createCharacterController(float radius, float height, float stepHeight)
{
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(0, 5, 0));
	btCapsuleShape* capsule = new btCapsuleShape(radius, height);
	btPairCachingGhostObject* ghost = new btPairCachingGhostObject();
	ghost->setCollisionShape(capsule);
	ghost->setWorldTransform(t);
	
	btKinematicCharacterController* cc = new btKinematicCharacterController(ghost, capsule, stepHeight);
	world->addCollisionObject(ghost, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter|btBroadphaseProxy::KinematicFilter);

	world->addCharacter(cc);
	return cc;
}

void PhysicsManager::removeCharacterController(btKinematicCharacterController* cc)
{
	world->removeCollisionObject(cc->getGhostObject());
	world->removeCharacter(cc);
}