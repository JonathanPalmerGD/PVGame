#pragma once

#include <map>
#include <string>
#include "Constants.h"
#include "Common/Camera.h"
#include "bullet-2.81-rev2613\src\btBulletCollisionCommon.h"
#include "bullet-2.81-rev2613\src\btBulletDynamicsCommon.h"
#include "bullet-2.81-rev2613\src\Bullet-C-Api.h"
#include "bullet-2.81-rev2613\src\BulletDynamics\Character\btKinematicCharacterController.h"
#include "bullet-2.81-rev2613\src\BulletCollision\CollisionDispatch\btGhostObject.h"

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
	map<string, btTriangleMesh*> TRIANGLE_MESHES;
	PhysicsManager(void);
	~PhysicsManager(void);
	void update(float dt);
	btRigidBody* createPlane(float x, float y, float z);
	btRigidBody* createRigidBody(string handle, float mass = 0.0);
	btRigidBody* createRigidBody(string handle, float xPos, float yPos, float zPos, float mass = 0.0);
	btRigidBody* createRigidBody(string handle, float xPos, float yPos, float zPos, float xScale, float yScale, float zScale, float mass = 0.0);
	void addTriangleMesh(string handle, MeshData meshData);
	void addRigidBodyToWorld(btRigidBody* rigidBody);
	void removeRigidBodyFromWorld(btRigidBody* rigidBody);
	bool broadPhase(Camera* playCamera, btVector3* targetV3);
	btKinematicCharacterController* createCharacterController(float height, float radius, float stepHeight);
	void removeCharacterController(btKinematicCharacterController* cc);
};

