#include "Crest.h"

Crest::Crest(void)
{
	crestType = MEDUSA;
	inVision = false;
}

Crest::Crest(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, CREST_TYPE aCrestType) : GameObject(aMeshKey, aMaterialKey, aWorldMatrix, physicsMan, true)
{
	crestType = aCrestType;
	inVision = false;
}

Crest::Crest(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, CREST_TYPE aCrestType, float mass) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, mass, true)
{
	crestType = aCrestType;
	inVision = false;
}

Crest::~Crest(void)
{
}

//Accessor to see if the crest is currently being viewed
CREST_TYPE Crest::GetCrestType()
{
	return crestType;
}

//Accessor to see if the crest is currently being viewed
bool Crest::InView()
{
	return inVision;
}

void Crest::ChangeView(bool newVisionBool)
{
	inVision = newVisionBool;
}

void Crest::Update()
{
	if(inVision)
	{
		switch(crestType)
		{
		case MEDUSA:
			//Increment the player's movement speed.
			break;
		case LEAP:
			
			//Increase the player's jump variable.
			//player.setJumpHeight(bigger number)
			break;
		case MOBILITY:
			//Increase the player's movement speed.
			break;
		case UNLOCK:
			//Change the unlocking object to the unlocked state.
			break;
		}
	}

}