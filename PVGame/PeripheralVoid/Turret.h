#pragma once
#include "GameObject.h"
#include "PVGame.h"

class Turret :	public GameObject
{

public:
	Turret(void);
	Turret(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, TURRET_TYPE aTurretType, float mass = 0.0);
	~Turret(void);
	
	void SetupAudio();
	bool InView();
	void ChangeView(bool newVisionBool);
	//void SetShootTarget(btVector3 newShootTarget);
	TURRET_TYPE GetTurretType();
	int GetLightIndex();
	void Update(Player* player);

protected:
	TURRET_TYPE turretType;
	//btVector3 shootTarget;
	bool inVision;
	RenderManager *renderMan;
};