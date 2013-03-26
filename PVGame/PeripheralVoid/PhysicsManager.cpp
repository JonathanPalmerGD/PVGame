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
	btStaticPlaneShape plane(btVector3(0,1,0),0);
    btMotionState* motion=new btDefaultMotionState(t);
    btRigidBody::btRigidBodyConstructionInfo info(0.0,motion, &plane);
    btRigidBody* body=new btRigidBody(info);
    world->addRigidBody(body);
	return body;
}