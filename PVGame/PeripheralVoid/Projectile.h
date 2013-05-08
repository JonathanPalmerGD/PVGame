#pragma once
#include "GameObject.h"
#include "PVGame.h"

class Turret;

class Projectile : public GameObject
{
public:
	Projectile(void);
	Projectile(string aMeshKey, string aMaterialKey, btRigidBody* rB, PhysicsManager* physicsMan);
	~Projectile(void);
	void SetMoving(bool newMoving);
	//void SetFacing(btVector3 newFacing);
	void SetParent(Turret *newParent);
	bool GetMoving();
	btVector3 GetFacing(); //Will return the gameObject 'rigidbody' position
	Turret GetParent();
	void ResetProjectile();

protected:
	bool moving;
	//btVector3 facing;
	Turret *parent;
};

