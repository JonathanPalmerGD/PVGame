#include "PVGame.h"

const map<string, MeshData>MeshMaps::MESH_MAPS = MeshMaps::create_map();

PVGame::PVGame(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	// Initialize smart pointers.
	input = new Input();
	player = new Player();

	gameState = PLAYING;
}


PVGame::~PVGame(void)
{
	// Taken from http://stackoverflow.com/questions/307082/cleaning-up-an-stl-list-vector-of-pointers
	// Thought it was automatically cleaned up but I guess not.
	while (!gameObjects.empty())
	{
		delete gameObjects.back();
		gameObjects.pop_back();
	}
	
	delete player;
	delete physicsMan;
}

bool PVGame::Init()
{
	if (!D3DApp::Init())
		return false;

	physicsMan = new PhysicsManager();

	renderMan->BuildBuffers();

	//Cook Rigid Bodies from the meshes
	map<string, MeshData>::const_iterator itr;
	for(itr = MeshMaps::MESH_MAPS.begin(); itr != MeshMaps::MESH_MAPS.end(); itr++)
	{
		physicsMan->addRigidBodyToMap((*itr).first, (*itr).second, 0.0);
	}

	BuildFX();
	BuildVertexLayout();
	BuildGeometryBuffers();
	
	LoadContent();

	mPhi = 1.5f*MathHelper::Pi;
	mTheta = 0.25f*MathHelper::Pi;
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	SortGameObjects();

	return true;
}

bool PVGame::LoadContent()
{	
	LoadXML();
	return true;
}

bool PVGame::LoadXML()
{
	//Get the filename from constants, hand it into tinyxml
	tinyxml2::XMLDocument doc;

	doc.LoadFile(MAP_LEVEL_1);
	/*static const char* xml = 	
		"<level>"
		"	<walls>"
		"		<wall />"
		"	</walls>"
		"	<spawns>"
		"		<spawn />"
		"	</spawns>"
		"</level>";

	doc.Parse( xml );*/

	XMLElement* walls = doc.FirstChildElement( "level" )->FirstChildElement( "walls" );
	XMLElement* spawns = doc.FirstChildElement( "level" )->FirstChildElement( "spawns" );

	SurfaceMaterial aMaterial;
	aMaterial.Ambient = XMFLOAT4(1.0f, 0.46f, 0.46f, 1.0f);
	aMaterial.Diffuse = XMFLOAT4(1.0f, 0.46f, 0.46f, 1.0f);
	aMaterial.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 16.0f);

	for (XMLElement* wall = walls->FirstChildElement("wall"); wall != NULL; wall = wall->NextSiblingElement("wall"))
	{
		const char* row = wall->Attribute("row");
		const char* col = wall->Attribute("col");

		XMMATRIX matrix = XMMatrixIdentity();
		matrix *= XMMatrixScaling(1.0f, 3.0f, 1.0f);
		matrix *= XMMatrixTranslation((float)atof(col), 1.5f, (float)atof(row));

		//FUCKING RIDICULOUS STUPID AS SHIT CODE, need to improve some how
		//Due to error C2719, a known bug in Visual Studio dealing with stl containers
		map<string, btRigidBody>::iterator ptr = physicsMan->RIGID_BODIES.find("Cube");
		if(ptr != physicsMan->RIGID_BODIES.end())
		{
			const btRigidBody body = ptr->second;
			GameObject* wallObj = new GameObject("Cube", aMaterial, &matrix, body, physicsMan);
			wallObj->translate((float)atof(col), 1.5f, (float)atof(row));
			wallObj->scale(1.0,3.0,1.0);
			gameObjects.push_back(wallObj);
		}
		else
		{
			GameObject* wallObj = new GameObject("Cube", aMaterial, &matrix, physicsMan);
			gameObjects.push_back(wallObj);
		}

		// Debug output
		OutputDebugString(TEXT("\nwall row: "));
		OutputDebugStringA(row);
		OutputDebugString(TEXT(" col: "));
		OutputDebugStringA(col);
	}

	for (XMLElement* spawn = spawns->FirstChildElement("spawn"); spawn != NULL; spawn = spawn->NextSiblingElement("spawn"))
	{
		const char* row = spawn->Attribute("row");
		const char* col = spawn->Attribute("col");

		// GameObject* spawnObj = new GameObject("Spawn", aMaterial, &XMMatrixIdentity());
		// gameObjects.push_back(spawnObj);

		// Debug output
		OutputDebugString(TEXT("\nspawn row: "));
		OutputDebugStringA(row);
		OutputDebugString(TEXT(" col: "));
		OutputDebugStringA(col);
	}

	// Debug output
	OutputDebugString(TEXT("\n"));

	//tinyxml2::XMLElement* titleElement = doc.FirstChildElement( 


