#include "Turret.h"

//Turret::Turret(void)
//{
//}

Turret::Turret(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, TURRET_TYPE aTurretType) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, VISION_AFFECTED, 0.0f, true)
{
	turretType = aTurretType;
	inVision = false;
	renderMan = &RenderManager::getInstance();
	SetupAudio();
	CreateLightAndIndex();
}

Turret::~Turret(void)
{
}

void Turret::SetupAudio()
{
	switch(turretType)
	{
	case ALPHA:
		//initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
		break;
	case BETA:
		//initAudio("Audio\\RedOn.wav");
		//initAudio("Audio\\RedOff.wav");
		break;
	case GAMMA:
		//initAudio("Audio\\MobilityOn.wav");
		//initAudio("Audio\\MobilityOff.wav");
		break;
	}
}

bool Turret::InView()
{
	return inVision;
}

void Turret::SetLightIndex(int newLightIndex)
{
	lightIndex = newLightIndex;
}

void Turret::ChangeView(bool newVisionBool)
{
	if(newVisionBool && !inVision)
	{
		renderMan->EnableLight(lightIndex);
		if(!audioSource->isPlaying())
		{
			audioSource->play();
		}
	}
	if(!newVisionBool)
	{
		if(audioSource->isPlaying())
		{
			audioSource->stop();
		}
		renderMan->DisableLight(lightIndex);
	}
	inVision = newVisionBool;
}

//Accessor to see if the crest is currently being viewed
TURRET_TYPE Turret::GetTurretType()
{
	return turretType;
}

int Turret::GetLightIndex()
{
	return lightIndex;
}
//Accessor to see if the crest is currently being viewed
void Turret::CreateLightAndIndex()
{
	switch(turretType)
	{
	case ALPHA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.00f, 0.0f, 1.0f), XMFLOAT4(0.0f, 10.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 5.0f, XMFLOAT3(-0.0f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case BETA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.00f, 0.0f, 0.0f, 1.0f), XMFLOAT4(10.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case GAMMA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.00f, 1.0f), XMFLOAT4(0.0f, 0.0f, 10.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	}
}

void Turret::Update(Player* player)
{
	btVector3 crestPos = getRigidBody()->getCenterOfMassPosition();
	renderMan->SetLightPosition(lightIndex, &crestPos);

	if(inVision)
	{
		switch(turretType)
		{
		case ALPHA:
			//Increment the player's movement speed.
			//player->setMedusaStatus(true);
			//player->increaseMedusaPercent();
			break;
		case BETA:
			//Increase the player's jump variable.
			//player->setLeapStatus(true);
			break;
		case GAMMA:
			//Increase the player's movement speed.
			//player->setMobilityStatus(true);
			break;
		}
	}
	else
	{
		//if(turretType == UNLOCK)
		//{
		//	if(targetObject != NULL)
		//	{
		//	
	//Change the targetObject to the locked state.
		//		targetObject->SetTargetPosition(0);
		//	}
		//}
		//renderMan->DisableLight(lightIndex);
	}
}

//void Crest::ShootTargetObject(btVector3 newShootTarget)
//{
//	if(crestType == UNLOCK)
//	{
//		targetObject = newTargetObject;
//	}
//}