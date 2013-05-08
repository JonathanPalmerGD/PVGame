#include "PhysicsManager.h"

PhysicsManager::PhysicsManager(void)
{
	collisionConfig		= new btDefaultCollisionConfiguration();
    dispatcher			= new btCollisionDispatcher(collisionConfig);
    broadphase			= new btDbvtBroadphase();
    solver				= new btSequentialImpulseConstraintSolver();
    world				= new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
	ghostPairCallback	= new btGhostPairCallback();
	filterCallback		= new CustomFilterCallback(); //Set up custom collision filter

	//Set up custom collision filter
	//btOverlapFilterCallback * filterCallback = new CustomFilterCallback();
	world->getPairCache()->setOverlapFilterCallback(filterCallback);

	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	world->setGravity(btVector3(0.0f,-9.81f,0.0f));

	pStepSize = 1.0f / 60.0f; //Update at 60 fps
	pAccumulator = 0.0f;
}

PhysicsManager::~PhysicsManager()
{
	delete world;
    delete solver;
    delete collisionConfig;
    delete dispatcher;
	delete ghostPairCallback;
    delete filterCallback;
    delete broadphase;

	std::map<string, btTriangleMesh*>::iterator itr = TRIANGLE_MESHES.begin();
	while (itr != TRIANGLE_MESHES.end())
	{
		delete itr->second;
		itr++;
	}
}

float PhysicsManager::getStepSize()
{
	return pStepSize;
}

/* update()
 *
 * steps the simulation at a rate of 60 fps
 *
 * param: the time passed in the game in milliseconds
 */
bool PhysicsManager::update(float dt)
{
	 pAccumulator += dt;

	 if(pAccumulator >= pStepSize)
	 {
		 world->stepSimulation(pStepSize);
		 pAccumulator-= pStepSize;
		 return true;
	 }

	 return false;
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
		//rigidBody->setCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE); // Commented out because boxes (but not spheres?) would fall through walls.
		return rigidBody;
	}
	return NULL;
}

/* createRigidBody()
 *
 * Creates a rigid body from saved Triangle Mesh Data at position (xPos,yPos,zPos) with scale (xScale, yScale, zScale)
 *
 * params: handle   - the string that will be used to access the the mesh data
 *         Pos      - the position the rigid body will start atd
 *         Scale    - the scaling of the rigid bodys
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
			triMeshShape = new btSphereShape(3.14f);
		else
			triMeshShape = new btConvexTriangleMeshShape(ptr->second);

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

btPairCachingGhostObject* PhysicsManager::makeCameraFrustumObject(btTriangleMesh* tMesh)
{
	btCollisionShape* ConvexShape = new btConvexTriangleMeshShape(tMesh);
	//btBoxShape* bShape = new btBoxShape(btVector3(1,1,1));
	//btConeShapeZ* coneShape = new btConeShapeZ(10, 100);
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(0,2,0));

	btPairCachingGhostObject* frustum = new btPairCachingGhostObject();
	frustum->setCollisionShape(ConvexShape);
	frustum->setCollisionFlags(frustum->getCollisionFlags()|btCollisionObject::CF_NO_CONTACT_RESPONSE);
    frustum->setWorldTransform(t);

    return frustum;
}

btPairCachingGhostObject* PhysicsManager::makeCameraFrustumObject(btVector3* points, int numPoints)
{
	btConvexHullShape* ConvexShape = new btConvexHullShape();
	for(int i = 0; i < numPoints; i++)
		ConvexShape->addPoint(points[i]);

	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(0,2,0));

	btPairCachingGhostObject* frustum = new btPairCachingGhostObject();
	frustum->setCollisionShape(ConvexShape);
	frustum->setCollisionFlags(frustum->getCollisionFlags()|btCollisionObject::CF_NO_CONTACT_RESPONSE);
    frustum->setWorldTransform(t);

    return frustum;
}

void PhysicsManager::addGhostObjectToWorld(btPairCachingGhostObject* ghost)
{
	if(ghost != NULL)
		world->addCollisionObject(ghost, FRUSTUM, FRUSTUM);
}

void PhysicsManager::removeGhostObjectFromWorld(btPairCachingGhostObject* ghost)
{
	if(ghost != NULL)
	{
		world->removeCollisionObject(ghost);
		delete ghost->getCollisionShape();
		delete ghost;
		ghost = NULL;
	}
}

/* frustumCulling()
 *
 * checks to see which objects the camera's frustum is colliding with
 * sets those objects to be seen
 */
