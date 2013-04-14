#pragma once
#include "GameObject.h"
#include "PVGame.h"
class MovingObject : public GameObject
{
public:

	void Update();
	static XMFLOAT3 Interpolate(XMFLOAT3 startPos, XMFLOAT3 targPos);
	void StepPosition();
	//static bool CompareBtV3AndXM3(btVector3 btV3, XMFLOAT3 xmV3);
	bool InView();
	int GetTargetPosition();
	float GetRateOfChange();
	XMFLOAT3 GetPosition(int index);
	void ChangeView(bool newVisionBool);
	void SetTargetPosition(int newIndex);
	void SetRateOfChange(float newRateOfChange);
	void ChangePositionAtIndex(int index, XMFLOAT3 newPosition);
	void AddPosition(XMFLOAT3 newPosition);

	MovingObject(void);
	MovingObject(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan);
	MovingObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan);

	~MovingObject(void);
	
protected:
	bool inVision;
	float rateOfChange;
	vector<XMFLOAT3> positions;
	int targetPosition;
	RenderManager *renderMan;
};
