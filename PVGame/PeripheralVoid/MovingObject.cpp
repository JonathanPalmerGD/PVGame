#include "MovingObject.h"

void MovingObject::activateCollidingObjects()
{
	
}

bool MovingObject::ReachedTargetPosition()
{
	btVector3 movingObjPos = getRigidBody()->getCenterOfMassPosition();
	if(movingObjPos.getX() == positions[targetPosition].x && movingObjPos.getY() == positions[targetPosition].y && movingObjPos.getZ() == positions[targetPosition].z)
	{
		return true;
	}
	return false;
}

void MovingObject::Update()
{
	GameObject::Update();
	btVector3 movingObjPos = getRigidBody()->getCenterOfMassPosition();
	
	if(movingObjPos.getX() == positions[targetPosition].x && movingObjPos.getY() == positions[targetPosition].y && movingObjPos.getZ() == positions[targetPosition].z)
	{
		
	}
	else
	{
		StepPosition();
	}
	//if(CompareBtV3AndXM3(movingObjPos, positions[targetPosition]))
	//{

	//}

	
}

void MovingObject::StepPosition()
{
	if(rateOfChange < 1.0f)
		rateOfChange += .004f;
	btVector3 movingObjPos = getRigidBody()->getCenterOfMassPosition();
	btVector3 newPosition = btVector3(
		(1.0f - rateOfChange) * movingObjPos.getX() + (positions[targetPosition].x * rateOfChange),
		(1.0f - rateOfChange) * movingObjPos.getY() + (positions[targetPosition].y * rateOfChange),
		(1.0f - rateOfChange) * movingObjPos.getZ() + (positions[targetPosition].z * rateOfChange));
	//btVector3 newPosition = btVector3(positions[targetPosition].x, positions[targetPosition].y, positions[targetPosition].z);
	
	setPosition(newPosition.getX(), newPosition.getY(), newPosition.getZ());
	//translate(newPosition.getX() - movingObjPos.getX(), newPosition.getY() - movingObjPos.getY(), newPosition.getZ() - movingObjPos.getZ());
	//getRigidBody()->translate(positions[targetPosition]));
}

static XMFLOAT3 Interpolate(XMFLOAT3 startPos, XMFLOAT3 targPos)
{

}

/*static bool CompareBtV3AndXM3(btVector3 btV3, XMFLOAT3 xmV3)
{
	if(btV3.getX() == xmV3.x)
	{
		if(btV3.getY() == xmV3.y)
		{
			if(btV3.getZ() == xmV3.z)
			{
				return true;
			}
		}
	}
	return false;
}*/

bool MovingObject::InView() { return inVision; }
int MovingObject::GetTargetPosition() { return targetPosition; }
float MovingObject::GetRateOfChange() { return rateOfChange; }
XMFLOAT3 MovingObject::GetPosition(int index) 
{ 
	if(index < (int)positions.size())
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
	if(newIndex < (int)positions.size())
	{
		if(newIndex != targetPosition)
		{
			//Call the 'Activate touched objects' thing for bullet
			rateOfChange = 0.0f;
			targetPosition = newIndex;
		}
	}
}
void MovingObject::SetRateOfChange(float newRateOfChange) { rateOfChange = newRateOfChange; }
void MovingObject::ChangePositionAtIndex(int index, XMFLOAT3 newPosition) 
{
	if(index < (int)positions.size()) 
	{ 
		positions[index] = newPosition; 
	}
}
void MovingObject::AddPosition(XMFLOAT3 newPosition) { positions.push_back(newPosition); }

MovingObject::MovingObject(void)
{
	inVision = false;
	targetPosition = 0;
	rateOfChange = 0.0f;
	renderMan = &RenderManager::getInstance();
}

MovingObject::MovingObject(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan) : GameObject(aMeshKey, aMaterialKey, aWorldMatrix, physicsMan, false)
{
	//crestType = aCrestType;
	inVision = false;
	targetPosition = 0;
	rateOfChange = 0.0f;
	renderMan = &RenderManager::getInstance();
}

MovingObject::MovingObject(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, WORLD, 0.0f, true)
{
	//crestType = aCrestType;
	inVision = false;
	targetPosition = 0;
	rateOfChange = 0.0f;
	renderMan = &RenderManager::getInstance();
}

MovingObject::~MovingObject(void)
{
}