void PhysicsManager::frustumCulling(btPairCachingGhostObject* ghost)
{
   world->getDispatcher()->dispatchAllCollisionPairs(ghost->getOverlappingPairCache(), world->getDispatchInfo(), world->getDispatcher());
   btBroadphasePairArray &CollisionPairs = ghost->getOverlappingPairCache()->getOverlappingPairArray();
   int VisibleCount = 0;
   for(int i = 0; i < CollisionPairs.size(); ++i) {
      const btBroadphasePair &CollisionPair = CollisionPairs[i];
      btManifoldArray ManifoldArray;
      CollisionPair.m_algorithm->getAllContactManifolds(ManifoldArray);
      for(int j = 0; j < ManifoldArray.size(); ++j) {
         btPersistentManifold *Manifold = ManifoldArray[j];
         for(int p = 0; p < Manifold->getNumContacts(); ++p) {
            const btManifoldPoint &Point = Manifold->getContactPoint(p);
            if(Point.getDistance() < 0.0) {
				//camera frustum is body0, so do body1
				GameObject* aGO = (GameObject*)(Manifold->getBody1()->getUserPointer()); //Not sure why this is nulling out
				if(aGO)
					aGO->setSeen(true);
               ++VisibleCount;
            }
         }
      }
   }
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
void PhysicsManager::addRigidBodyToWorld(btRigidBody* rigidBody, short collisionLayer)
{
	if(rigidBody != NULL)
		world->addRigidBody(rigidBody, collisionLayer, collisionLayer);
}

/* removeRigidBodyFromWorld()
 *
 * removes a rigid body from the world
 */
void PhysicsManager::removeRigidBodyFromWorld(btRigidBody* rigidBody)
{
	if(rigidBody != nullptr)
	{
		world->removeRigidBody(rigidBody);
		delete rigidBody->getCollisionShape();
		delete rigidBody->getMotionState();
		delete rigidBody;
	}
}

btKinematicCharacterController* PhysicsManager::createCharacterController(float radius, float height, float stepHeight)
{
	btTransform t;
	t.setIdentity();
	t.setOrigin(btVector3(0, 5, 0));
//btCapsuleShape* capsule = new btCapsuleShape(radius, height);
	btCylinderShape* capsule = new btCylinderShape(btVector3(radius, height/2.0f, radius));
	btPairCachingGhostObject* ghost = new btPairCachingGhostObject();
	ghost->setCollisionShape(capsule);
	ghost->setWorldTransform(t);
	
	btKinematicCharacterController* cc = new btKinematicCharacterController(ghost, capsule, stepHeight);
	world->addCollisionObject(ghost, PLAYER, PLAYER);
	//world->addCollisionObject(ghost, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter);
	world->addCharacter(cc);
	return cc;
}

void PhysicsManager::removeCharacterController(btKinematicCharacterController* cc)
{
	world->removeCollisionObject(cc->getGhostObject());
	world->removeCharacter(cc);
	delete cc->getGhostObject()->getCollisionShape();
	delete cc->getGhostObject();
	delete cc;
}

/* broadPhase()
*
* Takes the player's view info and the object to check.
* Returns true if the dot product of the player's view and the objects position is positive.
*/
bool PhysicsManager::broadPhase(Camera* playCamera, GameObject* target)
{
#if USE_FRUSTUM_CULLING
	return target->isSeen();
#else
	XMMATRIX matrix = XMMatrixIdentity() * XMMatrixTranslation(target->getRigidBody()->getWorldTransform().getOrigin().getX(), 
		                                                       target->getRigidBody()->getWorldTransform().getOrigin().getY(),
															   target->getRigidBody()->getWorldTransform().getOrigin().getZ())
															   * playCamera->ViewProj();
	XMVECTOR result = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), matrix);
	XMFLOAT4 resultFloat;
	XMStoreFloat4(&resultFloat, result);

	/*DBOUT(resultFloat.x / resultFloat.w);
	DBOUT(resultFloat.y / resultFloat.w);
	DBOUT(resultFloat.z / resultFloat.w);
	DBOUT("\n");
	*/
	if (resultFloat.z < 0.0f)
	return false;
	if (abs(resultFloat.x / resultFloat.w) > 1.0f)
	return false;
	if (abs(resultFloat.y / resultFloat.w) > 1.0f)
	return false;

	return true;
