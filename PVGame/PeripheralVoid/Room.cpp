#include "Room.h"

Room::Room(const char* xmlFile, PhysicsManager* pm, float xPos, float zPos)
{
	mapFile = xmlFile;
	physicsMan = pm;
	x = xPos;
	z = zPos;

	tinyxml2::XMLDocument doc;

	doc.LoadFile(xmlFile);
	
	// Set initial room dimensions
	width = -100.0f;
	depth = -100.0f;

	mapOffsetX = 0.0f;
	mapOffsetZ = 0.0f;

	bool isFirst = true;

	XMLElement* walls = doc.FirstChildElement( "level" )->FirstChildElement( "walls" );

	// Find room walls
	for (XMLElement* wall = walls->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
	{
		// Get information from xml
		const char* row = wall->Attribute("row");
		const char* col = wall->Attribute("col");

		const char* xLength = wall->Attribute("xLength");
		const char* zLength = wall->Attribute("zLength");

		if (isFirst)
		{
			mapOffsetX = -atof(col);
			mapOffsetZ = -atof(row);

		//	width -= mapOffsetX;
		//	depth -= mapOffsetZ;

			isFirst = false;
		}

		else
		{
			if (atof(col) < -mapOffsetX)
				mapOffsetX = -atof(col);
		}


		// increase room dimensions if necessary
		if (atof(col) + atof(xLength) + 1 + mapOffsetX > width)
			width = atof(col) + atof(xLength) + mapOffsetX;
		if (atof(row) + atof(zLength) + 1 + mapOffsetZ > depth)
			depth = atof(row) + atof(zLength) + mapOffsetZ;


		/*if ((atof(col) + atof(xLength)) + mapOffsetX > width)
			width += atof(xLength);
		if ((atof(row) + atof(zLength)) + mapOffsetZ > depth)
			depth += atof(zLength);*/
	}

	XMLElement* exits = doc.FirstChildElement( "level" )->FirstChildElement( "exits" );

	// Find room exits
	for (XMLElement* exit = exits->FirstChildElement("exit"); exit != NULL; exit = exit->NextSiblingElement("exit"))
	{
		Wall* tempWall = new Wall();

		// Get information from xml
		const char* row = exit->Attribute("row");
		const char* col = exit->Attribute("col");
		const char* file = exit->Attribute("file");
		const char* xLength = exit->Attribute("xLength");
		const char* zLength = exit->Attribute("zLength");
		const char* centerX = exit->Attribute("centerX");
		const char* centerY = exit->Attribute("centerY");
		const char* centerZ = exit->Attribute("centerZ");

		// Get full filename of xml file
		char folder[80] = "Assets/";
		const char* fullFile = strcat(folder, file);
		
		// Store exit information in Wall object
		tempWall->row = (float)atof(row) + mapOffsetZ;
		tempWall->col = (float)atof(col) + mapOffsetX;
		tempWall->xLength = (float)atof(xLength);
		tempWall->zLength = (float)atof(zLength);
		tempWall->centerX = (float)atof(centerX) + mapOffsetX;
		tempWall->centerY = (float)atof(centerY);
		tempWall->centerZ = (float)atof(centerZ) + mapOffsetZ;
		tempWall->file = fullFile;
		tempWall->direction = "";

		// Add to exit vector
		exitVector.push_back(tempWall);
	}

	int a = 0;
}

Room::~Room(void)
{

}

void Room::loadRoom(void)
{
	loadRoom(x, z);
}

void Room::loadRoom(float xPos, float zPos)
{
	tinyxml2::XMLDocument doc;

	doc.LoadFile(mapFile);

	XMLElement* walls = doc.FirstChildElement( "level" )->FirstChildElement( "walls" );
	XMLElement* spawns = doc.FirstChildElement( "level" )->FirstChildElement( "spawns" );

	vector<vector<Wall*>> wallRowCol;
	vector<Wall*> wallVector;

	wallRowCol.assign(25, wallVector);

	for (XMLElement* wall = walls->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
	{
		Wall* tempWall = new Wall();

		const char* row = wall->Attribute("row");
		const char* col = wall->Attribute("col");
		const char* xLength = wall->Attribute("xLength");
		const char* zLength = wall->Attribute("zLength");
		const char* centerX = wall->Attribute("centerX");
		const char* centerY = wall->Attribute("centerY");
		const char* centerZ = wall->Attribute("centerZ");

		tempWall->row = (float)atof(row) + mapOffsetZ;
		tempWall->col = (float)atof(col) + mapOffsetX;
		tempWall->xLength = (float)atof(xLength);
		tempWall->zLength = (float)atof(zLength);
		tempWall->centerX = (float)atof(centerX) + mapOffsetX;
		tempWall->centerY = (float)atof(centerY);
		tempWall->centerZ = (float)atof(centerZ) + mapOffsetZ;
		tempWall->direction = "";
		tempWall->file = "";
	
		wallRowCol[(unsigned int)atof(row) + mapOffsetZ].push_back(tempWall);
	}

	for (XMLElement* spawn = spawns->FirstChildElement("spawn"); spawn != NULL; spawn = spawn->NextSiblingElement("spawn"))
	{
		Wall* tempWall = new Wall();

		const char* row = spawn->Attribute("row");
		const char* col = spawn->Attribute("col");
		const char* dir = spawn->Attribute("dir");
		const char* xLength = spawn->Attribute("xLength");
		const char* zLength = spawn->Attribute("zLength");
		const char* centerX = spawn->Attribute("centerX");
		const char* centerY = spawn->Attribute("centerY");
		const char* centerZ = spawn->Attribute("centerZ");

		tempWall->row = (float)atof(row) + mapOffsetZ;
		tempWall->col = (float)atof(col) + mapOffsetX;
		tempWall->xLength = (float)atof(xLength);
		tempWall->zLength = (float)atof(zLength);
		tempWall->centerX = (float)atof(centerX) + mapOffsetX;
		tempWall->centerY = (float)atof(centerY);
		tempWall->centerZ = (float)atof(centerZ) + mapOffsetZ;
		tempWall->direction = dir;
		tempWall->file = "";
	
		spawnVector.push_back(tempWall);
	}
	
	// Create walls and add to GameObject vector
	for (unsigned int i = 0; i < wallRowCol.size(); i++)
	{
		for (unsigned int j = 0; j < wallRowCol[i].size(); j++)
		{
			XMMATRIX matrix = XMMatrixIdentity();

			GameObject* wallObj = new GameObject("Cube", "Test Wall", physicsMan->createRigidBody("Cube", wallRowCol[i][j]->centerX + xPos, 1.5f, wallRowCol[i][j]->centerZ + zPos), physicsMan);
			wallObj->scale(wallRowCol[i][j]->xLength,3.0,wallRowCol[i][j]->zLength);
			gameObjs.push_back(wallObj);
		}
	}

	GameObject* wallObj = new GameObject("Cube", "Test Wood", physicsMan->createRigidBody("Cube", xPos + (width / 2), -0.5f, zPos + (depth / 2)), physicsMan);
	wallObj->scale(width, 1.0f, depth);
	gameObjs.push_back(wallObj);

	#pragma endregion
}

void Room::loadNeighbors(void)
{
	// Clear neighbors
	neighbors.clear();

	// Check exits
	for (int i = 0; i < exitVector.size(); i++)
	{
		float offsetX = x, offsetZ = z;

		Room* tmpRoom = new Room(exitVector[i]->file.c_str(), physicsMan, 0, 0); 
		Wall* roomEntrance;

		for (int j = 0; j < tmpRoom->getExits().size(); j++)
		{
			if (tmpRoom->getExits()[j]->file == mapFile)
				roomEntrance = tmpRoom->getExits()[j];
		}

		if (exitVector[i]->row  == 0)
		{
			offsetX -= (roomEntrance->centerX - exitVector[i]->centerX);
			offsetZ -= tmpRoom->depth;
		}

		if (exitVector[i]->row  == depth - 1)
		{
			offsetX -= (roomEntrance->centerX - exitVector[i]->centerX);
			offsetZ += depth;
		}

		if (exitVector[i]->col  == 0)
		{
			offsetX -= tmpRoom->width;
			offsetZ -= (roomEntrance->centerZ - exitVector[i]->centerZ);
		}

		if (exitVector[i]->col == width - 1)
		{
			offsetX += width;
			offsetZ -= (roomEntrance->centerZ - exitVector[i]->centerZ);
		}

		tmpRoom->setX(offsetX);
		tmpRoom->setZ(offsetZ);

		tmpRoom->loadRoom(offsetX, offsetZ);

		neighbors.push_back(tmpRoom);
	}
}