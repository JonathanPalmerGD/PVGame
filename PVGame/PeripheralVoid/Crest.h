#pragma once
#include "gameobject.h"
class Crest :
	public GameObject
{
public:
	Crest(void);
	Crest(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, unsigned int crestType);
	Crest(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, unsigned int crestType, float mass = 0.0);
	~Crest(void);

	bool InView();
	void ChangeView(bool newVisionBool);
	bool GetCrestType();
	void Update();

protected:
	unsigned int crestType;
	bool inVision;
};

