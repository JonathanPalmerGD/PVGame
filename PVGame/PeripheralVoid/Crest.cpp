#include "Crest.h"

Crest::Crest(void)
{
	crestType = LEAP;
	inVision = false;
	renderMan = &RenderManager::getInstance();
	SetupAudio();
	CreateLightAndIndex();
}

Crest::Crest(string aMeshKey, string aMaterialKey, XMMATRIX* aWorldMatrix, PhysicsManager* physicsMan, CREST_TYPE aCrestType) : GameObject(aMeshKey, aMaterialKey, aWorldMatrix, physicsMan, true)
{
	crestType = aCrestType;
	inVision = false;
	renderMan = &RenderManager::getInstance();
	SetupAudio();
	CreateLightAndIndex();
}

Crest::Crest(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, CREST_TYPE aCrestType, float mass) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, VISION_AFFECTED_NOCOLLISION, mass, true)
{
	crestType = aCrestType;
	inVision = false;
	renderMan = &RenderManager::getInstance();
	SetupAudio();
	CreateLightAndIndex();
}

Crest::~Crest(void)
{
}

void Crest::SetLightIndex(int newLightIndex)
{
	lightIndex = newLightIndex;
}

void Crest::ChangeView(bool newVisionBool)
{
	if(newVisionBool && !inVision)
	{
		renderMan->EnableLight(lightIndex);

		if(!audioSource->isPlaying())
		{
			audioSource->play();
		}

		if(crestType == HADES)
		{
			changeCollisionLayer(VISION_AFFECTED_COLLISION);
		}
	}
	if(!newVisionBool)
	{
		renderMan->DisableLight(lightIndex);

		if(audioSource->isPlaying())
		{
			audioSource->stop();
		}

		if(crestType == HADES)
		{
			changeCollisionLayer(VISION_AFFECTED_NOCOLLISION);
		}
	}
	inVision = newVisionBool;
}

void Crest::SetTargetObject(MovingObject *newTargetObject)
{
	if(crestType == UNLOCK)
	{
		targetObject = newTargetObject;
	}
}

//Accessor to see if the crest is currently being viewed
CREST_TYPE Crest::GetCrestType()
{
	return crestType;
}

// Returns the name of the supplied crest type - this serves as the key to the linked GameMaterial as well.
string Crest::GetCrestTypeString(CREST_TYPE aType)
{
	switch (aType)
	{
		case MEDUSA:
			return "Medusa Crest";
			break;
		case MOBILITY:
			return "Mobility Crest";
			break;
		case LEAP:
			return "Leap Crest";
			break;
		case UNLOCK:
			return "Unlock Crest";
			break;
		case HADES:
			return "Hades Crest";
			break;
	}
	return "Unknown Crest";
}

int Crest::GetLightIndex()
{
	return lightIndex;
}

//Accessor to see if the crest is currently being viewed
bool Crest::InView()
{
	return inVision;
}

void Crest::SetupAudio()
{
	switch(crestType)
	{
	case MEDUSA:
		//initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
		initAudio("Audio\\Scream.wav");
		break;
	case LEAP:
		initAudio("Audio\\RedOn.wav");
		//initAudio("Audio\\RedOff.wav");
		break;
	case MOBILITY:
		initAudio("Audio\\MobilityOn.wav");
		//initAudio("Audio\\MobilityOff.wav");
		break;
	case UNLOCK:
		initAudio("Audio\\YellowOn.wav");
		//initAudio("Audio\\YellowOff.wav");
		break;
	}
}

void Crest::CreateLightAndIndex()
{
	switch(crestType)
	{
	case MEDUSA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.00f, 0.0f, 1.0f), XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 5.0f, XMFLOAT3(-0.0f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case MOBILITY:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.00f, 0.0f, 0.0f, 1.0f), XMFLOAT4(10.0f, 3.0f, 3.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case LEAP:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(3.0f, 3.0f, 10.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case UNLOCK:   //255, 215, 0
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(10.0f, 8.4f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case HADES:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(3.0f, 3.0f, 3.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	}
}

void Crest::Update(Player* player)
{
	btVector3 crestPos = getRigidBody()->getCenterOfMassPosition();
	renderMan->SetLightPosition(lightIndex, &crestPos);

	if(inVision)
	{
		switch(crestType)
		{
		case MEDUSA:
			//Increment the player's movement speed.
			player->setMedusaStatus(true);
			player->increaseMedusaPercent();
			break;
		case LEAP:
			//Increase the player's jump variable.
			player->setLeapStatus(true);
			break;
		case MOBILITY:
			//Increase the player's movement speed.
			player->setMobilityStatus(true);
			break;
		case UNLOCK:
			if(targetObject != NULL)
			{
				//Change the targetObject to the unlocked state.
				targetObject->SetTargetPosition(1);
			}
			break;
		case HADES:
				//Hi
			break;
		}
	}
	else
	{
		if(crestType == UNLOCK)
		{
			if(targetObject != NULL)
			{
				//Change the targetObject to the locked state.
				targetObject->SetTargetPosition(0);
			}
		}
		if(crestType == HADES)
		{
			//Hi
		}
	}
}