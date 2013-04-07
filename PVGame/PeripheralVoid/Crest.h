#pragma once
#include "gameobject.h"
class Crest :
	public GameObject
{
public:
	Crest(void);
	Crest(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, CREST_TYPE aCrestType);
	Crest(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, CREST_TYPE aCrestType, float mass = 0.0);
	~Crest(void);

	bool InView();
	void ChangeView(bool newVisionBool);
	CREST_TYPE GetCrestType();
	void Update();

protected:
	CREST_TYPE crestType;
	bool inVision;
};

