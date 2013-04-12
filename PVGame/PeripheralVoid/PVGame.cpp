#include "PVGame.h"

const map<string, MeshData>MeshMaps::MESH_MAPS = MeshMaps::create_map();

PVGame::PVGame(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	// Initialize smart pointers.
	input = new Input();
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
	delete currentRoom;
	alcDestroyContext(audioContext);
    alcCloseDevice(audioDevice);
}

bool PVGame::Init()
{
	if (!D3DApp::Init())
		return false;

	audioDevice=alcOpenDevice(NULL);
    if(audioDevice==NULL)
	{
		//std::cout << "cannot open sound card" << std::endl;
		return 0;
	}
	audioContext=alcCreateContext(audioDevice,NULL);
	if(audioContext==NULL)
	{
		//std::cout << "cannot open context" << std::endl;
		return 0;
	}
	alcMakeContextCurrent(audioContext);
        

	physicsMan = new PhysicsManager();
	player = new Player(physicsMan, renderMan);

	renderMan->BuildBuffers();

	//Cook Rigid Bodies from the meshes
	map<string, MeshData>::const_iterator itr;
	for(itr = MeshMaps::MESH_MAPS.begin(); itr != MeshMaps::MESH_MAPS.end(); itr++)
		physicsMan->addTriangleMesh((*itr).first, (*itr).second);

	BuildFX();
	BuildVertexLayout();
	
	LoadContent();
	BuildGeometryBuffers();

	mPhi = 1.5f*MathHelper::Pi;
	mTheta = 0.25f*MathHelper::Pi;
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	SortGameObjects();

	renderMan->BuildInstancedBuffer(gameObjects);

	return true;
}

bool PVGame::LoadContent()
{	
	renderMan->LoadContent();
	LoadXML();
	return true;
}

