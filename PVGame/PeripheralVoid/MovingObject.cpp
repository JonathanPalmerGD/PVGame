#include "MovingObject.h"


void MovingObject::Update()
{
	//if(rigidBody->getWorldTransform().getOrigin() == positions[targetPosition])
	//{

	//}

	GameObject::Update();
}

static XMFLOAT3 Interpolate(XMFLOAT3 startPos, XMFLOAT3 targPos)
{

}

bool MovingObject::InView() { return inVision; }
int MovingObject::GetTargetPosition() { return targetPosition; }
float MovingObject::GetRateOfChange() { return rateOfChange; }
XMFLOAT3 MovingObject::GetPosition(int index) 
{ 
	if(index < positions.size())
	{
		return positions[index]; 
	}
	else
	{
		return XMFLOAT3(0,0,0);
	}
}

void MovingObject::ChangeView(bool newVisionBool) { inVision = newVisionBool; }
void MovingObject::SetTargetPosition(int newIndex) 
{ 
	if(newIndex < positions.size())
	{
		targetPosition = newIndex; 
	}
}
void MovingObject::SetRateOfChange(float newRateOfChange) { rateOfChange = newRateOfChange; }
void MovingObject::ChangePositionAtIndex(int index, XMFLOAT3 newPosition) 
{
	if(index < positions.size()) 
	{ 
		positions[index] = newPosition; 
	}
}
void MovingObject::AddPosition(XMFLOAT3 newPosition) { positions.push_back(newPosition); }



MovingObject::MovingObject(void)
{
	inVision = false;
}

MovingObject::MovingObject(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan) : GameObject(aMeshKey, aMaterialKey, aWorldMatrix, physicsMan, true)
{
	//crestType = aCrestType;
	inVision = false;
}

MovingObject::MovingObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, float mass) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, mass, true)
{
	//crestType = aCrestType;
	inVision = false;
}

MovingObject::~MovingObject(void)
{
}
