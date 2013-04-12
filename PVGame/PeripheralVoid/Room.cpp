#include "Room.h"

Room::Room(const char* xmlFile, PhysicsManager* pm, float xPos, float zPos)
{
	mapFile = xmlFile;
	physicsMan = pm;
	x = xPos;
	z = zPos;

	tinyxml2::XMLDocument doc;

	doc.LoadFile(xmlFile);

	XMLElement* exits = doc.FirstChildElement( "level" )->FirstChildElement( "exits" );

	Wall tempWall = {0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, "", ""};

	// Find room exits
	for (XMLElement* exit = exits->FirstChildElement("exit"); exit != NULL; exit = exit->NextSiblingElement("exit"))
	{
		// Get information from xml
		const char* row = exit->Attribute("row");
		const char* col = exit->Attribute("col");
		const char* file = exit->Attribute("file");

		// Get full filename of xml file
		char folder[80] = "Assets/";
		const char* fullFile = strcat(folder, file);
		
		// Store exit information in Wall object
		tempWall.row = (float)atof(row);
		tempWall.col = (float)atof(col);
		tempWall.centerX = (float)(tempWall.col + tempWall.xLength / 2);
		tempWall.centerZ = (float)(tempWall.row + tempWall.zLength / 2);
		tempWall.file = fullFile;
	
		// Add to exit vector
		exitVector.push_back(tempWall);
	}
	
	// Set initial room dimensions
	width = 1;
	depth = 1;


	XMLElement* walls = doc.FirstChildElement( "level" )->FirstChildElement( "walls" );

	// Find room walls
	for (XMLElement* wall = walls->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
	{
		// Get information from xml
		const char* row = wall->Attribute("row");
		const char* col = wall->Attribute("col");

		// increase room dimensions if necessary
		if (atof(col) + 1 > width)
			width = atof(col) + 1;
		if (atof(row) + 1 > depth)
			depth = atof(row) + 1;
	}
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

	Wall tempWall = {0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, "", ""};
	vector<vector<Wall>> wallRowCol;
	vector<Wall> wallVector;

	wallRowCol.assign(25, wallVector);

	for (XMLElement* wall = walls->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
	{
		const char* row = wall->Attribute("row");
		const char* col = wall->Attribute("col");
		const char* xLength = wall->Attribute("xLength");
		const char* zLength = wall->Attribute("zLength");
		const char* centerX = wall->Attribute("centerX");
		const char* centerY = wall->Attribute("centerY");
		const char* centerZ = wall->Attribute("centerZ");

		tempWall.row = (float)atof(row);
		tempWall.col = (float)atof(col);
		tempWall.xLength = (float)atof(xLength);
		tempWall.zLength = (float)atof(zLength);
		tempWall.centerX = (float)atof(centerX);
		tempWall.centerY = (float)atof(centerY);
		tempWall.centerZ = (float)atof(centerZ);
	
		wallRowCol[(unsigned int)atof(row)].push_back(tempWall);
	}

	for (XMLElement* spawn = spawns->FirstChildElement("spawn"); spawn != NULL; spawn = spawn->NextSiblingElement("spawn"))
	{
		const char* row = spawn->Attribute("row");
		const char* col = spawn->Attribute("col");
		const char* dir = spawn->Attribute("dir");
		const char* xLength = spawn->Attribute("xLength");
		const char* zLength = spawn->Attribute("zLength");
		const char* centerX = spawn->Attribute("centerX");
		const char* centerY = spawn->Attribute("centerY");
		const char* centerZ = spawn->Attribute("centerZ");

		tempWall.row = (float)atof(row);
		tempWall.col = (float)atof(col);
		tempWall.xLength = (float)atof(xLength);
		tempWall.zLength = (float)atof(zLength);
		tempWall.centerX = (float)atof(centerX);
		tempWall.centerY = (float)atof(centerY);
		tempWall.centerZ = (float)atof(centerZ);
		tempWall.direction = dir;
	
		spawnVector.push_back(tempWall);
	}
	
	// Create walls and add to GameObject vector
	for (unsigned int i = 0; i < wallRowCol.size(); i++)
	{
		for (unsigned int j = 0; j < wallRowCol[i].size(); j++)
		{
			XMMATRIX matrix = XMMatrixIdentity();

			GameObject* wallObj = new GameObject("Cube", "Test Wall", physicsMan->createRigidBody("Cube", wallRowCol[i][j].centerX + xPos, 1.5f, wallRowCol[i][j].centerZ + zPos), physicsMan);
			wallObj->scale(wallRowCol[i][j].xLength,3.0,wallRowCol[i][j].zLength);
			gameObjs.push_back(wallObj);
		}
	}

	GameObject* wallObj = new GameObject("Cube", "Test Wood", physicsMan->createRigidBody("Cube", x + (width / 2), -0.5f, z + (depth / 2)), physicsMan);
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

		Room tmpRoom(exitVector[i].file.c_str(), physicsMan, 0, 0); 

		if (exitVector[i].row == 0)
			offsetZ = z - tmpRoom.depth;

		if (exitVector[i].row == depth - 1)
			offsetZ = z + depth;

		if (exitVector[i].col == 0)
			offsetX = x - tmpRoom.width;

		if (exitVector[i].col == width - 1)
			offsetX = x + width;

		tmpRoom.setX(offsetX);
		tmpRoom.setZ(offsetZ);

		tmpRoom.loadRoom(offsetX, offsetZ);

		neighbors.push_back(tmpRoom);
	}
}