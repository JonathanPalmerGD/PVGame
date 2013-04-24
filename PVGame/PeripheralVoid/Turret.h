#pragma once
#include "GameObject.h"
#include "PVGame.h"

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
	btVector3 GetFacing();
	int GetLightIndex();
	void CreateLightAndIndex();
	void Update(Player* player);

protected:
	TURRET_TYPE turretType;
	int lightIndex;
	btVector3 facing;
	vector<Crest> projectiles;
	//Each projectile wants a boolean of whether or not it is active. Probably a max number of projectiles.
	//btVector3 shootTarget;
	bool inVision;
	RenderManager *renderMan;
};