#endif
	/*btVector3* playerV3 = new btVector3(playCamera->GetPosition().x, playCamera->GetPosition().y, playCamera->GetPosition().z);
	btVector3* playerV3 = new btVector3(playCamera->GetLook().x, playCamera->GetLook().y, playCamera->GetLook().z);
	btVector3 targetRelPosV3 = *targetV3 - *playerV3;
	btScalar pDT= playerV3->dot(targetRelPosV3);
	float angle = acos( pDT / (playerV3->length() * targetRelPosV3.length()));
	playCamera->GetLook();
	angle = angle * 180 / 3.14;
	DBOUT(angle);

	if(angle < 120 && angle > 30 )
	return true;
	else
	return false;*/
}

/* narrowPhase()
 *
 * Takes the player's view info and the object to check.
 * Returns true if the first thing a raycast from the center of the camera to the center
 * of the object hits is the GameObject
 */
bool PhysicsManager::narrowPhase(Camera* playCamera, GameObject* target)
{
	btVector3 rayFrom(playCamera->GetPosition().x, playCamera->GetPosition().y, playCamera->GetPosition().z);
	btVector3 rayTo(target->getRigidBody()->getWorldTransform().getOrigin());

	btCollisionWorld::ClosestRayResultCallback callback(rayFrom, rayTo);
	callback.m_collisionFilterGroup = COL_RAYCAST;
	callback.m_collisionFilterMask = COL_RAYCAST;
	world->rayTest(rayFrom, rayTo, callback);

	if(callback.hasHit())
	{
		if(callback.m_collisionObject->getCollisionShape() == target->getRigidBody()->getCollisionShape())
			return true;
		#if FINE_PHASE
		else //Generate an "octree" type thing and raycast to "areas"
		{
			btVector3 min, max;
			target->getRigidBody()->getCollisionShape()->getAabb(target->getRigidBody()->getWorldTransform(), min, max);

			float xStep = (max.getX() - min.getX()) / 3.0f;
			float yStep = (max.getY() - min.getY()) / 3.0f;
			float zStep = (max.getZ() - min.getZ()) / 3.0f;

			for(float x = min.getX() + xStep; x < max.getX(); x += xStep)
				for(float y = min.getY() + yStep; y < max.getY(); y += yStep)
					for(float z = min.getZ() + zStep; z < max.getZ(); z += zStep)
					{
						rayTo = btVector3(x,y,z);
						callback = btCollisionWorld::ClosestRayResultCallback(rayFrom, rayTo);
						callback.m_collisionFilterGroup = COL_RAYCAST;
						callback.m_collisionFilterMask = COL_RAYCAST;
						world->rayTest(rayFrom, rayTo, callback);
						if(callback.hasHit() && callback.m_collisionObject->getCollisionShape() == target->getRigidBody()->getCollisionShape())
							return true;
					}
		}
		#endif
		//else // Loop through all vertices and raycast to them
		//{
		//	vector<Vertex> verticies = MeshMaps::MESH_MAPS.find(target->GetMeshKey())->second.vertices;
		//	for(int i = 0; i < verticies.size(); i++)
		//	{ 
		//		//Translate from object space to world space
		//		XMVECTOR result = XMVector4Transform(XMVectorSet(verticies[i].Pos.x,verticies[i].Pos.y, verticies[i].Pos.z, 1.0f), XMLoadFloat4x4(&target->GetWorldMatrix()));
		//		XMFLOAT4 worldVertex;
		//		XMStoreFloat4(&worldVertex, result);

		//		//Raycast to vertex
		//		rayTo = btVector3(worldVertex.x, worldVertex.y, worldVertex.z);
		//		callback = btCollisionWorld::ClosestRayResultCallback(rayFrom, rayTo);
		//		callback.m_collisionFilterGroup = COL_RAYCAST;
		//		callback.m_collisionFilterMask = COL_RAYCAST;
		//		world->rayTest(rayFrom, rayTo, callback);
		//		if(callback.hasHit() && callback.m_collisionObject->getCollisionShape() == target->getRigidBody()->getCollisionShape())
		//			return true;
		//	}
		//}
	}
	return false;
}