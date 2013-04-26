#include "PVGame.h"

map<string, MeshData>MeshMaps::MESH_MAPS = MeshMaps::create_map();

PVGame::PVGame(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	// Initialize smart pointers.
	input = new Input();
	gameState = MENU;
}


PVGame::~PVGame(void)
{
	delete player;
	
	for (unsigned int i = 0; i < proceduralGameObjects.size(); ++i)
	{
		delete proceduralGameObjects[i];
	}

	gameObjects.clear();
	proceduralGameObjects.clear();
	
	ClearRooms();	

	alcDestroyContext(audioContext);
    alcCloseDevice(audioDevice);
	delete physicsMan;
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
	
	//Test load a cube.obj
	//renderMan->LoadFile(L"crest.obj", "crest");
	renderMan->LoadFile(L"medusacrest.obj", "medusacrest");

	renderMan->BuildBuffers();

	//Cook Rigid Bodies from the meshes
	map<string, MeshData>::const_iterator itr;
	for(itr = MeshMaps::MESH_MAPS.begin(); itr != MeshMaps::MESH_MAPS.end(); itr++)
		physicsMan->addTriangleMesh((*itr).first, (*itr).second);

	BuildFX();
	BuildVertexLayout();
	
	LoadContent();
	BuildGeometryBuffers();

#if DRAW_FRUSTUM
		gameObjects.push_back(player->GetCamera()->frustumBody);
		proceduralGameObjects.push_back(player->GetCamera()->frustumBody);
#endif

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
	Room* startRoom = new Room(MAP_LEVEL_1, physicsMan, 0, 0);
	startRoom->loadRoom();
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

		XMLElement* glow = material->FirstChildElement("Glow");
		if (glow)
			aMaterial.GlowColor = XMFLOAT4((float)atof(glow->Attribute("r")), (float)atof(glow->Attribute("g")), 
									 (float)atof(glow->Attribute("b")), (float)atof(glow->Attribute("a")));
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
	BuildRooms(currentRoom);

	player->setPosition(currentRoom->getSpawn()->col, 2.0f, currentRoom->getSpawn()->row);
	#pragma endregion

	SortGameObjects();

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

#pragma region Awful use of variables courtesy of Jason
bool is1Up = true;
bool is2Up = true;
bool is8Up = true;
bool isEUp = true;
#pragma endregion
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
		#pragma region Player Wireframe and blur controls
		if (input->wasKeyPressed('R'))
			renderMan->AddPostProcessingEffect(WireframeEffect);
		if (input->wasKeyPressed('N'))
			renderMan->RemovePostProcessingEffect(WireframeEffect);

		if (input->wasKeyPressed('B'))
			renderMan->AddPostProcessingEffect(BlurEffect);
		if (input->wasKeyPressed('V'))
			renderMan->RemovePostProcessingEffect(BlurEffect);

		if (input->wasKeyPressed(VK_OEM_6))
			renderMan->ChangeBlurCount(1);
		if (input->wasKeyPressed(VK_OEM_4))
			renderMan->ChangeBlurCount(-1);
		#pragma endregion
		#pragma region Physics for Worlds Game Objects
		// If physics updated, tell the game objects to update their world matrix.
		if (physicsMan->update(dt))
		{
			for (unsigned int i = 0; i < gameObjects.size(); ++i)
			{
				gameObjects[i]->Update();
				/* //Should delete objects below -20. Doesn't work 'well' or 'at all'
				if(gameObjects[i]->getRigidBody()->getWorldTransform().getOrigin().getY() < -20)
				{
				gameObjects.erase(gameObjects.begin() += i);

				SortGameObjects();
				}*/
			}
		}
		#pragma endregion
		#if USE_FRUSTUM_CULLING
		player->GetCamera()->frustumCull();
		#endif

		#pragma region Player Room Tracking and Resetting to Checkpoints
		for (unsigned int i = 0; i < loadedRooms.size(); i++)
		{
			if ((player->getPosition().x > loadedRooms[i]->getX()) && (player->getPosition().x < (loadedRooms[i]->getX() + loadedRooms[i]->getWidth())) &&
				(player->getPosition().z > loadedRooms[i]->getZ()) && (player->getPosition().z < (loadedRooms[i]->getZ() + loadedRooms[i]->getDepth())))
			{
					currentRoom = loadedRooms[i];
					break;
			}
		}
		//If the player falls of the edge of the world, respawn in current room
		if (player->getPosition().y < -20)
			player->setPosition((currentRoom->getX() + currentRoom->getSpawn()->centerX), 2.0f, (currentRoom->getZ() + currentRoom->getSpawn()->centerZ));
		#pragma endregion
		#pragma region Player Statuses and Crest Checking
		player->resetStatuses();

		// Reset blur, we only do it if a single Medusa is in sight.
		renderMan->RemovePostProcessingEffect(BlurEffect);

		for(unsigned int i = 0; i < gameObjects.size(); i++)
		{
			if(gameObjects[i]->GetVisionAffected())
			{	
				//If it is a crest
					//And if it is colliding
				if(MovingObject* currentMovObj = dynamic_cast<MovingObject*>(gameObjects[i]))
				{
					currentMovObj->Update();
				}
				
				if(Crest* currentCrest = dynamic_cast<Crest*>(gameObjects[i]))
				{
					btVector3 crestPos = gameObjects[i]->getRigidBody()->getCenterOfMassPosition();
					
					if(physicsMan->broadPhase(player->GetCamera(), gameObjects[i]) && physicsMan->narrowPhase(player->GetCamera(), gameObjects[i]))
					{
						currentCrest->ChangeView(true);

						// For now, only Medusa causes blur effect.
						if (currentCrest->GetCrestType() == MEDUSA && player->getController()->onGround())
						{
							renderMan->SetBlurColor(XMFLOAT4(0.0f, 0.25f, 0.0f, 1.0f));
							renderMan->AddPostProcessingEffect(BlurEffect);
						}
					}
					else
					{
						currentCrest->ChangeView(false);

						// If Medusa is out of sight, remove blur. Overrides manual blur add - comment out to require manual toggle on/off.
						if (currentCrest->GetCrestType() == MEDUSA)
						{
							renderMan->RemovePostProcessingEffect(BlurEffect);
						}
					}
					currentCrest->Update(player);
				}
			}
		}
		#pragma endregion

		#pragma region Throwing Crests
		if(input->wasKeyPressed('3'))
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;
			GameObject* crestObj = new Crest("Cube", Crest::GetCrestTypeString(MOBILITY), physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0f), physicsMan, MOBILITY, 1.0f);
			crestObj->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			gameObjects.push_back(crestObj);
			proceduralGameObjects.push_back(crestObj);
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
		}
		if(input->wasKeyPressed('4'))
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;
			GameObject* crestObj = new Crest("Cube", Crest::GetCrestTypeString(LEAP), physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0f), physicsMan, LEAP, 1.0f);
			crestObj->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			gameObjects.push_back(crestObj);
			proceduralGameObjects.push_back(crestObj);
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
		}
		/*
		if(input->wasKeyPressed('5'))
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;
			GameObject* crestObj = new Crest("Cube", "Test Wood", physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0f), physicsMan, UNLOCK, 1.0f);
			crestObj->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			gameObjects.push_back(crestObj);
			renderMan->BuildInstancedBuffer(gameObjects);
		}*/
		if(input->wasKeyPressed('9'))
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 15;
			GameObject* crestObj = new Crest("Cube", Crest::GetCrestTypeString(MEDUSA), physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0f), physicsMan, MEDUSA, 1.0f);
			crestObj->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			gameObjects.push_back(crestObj);
			proceduralGameObjects.push_back(crestObj);
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
		}
		#pragma endregion

		#pragma region Outdated Light Code
		/*if(renderMan->getNumLights() > 3)
		{
			renderMan->SetLightPosition(3, &player->getCameraPosition());
		}*/
		if(input->wasKeyPressed('Q'))
		{
			/*if(renderMan->getNumLights() < 4)
			{
				XMFLOAT4 p = player->getPosition();
				XMFLOAT3 look = player->GetCamera()->GetLook();
				XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
				renderMan->CreateLight(XMFLOAT4(0.00f, 0.00f, 0.00f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), 25.0f, pos, XMFLOAT3(0.0f, 0.0f, 0.2f));
			}
			else
			{
				renderMan->ToggleLight(3);
			}*/
		}
		#pragma	endregion
		#pragma region 1: Slow Sphere
		if((input->wasKeyPressed('1') || input->getGamepadLeftTrigger(0)) && is1Up)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 10;

			GameObject* testSphere = new GameObject("Sphere", "Test Wood", physicsMan->createRigidBody("Sphere", pos.x, pos.y, pos.z, 0.3f, 0.3f, 0.3f, 1.0f), physicsMan, WORLD, 1.0f);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\shot.wav");
			
			testSphere->playAudio();
			gameObjects.push_back(testSphere);
			proceduralGameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
			is1Up = false;
		}
		else if(!input->isKeyDown('1') && !input->getGamepadLeftTrigger(0))
			is1Up = true;
		#pragma endregion
		#pragma region 2: Slow Cube
		if((input->wasKeyPressed('2') || input->getGamepadRightTrigger(0))  && is2Up)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 20;

			GameObject* testSphere = new GameObject("Cube", "Test Wood", physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0), physicsMan, WORLD, 1.0);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
			//testSphere->playAudio();
			gameObjects.push_back(testSphere);
			proceduralGameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
			is2Up = false;
		}
		else if(!input->isKeyDown('2') && !input->getGamepadRightTrigger(0))
			is2Up = true;
		#pragma endregion
		#pragma region 8: Speedless Cube
		if((input->wasKeyPressed('8'))  && is8Up)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 0;

			GameObject* testSphere = new GameObject("Cube", "Test Wood", physicsMan->createRigidBody("Cube", pos.x, pos.y, pos.z, 1.0), physicsMan, WORLD, 1.0);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\test_mono_8000Hz_8bit_PCM.wav");
			//testSphere->playAudio();
			gameObjects.push_back(testSphere);
			proceduralGameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
			is8Up = false;
		}
		else if(!input->isKeyDown('8'))
			is8Up = true;
		#pragma endregion
		#pragma region E: Fast Sphere
		if((input->wasKeyPressed('E'))  && isEUp)
		{
			XMFLOAT4 p = player->getPosition();
			XMFLOAT3 look = player->GetCamera()->GetLook();
			XMFLOAT3 pos(p.x + (look.x * 2),p.y + (look.y * 2),p.z + (look.z * 2));
			float speed = 90;

			GameObject* testSphere = new GameObject("Sphere", "Test Wood", physicsMan->createRigidBody("Sphere", pos.x, pos.y, pos.z, 0.3f, 0.3f, 0.3f, 90.0f), physicsMan, WORLD, 90.0f);
			testSphere->setLinearVelocity(look.x * speed, look.y * speed, look.z * speed);
			testSphere->initAudio("Audio\\shot.wav");
			testSphere->playAudio();
			gameObjects.push_back(testSphere);
			proceduralGameObjects.push_back(testSphere);
			SortGameObjects();
			renderMan->BuildInstancedBuffer(gameObjects);
			SortGameObjects();
			isEUp = false;
		}
		else if(!input->isKeyDown('E'))
			isEUp = true;
		#pragma endregion
		#pragma region Level Controls U and I
		if (input->wasKeyPressed('U'))
		{
			for (unsigned int i = 0; i < loadedRooms.size(); i++)
			{
				if (strcmp(currentRoom->getFile(), loadedRooms[i]->getFile()) == 0)
				{
					if (i > 0)
						currentRoom = loadedRooms[i - 1];
					else
						currentRoom = loadedRooms[loadedRooms.size() - 1];

					player->setPosition((currentRoom->getX() + currentRoom->getSpawn()->centerX), 2.0f, (currentRoom->getZ() + currentRoom->getSpawn()->centerZ));
					break;
				}
			}
		}
		if (input->wasKeyPressed('I'))
		{
			for (unsigned int i = 0; i < loadedRooms.size(); i++)
			{
				if (strcmp(currentRoom->getFile(), loadedRooms[i]->getFile()) == 0)
				{
					if (i < (loadedRooms.size() - 1))
						currentRoom = loadedRooms[i + 1];
					else
						currentRoom = loadedRooms[0];

					player->setPosition((currentRoom->getX() + currentRoom->getSpawn()->centerX), 2.0f, (currentRoom->getZ() + currentRoom->getSpawn()->centerZ));
					break;
				}
			}
		}
		#pragma endregion

