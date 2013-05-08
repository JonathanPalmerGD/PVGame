#include "Turret.h"

//Turret::Turret(void)
//{
//}

Turret::Turret(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan, TURRET_TYPE aTurretType) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, VISION_AFFECTED_COLLISION, 0.0f, true)
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
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT4(10.0f, 10.0f, 10.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-0.0f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case BETA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.00f, 0.0f, 0.0f, 1.0f), XMFLOAT4(10.0f, 0.0f, 10.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	case GAMMA:
		SetLightIndex(renderMan->CreateLight(XMFLOAT4(0.0f, 0.0f, 0.00f, 1.0f), XMFLOAT4(0.0f, 10.0f, 10.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), 5.0f, XMFLOAT3(-7.5f, 0.5f, -7.5f), XMFLOAT3(0.0f, 0.0f, 2.0f)));
		break;
	}
}

void Turret::CreateProjectiles(vector<GameObject> gameObjects)
{
	//btVector3 turretPosition = rigidBody->getCenterOfMassPosition();
	for(int i = 0; i < 10; i++)
	{
		//GameObject* turretGOJ = new Turret("Cube", "Snow", physicsMan->createRigidBody("Cube", 29.0f, 0.5f, 13.0f, 0.0f), physicsMan, ALPHA);
		//gameObjects.push_back(turretGOJ);

		//projectiles.push_back(Projectile());
		//Projectile* projectileObj = new Projectile("Sphere", "Gravel", physicsMan->createRigidBody("Sphere", turretPosition.getX(), turretPosition.getY(), turretPosition.getZ(), 0.0f), physicsMan);
		//GameObject* projectileGObj = new GameObject("Sphere", "Wall", physicsMan->createRigidBody("Sphere", turretPos.getX(), turretPos.getY(), turretPos.getZ(), 0.0f), physicsMan, WORLD, 1.0f, false);	
		//gameObjects.push_back(projectileObj);
		//projectiles.push_back(projectileObj);
		//gameObjects.push_back(projectileGObj);
	}
}

void Turret::Update(Player* player)
{
	btVector3 turretPos = getRigidBody()->getCenterOfMassPosition();
	renderMan->SetLightPosition(lightIndex, &turretPos);

	if(inVision)
	{
		switch(turretType)
		{
		case ALPHA:
			//If(++FireTimer > 1.0f)
				//Choose the first inactive projectile and make it active. 
				//Move it to the turret's muzzle.
				//Give it speed
				//Reset FireTimer

			
			UpdateProjectiles();
			break;
		case BETA:
			
			UpdateProjectiles();
			break;
		case GAMMA:
			
			UpdateProjectiles();
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

void Turret::UpdateProjectiles()
{
	//For each projectile
		//If it is active
			//Check collision with something?
				//Projectile.ResetProjectile();
				//If collided with nonplayer. Mark as inactive and move off the world.
				//If collided with player. Mark as inactive, move off world, reset the player to last checkpoint
}

//void Crest::ShootTargetObject(btVector3 newShootTarget)
//{
//	if(crestType == UNLOCK)
//	{
//		targetObject = newTargetObject;
//	}
//}