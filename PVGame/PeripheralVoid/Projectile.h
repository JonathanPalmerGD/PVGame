#pragma once
#include "GameObject.h"
#include "PVGame.h"
#include "Turret.h"

class Turret;

class Projectile : public GameObject
{
public:
	Projectile(void);
	~Projectile(void);
	void SetMoving(bool newMoving);
	//void SetFacing(btVector3 newFacing);
	void SetParent(Turret *newParent);
	bool GetMoving();
	btVector3 GetFacing();
	Turret GetParent();
	void ResetProjectile();

protected:
	bool moving;
	//btVector3 facing;
	Turret *parent;
};