//#ifdef NUM_LEVELS == 1		
//	doc.LoadFile( MAP_LEVEL );
//#else
//	//Loop through all the levels
//	for( int i = 0; i < NUM_LEVELS; i++)
//	{
//		//Open the file
//		doc.LoadFile( MAP_LEVELS[i] );
//
//		//Parse through the files
//		
//		/*static const char* xml = "<element/>";
//		XMLDocument doc;
//		doc.Parse( xml );
//		*/
//
//		//Create a new 'Level' or 'Room'
//
//		//Close the file
//
//	}
//#endif

	return true;
}

void PVGame::OnResize()
{
	D3DApp::OnResize();
	player->OnResize(AspectRatio());

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	//XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMStoreFloat4x4(&mProj, P);
}

void PVGame::UpdateScene(float dt)
{
	switch(gameState)
	{
	case MENU:
		if(input->isKeyDown(VK_RETURN))
			gameState = PLAYING;
		break;
	case PLAYING:
		player->HandleInput(input);
		physicsMan->update(dt);
		break;
	default:
		break;
	}
}

void PVGame::OnMouseMove(WPARAM btnState, int x, int y)
{
	// Make each pixel correspond to a quarter of a degree.
	float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
	float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

	// Update angles based on input to orbit camera around box.
	mTheta += dx;
	mPhi   += dy;

	mPhi = MathHelper::Clamp(dy, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);
	
	//camera->Pitch(mPhi); // Rotate the camera  up/down.
	//camera->RotateY(dx / 2); // Rotate ABOUT the y-axis. So really turning left/right.
	//camera->UpdateViewMatrix();

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void PVGame::DrawScene()
{
	switch(gameState)
	{
	case MENU:
		renderMan->DrawMenu();
		break;
	case PLAYING:
		renderMan->DrawScene(player->GetCamera(), gameObjects);
		break;
	default:
		break;
	}
}

void PVGame::BuildGeometryBuffers()
{
	SurfaceMaterial aMaterial;
	aMaterial.Ambient = XMFLOAT4(1.0f, 0.46f, 0.46f, 1.0f);
	aMaterial.Diffuse = XMFLOAT4(1.0f, 0.46f, 0.46f, 1.0f);
	aMaterial.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 16.0f);

	//GameObject* aGameObject = new GameObject("Triangle", aMaterial, &XMMatrixIdentity());
	// gameObjects.push_back(aGameObject);

	//aGameObject = new GameObject("Cube", aMaterial, &XMMatrixIdentity());
	// gameObjects.push_back(aGameObject);

	GameObject* aGameObject = new GameObject("Plane", aMaterial, &(XMMatrixIdentity() * XMMatrixScaling(10.0f, 1.0f, 10.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f)), physicsMan);
	aGameObject->SetRigidBody(physicsMan->createPlane(0,0,0));
	aGameObject->scale(20.0, 1.0, 20.0);
	gameObjects.push_back(aGameObject);

	GameObject* bGameObject = new GameObject("Plane", aMaterial, &(XMMatrixIdentity() * XMMatrixRotationZ(3.14f) * XMMatrixScaling(10.0f, 1.0f, 10.0f) * XMMatrixTranslation(0.0f, 3.0f, 0.0f)), physicsMan);
	bGameObject->SetRigidBody(physicsMan->createPlane(0.0f,0.0f,0.0f));
	bGameObject->scale(20.0f, 1.0f, 20.0f);
	bGameObject->translate(0.0f, 3.0f, 0.0f);
	bGameObject->rotate(1.0f, 0.0f, 0.0f, 0.0f);
	gameObjects.push_back(bGameObject);

	aMaterial.Ambient = XMFLOAT4(0.46f, 0.46f, 0.46f, 1.0f);
	GameObject* testSphere = new GameObject("Sphere", aMaterial, &(XMMatrixIdentity() * XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(5.0f, 1.0f, 0.0f)), physicsMan);
	testSphere->translate(5.0f, 1.0f, 0.0f);
	gameObjects.push_back(testSphere);
}
 
void PVGame::BuildFX()
{
	renderMan->BuildFX();
}

void PVGame::BuildVertexLayout()
{
	renderMan->BuildVertexLayout();
}

// Sorts game objects based on mesh key. Should only be called after a batch of GameObjects are added.
void PVGame::SortGameObjects()
{
	// GameObjectCompaper() is defined in GameObject.h for now. It's a boolean operator.
	sort(gameObjects.begin(), gameObjects.end(), GameObjectComparer());
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	PVGame theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

