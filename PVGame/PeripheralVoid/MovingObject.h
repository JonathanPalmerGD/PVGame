#pragma once
#include "GameObject.h"
class MovingObject : public GameObject
{
public:

	void Update();
	static XMFLOAT3 Interpolate(XMFLOAT3 startPos, XMFLOAT3 targPos);
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
	MovingObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, float mass = 0.0);

	~MovingObject(void);
	
protected:
	bool inVision;
	float rateOfChange;
	vector<XMFLOAT3> positions;
	int targetPosition;
};