#if USE_FRUSTUM_CULLING
		player->GetCamera()->frustumCull();
#endif
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
	
	//player->GetCamera()->Pitch(mPhi); // Rotate the camera  up/down.
	//player->GetCamera()->RotateY(dx / 2); // Rotate ABOUT the y-axis. So really turning left/right.
	//player->GetCamera()->UpdateViewMatrix();

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

	/*GameObject* aGameObject = new GameObject("Plane", "Test Wood", &(XMMatrixIdentity() * XMMatrixScaling(80.0f, 1.0f, 80.0f) * XMMatrixTranslation(0.0f, 0.0f, 0.0f)), physicsMan);
	aGameObject->SetRigidBody(physicsMan->createPlane(0.0f, 0.0f, 0.0f));
	aGameObject->scale(8000.0, 1.0, 8000.0);
	gameObjects.push_back(aGameObject);*/

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

void PVGame::BuildRooms(Room* startRoom)
{
	bool isLoaded = false;

	for (unsigned int i = 0; i < loadedRooms.size(); i++)
	{
		if (strcmp(loadedRooms[i]->getFile(), startRoom->getFile()) == 0)
			isLoaded = true;
	}

	if (!isLoaded)
	{
		for (unsigned int i = 0; i < startRoom->getGameObjs().size(); i++)
		{
			gameObjects.push_back(startRoom->getGameObjs()[i]);
		}

		startRoom->loadNeighbors(loadedRooms);
		loadedRooms.push_back(startRoom);

		for (unsigned int i = 0; i < startRoom->getNeighbors().size(); i++)
		{
			BuildRooms(startRoom->getNeighbors()[i]);
		}
	}
}

void PVGame::ClearRooms()
{
	for (unsigned int i = 0; i < loadedRooms.size(); i++)
	{
		for (unsigned int j = 0; j < loadedRooms[i]->getNeighbors().size(); j++)
		{
			loadedRooms[i]->getNeighbors()[j] = NULL;
		}

		delete loadedRooms[i];
	}
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

