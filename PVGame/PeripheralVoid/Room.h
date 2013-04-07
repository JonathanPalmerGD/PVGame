#pragma once

#include <vector>

#include "Constants.h"
#include "GameObject.h"
#include "PhysicsManager.h"
#include "tinyxml2.h"

using namespace tinyxml2;

class Room
{
	public:
		Room(const char* xmlFile, PhysicsManager* pm);
		~Room(void);
		vector<GameObject*> getGameObjs(void) {return gameObjs;};
		void loadRoom(const char* xmlFile);
		Wall getSpawn(void){return spawnVector[0];};
	private:
		PhysicsManager* physicsMan;
		vector<GameObject*> gameObjs;
		vector<Wall> exitVector;
		vector<Wall> spawnVector;
};

