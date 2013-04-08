#include "Room.h"

Room::Room(const char* xmlFile, PhysicsManager* pm)
{
	physicsMan = pm;

	tinyxml2::XMLDocument doc;

	doc.LoadFile(xmlFile);

	XMLElement* exits = doc.FirstChildElement( "level" )->FirstChildElement( "exits" );

	Wall tempWall = {0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, NULL, NULL};

	for (XMLElement* exit = exits->FirstChildElement("exit"); exit != NULL; exit = exit->NextSiblingElement("exit"))
	{
		const char* row = exit->Attribute("row");
		const char* col = exit->Attribute("col");
		const char* file = exit->Attribute("file");

		tempWall.row = (float)atof(row);
		tempWall.col = (float)atof(col);
		tempWall.centerX = (float) (tempWall.col + tempWall.xLength / 2);
		tempWall.centerZ = (float) (tempWall.row + tempWall.zLength / 2);
		tempWall.file = file;
	
		exitVector.push_back(tempWall);
	}

	loadRoom(xmlFile);
}

Room::~Room(void)
{

}

void Room::loadRoom(const char* xmlFile)
{
	tinyxml2::XMLDocument doc;

	doc.LoadFile(xmlFile);

	XMLElement* walls = doc.FirstChildElement( "level" )->FirstChildElement( "walls" );
	XMLElement* spawns = doc.FirstChildElement( "level" )->FirstChildElement( "spawns" );

	Wall tempWall = {0.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, NULL, NULL};
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
	
	for (unsigned int i = 0; i < wallRowCol.size(); i++)
	{
		for (unsigned int j = 0; j < wallRowCol[i].size(); j++)
		{
			XMMATRIX matrix = XMMatrixIdentity();

			GameObject* wallObj = new GameObject("Cube", "Test Wall", physicsMan->createRigidBody("Cube", wallRowCol[i][j].centerX, 1.5f, wallRowCol[i][j].centerZ), physicsMan);
			wallObj->scale(wallRowCol[i][j].xLength,3.0,wallRowCol[i][j].zLength);
			gameObjs.push_back(wallObj);
		}
	}

	#pragma endregion
}