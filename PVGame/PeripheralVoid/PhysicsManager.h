#pragma once
#define PX_PHYSX_CHARACTER_STATIC_LIB 

#include <stdio.h>
#include <iostream>
#include <vector>
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
	PhysicsManager(void);
	~PhysicsManager(void);
	void update(float dt);
	btRigidBody* createPlane(float x, float y, float z);
};

