#include "Projectile.h"

Projectile::Projectile(void)
{
}

Projectile::Projectile(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan) : GameObject(aMeshKey, aMaterialKey, rB, physicsMan, WORLD, 0.0f, false)
{
	/*
	renderMan = &RenderManager::getInstance();
	SetupAudio();
	CreateLightAndIndex();

	*/
}

Projectile::~Projectile(void)
{
}

//If collided with nonplayer. Mark as inactive and move off the world.