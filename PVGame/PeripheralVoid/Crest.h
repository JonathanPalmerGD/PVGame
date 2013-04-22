#pragma once
#include "GameObject.h"
#include "PVGame.h"
#include "MovingObject.h"

class MovingObject;

class Crest : public GameObject
{
public:
	Crest(void);
	Crest(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, CREST_TYPE aCrestType);
	Crest(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, CREST_TYPE aCrestType, float mass = 0.0);
	~Crest(void);

	void SetupAudio();
	bool InView();
	void SetLightIndex(int newLightIndex);
	void ChangeView(bool newVisionBool);
	void SetTargetObject(MovingObject *newTargetObject);
	CREST_TYPE GetCrestType();
	static string GetCrestTypeString(CREST_TYPE aType);
	int GetLightIndex();
	void CreateLightAndIndex();
	void Update(Player* player);

protected:
	CREST_TYPE crestType;
	int lightIndex;
	MovingObject *targetObject;
	bool inVision;
	RenderManager *renderMan;
};

