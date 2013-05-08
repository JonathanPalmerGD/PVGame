#pragma once
#include "GameObject.h"
#include "Projectile.h"
#include "PVGame.h"

class Projectile;

class Turret :	public GameObject
{

public:
	//Turret(void);
	Turret(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, TURRET_TYPE aTurretType);
	~Turret(void);
	
	void SetupAudio();
	bool InView();
	void SetLightIndex(int newLightIndex);
	//void SetFacing(btVector3 newFacing);
	void ChangeView(bool newVisionBool);
	//void SetShootTarget(btVector3 newShootTarget);
	TURRET_TYPE GetTurretType();
	btVector3 GetFacing(); //Will return the gameObject 'rigidbody' position
	int GetLightIndex();
	void CreateLightAndIndex();
	void CreateProjectiles(vector<GameObject> gameObjects);
	void Update(Player* player);
	void UpdateProjectiles();

protected:
	TURRET_TYPE turretType;
	int lightIndex;
	//btVector3 facing;
	vector<Projectile> projectiles;
	//Each projectile wants a boolean of whether or not it is active. Probably a max number of projectiles.
	//btVector3 shootTarget;
	bool inVision;
	RenderManager *renderMan;
};