#pragma once

#include <map>
#include <string>
#include "Constants.h"
#include "bullet-2.81-rev2613\src\btBulletCollisionCommon.h"
#include "bullet-2.81-rev2613\src\btBulletDynamicsCommon.h"
#include "bullet-2.81-rev2613\src\Bullet-C-Api.h"


using namespace std;



class PhysicsManager
{
 
private:
	btDynamicsWorld* world;                    //The world the physics simulation exists in
	btDispatcher* dispatcher;                  //Event handling
	btCollisionConfiguration* collisionConfig; //Collision Handling
	btBroadphaseInterface* broadphase;         //Handles the Broad Phase of Collision Detections, aka determine pairs of objects that could possibly be colliding
	btConstraintSolver* solver;                //Solves Constraints
	
	float pStepSize;
	float pAccumulator;

public:
	map<string, btRigidBody> RIGID_BODIES;
	PhysicsManager(void);
	~PhysicsManager(void);
	void update(float dt);
	btRigidBody* createPlane(float x, float y, float z);
	void addRigidBodyToMap(string handle, MeshData meshData, float mass = 0.0);
	void addRigidBodyToWorld(btRigidBody* rigidBody);
	void removeRigidBodyFromWorld(btRigidBody* rigidBody);
};

