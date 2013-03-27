#include "PhysicsManager.h"


PhysicsManager::PhysicsManager(void)
{
	collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher      = new btCollisionDispatcher(collisionConfig);
    broadphase      = new btDbvtBroadphase();
    solver          = new btSequentialImpulseConstraintSolver();
    world           = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
    world->setGravity(btVector3(0,-9.81,0));

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
	 pAccumulator += dt/1000.0f;

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

/* addRigidBodyToMap()
 *
 * Cooks a rigid body from a bunch of MeshData. This
 * does not add it to the world, it just makes a kind of prefab
 * that can be used to build rigid bodies later much much faster.
 *
 * params: handle   - the string that will be used to access the rigid body later
 *         meshData - the data that will be used to create the rigid body
 *         mass     - the mass of the rigid body. 0 mass means that the rigid body is static
 */
void PhysicsManager::addRigidBodyToMap(string handle, MeshData meshData, float mass)
{
	btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(0,0,0));
	btDefaultMotionState* motionState = new btDefaultMotionState(t);
	btTriangleMesh* tMesh = new btTriangleMesh();
	
	//OH GOD WHY
	//Convert a mesh from our Vertex format to Bullet's btVector3 format
	//This is what makes this method slow
	for(int i = 0; i < meshData.indices.size(); i+=3)
	{
		 tMesh->addTriangle(btVector3(meshData.vertices[meshData.indices[i    ]].Pos.x, meshData.vertices[meshData.indices[i    ]].Pos.y, meshData.vertices[meshData.indices[i    ]].Pos.z), 
							btVector3(meshData.vertices[meshData.indices[i + 1]].Pos.x, meshData.vertices[meshData.indices[i + 1]].Pos.y, meshData.vertices[meshData.indices[i + 1]].Pos.z),
							btVector3(meshData.vertices[meshData.indices[i + 2]].Pos.x, meshData.vertices[meshData.indices[i + 2]].Pos.y, meshData.vertices[meshData.indices[i + 2]].Pos.z));
	}

	btCollisionShape* mTriMeshShape = new btBvhTriangleMeshShape(tMesh, false);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, mTriMeshShape, btVector3(0,0,0));
	btRigidBody rigidBody(rbInfo);

	RIGID_BODIES.insert(map<string, btRigidBody>::value_type(handle,rigidBody));
}

/* addRigidBodyToWorld()
 *
 * adds a rigid body to the world so it can be updated in the simulation
 *
 * params: rigidBody - the rigid body to be added to the world
 */
void PhysicsManager::addRigidBodyToWorld(btRigidBody* rigidBody)
{
	world->addRigidBody(rigidBody);
}

/* removeRigidBodyFromWorld()
 *
 * removes a rigid body from the world
 */
void PhysicsManager::removeRigidBodyFromWorld(btRigidBody* rigidBody)
{
	world->removeRigidBody(rigidBody);
}