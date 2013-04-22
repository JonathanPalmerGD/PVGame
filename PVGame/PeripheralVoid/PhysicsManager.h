#pragma once

#include <map>
#include <string>
#include "bullet-2.81-rev2613\src\btBulletCollisionCommon.h"
#include "bullet-2.81-rev2613\src\btBulletDynamicsCommon.h"
#include "bullet-2.81-rev2613\src\Bullet-C-Api.h"
#include "bullet-2.81-rev2613\src\BulletDynamics\Character\btKinematicCharacterController.h"
#include "bullet-2.81-rev2613\src\BulletCollision\CollisionDispatch\btGhostObject.h"
#include "GameObject.h"
#include "Common/Camera.h"

using namespace std;

class GameObject;
class Camera;


enum CollisionLayers {
    COL_NOTHING = 0,             //Collide with nothing
    COL_DEFAULT = BIT(0),        //Collide with everything
    COL_RAYCAST = BIT(1),        //Able to be hit by a raycast
    COL_VISION_AFFECTED = BIT(2) //Can collide with the frustum in frustum culling
};

//Make it easy to choose which layer(s) a game object goes into
enum ObjectType {
	NOTHING         = COL_NOTHING,
	WORLD           = COL_DEFAULT|COL_RAYCAST,
	VISION_AFFECTED = COL_DEFAULT|COL_RAYCAST|COL_VISION_AFFECTED,
	PLAYER          = COL_DEFAULT,
	FRUSTUM         = COL_VISION_AFFECTED,
    ALL             = COL_NOTHING|COL_DEFAULT|COL_RAYCAST|COL_VISION_AFFECTED
};

struct CustomFilterCallback : public btOverlapFilterCallback
{
	// return true when pairs need collision
	virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
	{
		return (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
	}
};

class PhysicsManager
{
 
private:
	btDynamicsWorld* world;                    //The world the physics simulation exists in
	btDispatcher* dispatcher;                  //Event handling
	btCollisionConfiguration* collisionConfig; //Collision Handling
	btBroadphaseInterface* broadphase;         //Handles the Broad Phase of Collision Detections, aka determine pairs of objects that could possibly be colliding
	btConstraintSolver* solver;                //Solves Constraints
	btGhostPairCallback* ghostPairCallback;	   // Needs to be separate so it can be deallocated.
	float pStepSize;
	float pAccumulator;

public:
	map<string, btTriangleMesh*> TRIANGLE_MESHES;
	PhysicsManager(void);
	~PhysicsManager(void);
	float getStepSize();
	bool update(float dt);
	btRigidBody* createPlane(float x, float y, float z);
	btRigidBody* createRigidBody(string handle, float mass = 0.0);
	btRigidBody* createRigidBody(string handle, float xPos, float yPos, float zPos, float mass = 0.0);
	btRigidBody* createRigidBody(string handle, float xPos, float yPos, float zPos, float xScale, float yScale, float zScale, float mass = 0.0);

	btPairCachingGhostObject* makeCameraFrustumObject(btTriangleMesh* tMesh);
	btPairCachingGhostObject* makeCameraFrustumObject(btVector3* points, int numPoints);

	void addGhostObjectToWorld(btPairCachingGhostObject* ghost);
	void removeGhostObjectFromWorld(btPairCachingGhostObject* ghost);
	void frustumCulling(btPairCachingGhostObject* ghost);

	void addTriangleMesh(string handle, MeshData meshData);
	void addRigidBodyToWorld(btRigidBody* rigidBody, short collisionLayer);
	void removeRigidBodyFromWorld(btRigidBody* rigidBody);
	
	btKinematicCharacterController* createCharacterController(float height, float radius, float stepHeight);
	void removeCharacterController(btKinematicCharacterController* cc);
	
	bool broadPhase(Camera* playCamera, GameObject* target);
	bool narrowPhase(Camera* playCamera, GameObject* target);
};

