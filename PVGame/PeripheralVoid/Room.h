#pragma once

#include "Constants.h"
#include "PhysicsManager.h"
#include "tinyxml2.h"

using namespace tinyxml2;

class MovingObject;

class Room
{
	public:
		Room(const char* xmlFile, PhysicsManager* pm, float xPos, float zPos);
		~Room(void);
		vector<GameObject*> getGameObjs(void) { return gameObjs; }
		void loadRoom(void);
		void loadNeighbors(vector<Room*> loadedRooms);
		Wall* getSpawn(void){return spawnVector[0];};
		float getX(void){return x;};
		float getZ(void){return z;};
		float getWidth(void){return width;};
		float getDepth(void){return depth;};
		const char* getFile(void){return mapFile;};
		vector<Wall*> getExits(void){return exitVector;};
		vector<Room*> getNeighbors(void){return neighbors;};
		void setX(float xPos){x = xPos;};
		void setZ(float zPos){z = zPos;};
	private:
		PhysicsManager* physicsMan;
		vector<GameObject*> gameObjs;
		vector<Wall*> floorVector;
		vector<Wall*> exitVector;
		vector<Wall*> spawnVector;
		vector<Cube*> cubeVector;
		vector<Wall*> crestVector;
		vector<Room*> neighbors;
		map<string, MovingObject*> cubeMap; 
		const char* mapFile;
		float x;
		float z;
		float width; 
		float depth;
		float mapOffsetX;
		float mapOffsetZ;
		void loadRoom(float xPos, float zPos);
};