bool PVGame::LoadXML()
{
	Room* startRoom = new Room(MAP_LEVEL_1, physicsMan);
	currentRoom = startRoom;

	tinyxml2::XMLDocument doc;

	#pragma region Materials
	doc.LoadFile(MATERIALS_FILE);
	for (XMLElement* material = doc.FirstChildElement("MaterialsList")->FirstChildElement("Material"); 
				material != NULL; material = material->NextSiblingElement("Material"))
	{
		// Loop through all the materials, setting appropriate attributes.
		GameMaterial aMaterial;
		aMaterial.Name = material->Attribute("name");
		aMaterial.SurfaceKey = material->FirstChildElement("SurfaceMaterial")->FirstChild()->Value();
		aMaterial.DiffuseKey = material->FirstChildElement("DiffuseMap")->FirstChild()->Value();
		GAME_MATERIALS[aMaterial.Name] = aMaterial;
	}
	#pragma endregion

	#pragma region Textures
	doc.LoadFile(TEXTURES_FILE);
	for (XMLElement* atlas = doc.FirstChildElement("TextureList")->FirstChildElement("Atlas"); 
				atlas != NULL; atlas = atlas->NextSiblingElement("Atlas"))
	{
		// Tells renderman to load the texture and save it in a map.
		renderMan->LoadTexture(atlas->Attribute("name"), atlas->Attribute("filename"),
			atlas->Attribute("type"));

		for (XMLElement* texture = atlas->FirstChildElement("Texture"); 
			texture != NULL; texture = texture->NextSiblingElement("Texture"))
		{
			XMFLOAT2 aCoord((float)atof(texture->FirstChildElement("OffsetU")->FirstChild()->Value()), 
							(float)atof(texture->FirstChildElement("OffsetV")->FirstChild()->Value()));
			renderMan->LoadTextureAtlasCoord(texture->Attribute("name"), aCoord);
		}
	}
	#pragma endregion

	#pragma region Surface Materials
	doc.LoadFile(SURFACE_MATERIALS_FILE);
	for (XMLElement* surfaceMaterial = doc.FirstChildElement("SurfaceMaterialsList")->FirstChildElement("SurfaceMaterial"); 
				surfaceMaterial != NULL; surfaceMaterial = surfaceMaterial->NextSiblingElement("SurfaceMaterial"))
	{
		// Gets values for surface material from xml.
		XMLElement* ambient = surfaceMaterial->FirstChildElement("Ambient");
		XMLElement* diffuse = surfaceMaterial->FirstChildElement("Diffuse");
		XMLElement* specular = surfaceMaterial->FirstChildElement("Specular");
		XMLElement* reflect = surfaceMaterial->FirstChildElement("Reflect");
		
		SurfaceMaterial aMaterial;
		aMaterial.Ambient = XMFLOAT4((float)atof(ambient->Attribute("r")), (float)atof(ambient->Attribute("g")), 
									 (float)atof(ambient->Attribute("b")), (float)atof(ambient->Attribute("a")));
		aMaterial.Diffuse = XMFLOAT4((float)atof(diffuse->Attribute("r")), (float)atof(diffuse->Attribute("g")), 
									 (float)atof(diffuse->Attribute("b")), (float)atof(diffuse->Attribute("a")));
		aMaterial.Specular = XMFLOAT4((float)atof(specular->Attribute("r")), (float)atof(specular->Attribute("g")), 
									 (float)atof(specular->Attribute("b")), (float)atof(specular->Attribute("a")));
		aMaterial.Reflect = XMFLOAT4((float)atof(reflect->Attribute("r")), (float)atof(reflect->Attribute("g")), 
									 (float)atof(reflect->Attribute("b")), (float)atof(reflect->Attribute("a")));
		SURFACE_MATERIALS[surfaceMaterial->Attribute("name")] = aMaterial;
	}
	#pragma endregion

	#pragma region Map Loading
	//Get the filename from constants, hand it into tinyxml

	gameObjects = startRoom->getGameObjs();
	player->setPosition(currentRoom->getSpawn().col, 2.0f, currentRoom->getSpawn().row);

	GameObject* crestObj = new Crest("Cube", "Test Wood", physicsMan->createRigidBody("Cube", 8.4f, 10.0f, 8.4f, 1.0f), physicsMan, LEAP, 1.0f);
	gameObjects.push_back(crestObj);
	
	GameObject* crestObj2 = new Crest("Cube", "Test Wood", physicsMan->createRigidBody("Cube", 20.7f, 4.0f, 20.7f, 1.0f), physicsMan, MEDUSA, 1.0f);
	gameObjects.push_back(crestObj2);

	GameObject* crestObj3 = new Crest("Cube", "Test Wood", physicsMan->createRigidBody("Cube", -16.0f, 10.0f, -16.0f, 1.0f), physicsMan, MOBILITY, 1.0f);
	gameObjects.push_back(crestObj3);
	#pragma endregion

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

bool is1Up = true;
bool is2Up = true;
void PVGame::UpdateScene(float dt)
{
	switch(gameState)
	{
	case MENU:
		if(input->isKeyDown(VK_RETURN))
			gameState = PLAYING;
		break;
	case PLAYING:
		if (input->isQuitPressed())
			PostMessage(this->mhMainWnd, WM_CLOSE, 0, 0);

		player->Update(dt, input);

		// If physics updated, tell the game objects to update their world matrix.
		if (physicsMan->update(dt))
		{
			for (unsigned int i = 0; i < gameObjects.size(); ++i)
				gameObjects[i]->Update();
		}

		if (player->getPosition().y < -20)
			player->setPosition(currentRoom->getSpawn().col, 2.0f, currentRoom->getSpawn().row);

		/*for(int i = 0; i < renderMan->getNumLights(); ++i)
		{
			btVector3 playerV3(player->getPosition().x, player->getPosition().y, player->getPosition().z);
			btVector3 lightPos = renderMan->getLightPosition(i);
			//btVector3 targetV3 = &lightPos;
			if(physicsMan->broadPhase(player->GetCamera(), &lightPos))
			{
				//renderMan->EnableLight(i);
			}
			else
			{
				//renderMan->DisableLight(i);
			}
		}*/
		
		#pragma region Player Statuses and Crest Checking
		player->resetStatuses();

		for(unsigned int i = 0; i < gameObjects.size(); i++)
		{
			if(gameObjects[i]->GetVisionAffected())
			{	
				//If it is a crest
					//And if it is colliding
				if(Crest* currentCrest = dynamic_cast<Crest*>(gameObjects[i]))
				{
					btVector3 crestPos = gameObjects[i]->getRigidBody()->getCenterOfMassPosition();
					#pragma region Switch for Crest Types
					switch(currentCrest->GetCrestType())
					{
						#pragma region Medusa Crest
						case MEDUSA: //GREEN
							//Increment the player's movement speed.
							renderMan->SetLightPosition(1, &crestPos);

							if(physicsMan->broadPhase(player->GetCamera(), &crestPos) && physicsMan->narrowPhase(player->GetCamera(), gameObjects[i]))
							{	
								renderMan->EnableLight(1);
								player->setMedusaStatus(true);
								player->increaseMedusaPercent();
							}
							else
							{
								renderMan->DisableLight(1);
							}
							break;
						#pragma endregion
						#pragma region Leap Crest
						case LEAP:	//RED
							//Increase the player's jump variable.
							renderMan->SetLightPosition(0, &crestPos);

							if(physicsMan->broadPhase(player->GetCamera(), &crestPos) && physicsMan->narrowPhase(player->GetCamera(), gameObjects[i]))
							{	
								renderMan->EnableLight(0);
								player->setLeapStatus(true);
							}
							else
							{
								renderMan->DisableLight(0);
							}
							break;
						#pragma endregion
						#pragma region Mobility Crest
						case MOBILITY:	//BLUE
							//Increase the player's movement speed.
							renderMan->SetLightPosition(2, &crestPos);
							if(physicsMan->broadPhase(player->GetCamera(), &crestPos) && physicsMan->narrowPhase(player->GetCamera(), gameObjects[i]))
							{	
								renderMan->EnableLight(2);
								player->setMobilityStatus(true);
							}
							else
							{
								renderMan->DisableLight(2);
							}
							break;
						#pragma endregion
						#pragma region Unlock Crest
						case UNLOCK:
							renderMan->SetLightPosition(3, &crestPos);
							//Change the unlocking object to the unlocked state.
							break;
						#pragma endregion
					}
					#pragma endregion
					
				}
			}
		}
		#pragma endregion

		#pragma region Outdated Light Code
		/*if(input->wasKeyPressed('9'))
		{
			renderMan->ToggleLight(0);
		}
		if(input->wasKeyPressed('8'))
		{
			renderMan->ToggleLight(1);
		}
		if(input->wasKeyPressed('7'))
		{
			renderMan->ToggleLight(2);
		}
		if(input->wasKeyPressed('6'))
		{
			renderMan->ToggleLight(3);
		}*/
		#pragma endregion

		if(renderMan->getNumLights() > 3)
		{
			renderMan->SetLightPosition(3, &player->getCameraPosition());
		}
		if(input->wasKeyPressed('Q'))
		{
			if(renderMan->getNumLights() < 4)
			{
				XMFLOAT4 p = player->getPosition();
				XMFLOAT3 look = player->GetCamera()->GetLook();
				XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
				renderMan->CreateLight(XMFLOAT4(0.00f, 0.00f, 0.00f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), 25.0f, pos, XMFLOAT3(0.0f, 0.0f, 0.2f));
			}
			else
			{
				renderMan->ToggleLight(3);
			}
		}

		if((input->isKeyDown('1') || input->getGamepadLeftTrigger(0)) && is1Up)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;

			GameObject* testSphere = new GameObject("Sphere", "Test Wood", physicsMan->createRigidBody("Sphere", pos.x, pos.y, pos.z, .3f, 0.3f, 0.3f, 1.0f), physicsMan, 1.0f);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
			
			//testSphere->playAudio();
			gameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			is1Up = false;
		}
		else if(!input->isKeyDown('1') && !input->getGamepadLeftTrigger(0))
			is1Up = true;

		if((input->isKeyDown('2') || input->getGamepadRightTrigger(0))  && is2Up)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;

			GameObject* testSphere = new GameObject("Cube", "Test Wood", physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0), physicsMan, 1.0);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
			//testSphere->playAudio();
			gameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			is2Up = false;
		}
		else if(!input->isKeyDown('2') && !input->getGamepadRightTrigger(0))
			is2Up = true;

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
	//GameObject* aGameObject = new GameObject("Triangle", aMaterial, &XMMatrixIdentity());
	// gameObjects.push_back(aGameObject);

	//aGameObject = new GameObject("Cube", aMaterial, &XMMatrixIdentity());
	// gameObjects.push_back(aGameObject);

	GameObject* aGameObject = new GameObject("Plane", "Test Wood", &(XMMatrixIdentity() * XMMatrixScaling(80.0f, 1.0f, 80.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f)), physicsMan);
	aGameObject->SetRigidBody(physicsMan->createPlane(0,0,0));
	aGameObject->scale(80.0, 1.0, 80.0);
	gameObjects.push_back(aGameObject);

	/*GameObject* bGameObject = new GameObject("Plane", aMaterial, &(XMMatrixIdentity() * XMMatrixRotationZ(3.14f) * XMMatrixScaling(10.0f, 1.0f, 10.0f) * XMMatrixTranslation(0.0f, 3.0f, 0.0f)), physicsMan);
	bGameObject->SetRigidBody(physicsMan->createPlane(0.0f,0.0f,0.0f));
	bGameObject->scale(20.0f, 1.0f, 20.0f);
	bGameObject->translate(0.0f, 3.0f, 0.0f);
	bGameObject->rotate(1.0f, 0.0f, 0.0f, 0.0f);
	gameObjects.push_back(bGameObject);*/

	//aMaterial.Ambient = XMFLOAT4(0.46f, 0.46f, 0.46f, 1.0f);
	//GameObject* testSphere = new GameObject("Sphere", aMaterial, &(XMMatrixIdentity() * XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(5.0f, 1.0f, 0.0f)), 
	//										physicsMan->createRigidBody("Sphere", 5.0, 2.0, 0.0, 1.0), physicsMan);
	////testSphere->translate(5.0f, 2.0f, 0.0f);
	//testSphere->scale(0.5, 0.5, 0.5);
	//testSphere->setLinearVelocity(0,0,0);
	//gameObjects.push_back(testSphere);
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

