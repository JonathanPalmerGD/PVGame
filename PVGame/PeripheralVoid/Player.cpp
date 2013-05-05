#include "Player.h"

Player::Player(PhysicsManager* pm, RenderManager* rm, RiftManager* riftM) 
	: PIXELS_PER_SEC(10.0f), LOOK_SPEED(3.5f)
//Player::Player(PhysicsManager* pm) : PIXELS_PER_SEC(10.0f), LOOK_SPEED(3.5f)
{
	// Build the view matrix. Now done in init because we only need to set it once.
	XMVECTOR aPos = XMVectorSet(0.0f, 1.727f, 0.0f, 1.0f);
	XMVECTOR aUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR aFwd = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR aRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMStoreFloat4(&position, aPos);
	XMStoreFloat3(&up, aUp);
	XMStoreFloat3(&fwd, aFwd);
	XMStoreFloat3(&right, aRight);

	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f);
	physicsMan = pm;
	playerCamera = new Camera(physicsMan, 1.3333334f);
	playerCamera->LookAt(aPos, target, aUp);
	playerCamera->UpdateViewMatrix();

	leftCamera = new Camera(*playerCamera);
	rightCamera = new Camera(*playerCamera);
	
	controller = physicsMan->createCharacterController( 0.4f, 1.0f, .1f);
	//controller = physicsMan->createCharacterController( 1.0f, .3f, .025f);
	controller->setGravity(30.0f);
	controller->setJumpSpeed(15.0f);
	controller->setMaxJumpHeight(50.0f);
	controller->setMaxSlope(3.0f * 3.1415f);

	medusaStatus = false;
	leapStatus = false;
	mobilityStatus = false;
	medusaPercent = 0.0f;

	eyeDist = 0.5f;

	renderMan = rm;

	listener = new AudioListener();
	//listener->mute();
	audioSource = new AudioSource();
	audioSource->initialize("Audio\\Jump.wav", AudioSource::WAV);

	//OCULUS RIFT
	riftMan = riftM;

	EyeYaw = 0;
	EyePitch = 0;
	EyeRoll = 0;
	yaw = 0;
}

void Player::Update(float dt, Input* input)
{
	if(controller->onGround())
	{
		if(leapStatus)
		{
			controller->setMaxJumpHeight(60.0f);
			controller->setJumpSpeed(22.0f);
		}
		else
		{
			controller->setJumpSpeed(15.0f);
			controller->setMaxJumpHeight(30.0f);
		}
	}
	
	playerSpeed = (physicsMan->getStepSize()) * PIXELS_PER_SEC;
	camLookSpeed = dt * LOOK_SPEED;
	this->HandleInput(input);
}

void Player::HandleInput(Input* input)
{
	#pragma region Camera Input
	
	#pragma region Oculus Rift Look controls
	if(riftMan->isRiftConnected())
	{
		//Get head orientation from rift
		Quatf hmdOrient = riftMan->getOrientation();
		hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&EyeYaw, &EyePitch, &EyeRoll);

		//Get extra orientation abilities from mouse, only yaw
		//(its much nicer than having to turn your body when trying to play)
		float dx = XMConvertToRadians(0.25f*static_cast<float>(input->getMouseX() - mLastMousePos.x));
		yaw += dx/MOUSESENSITIVITY;
		input->centerMouse();
		mLastMousePos.x = input->getMouseX();

		//Get extra orientation abilities from keyboard/gamepad, only yaw
		//(its much nicer than having to turn your body when trying to play)
		if (input->isCameraRightKeyDown())
		{
			float angle = camLookSpeed / 2;
			yaw += angle;
		}
		if (input->isCameraLeftKeyDown())
		{
			float angle = -camLookSpeed / 2;
			yaw += angle;
		}

		//Set the rotation of the player
		XMMATRIX R = XMMatrixRotationY(-EyeYaw+yaw);
		up    = XMFLOAT3(0.0f, 1.0f, 0.0f);
		right = XMFLOAT3(1.0f, 0.0f, 0.0f);
		fwd   = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));

		//Set the rotation of the camera
		playerCamera->setRotation(EyeYaw-yaw, EyePitch, EyeRoll);
	}
#pragma endregion
	else
	{
		#pragma region Mouse Controls
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(input->getMouseX() - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(input->getMouseY() - mLastMousePos.y));

		// Update angles based on input
		mTheta += dx/MOUSESENSITIVITY;
		mPhi   += dy/MOUSESENSITIVITY;
		playerCamera->Pitch(mPhi); // Rotate the camera  up/down.

		playerCamera->RotateY(mTheta); // Rotate ABOUT the y-axis. So really turning left/right.
		XMMATRIX R = XMMatrixRotationY(mTheta);

		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
		mPhi = MathHelper::Clamp(dy, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);
		mTheta = MathHelper::Clamp(dx, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);

		playerCamera->UpdateViewMatrix();
	
		input->centerMouse();

		mLastMousePos.x = input->getMouseX();
		mLastMousePos.y = input->getMouseY();
		#pragma endregion

		// Now check for camera input.
		if (input->isCameraUpKeyDown())
		{
			playerCamera->Pitch(-camLookSpeed / 2);
		}
		if (input->isCameraDownKeyDown())
		{
			playerCamera->Pitch(camLookSpeed/ 2);
		}

		if (input->isCameraLeftKeyDown())
		{
			float angle = -camLookSpeed / 2;
			playerCamera->RotateY(angle);
			XMMATRIX R = XMMatrixRotationY(angle);

			XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
			XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
			XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
			//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
		}
		if (input->isCameraRightKeyDown())
		{
			float angle = camLookSpeed / 2;
			playerCamera->RotateY(angle);
			XMMATRIX R = XMMatrixRotationY(angle);

			XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
			XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
			XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
			//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
		}
		#pragma endregion
	}
	#pragma endregion

	//XMVECTOR tempPosition = XMLoadFloat4(&position);

	//// Checking for position movement input, will want to move to separate method later.
	//if (input->isPlayerUpKeyDown())
	//	tempPosition = XMVectorAdd(tempPosition, XMLoadFloat3(&fwd) * playerSpeed);
	//if (input->isPlayerDownKeyDown())
	//	tempPosition = XMVectorAdd(tempPosition, -XMLoadFloat3(&fwd) * playerSpeed);
	//if (input->isPlayerLeftKeyDown())
	//	tempPosition = XMVectorAdd(tempPosition, -XMLoadFloat3(&right) * playerSpeed);
	//if (input->isPlayerRightKeyDown())
	//	tempPosition = XMVectorAdd(tempPosition, XMLoadFloat3(&right) * playerSpeed);

	btVector3 direction(0,0,0);
	btVector3 forward(fwd.x, fwd.y, fwd.z);
	btVector3 r(right.x, right.y, right.z);

	#pragma region Player Controls
	if(input->isPlayerUpKeyDown()) //if(input->isPlayerUpKeyDown() && !medusaStatus)
		direction += forward;
	if(input->isPlayerDownKeyDown()) //if(input->isPlayerDownKeyDown() && !medusaStatus)
		direction -= forward;
	if(input->isPlayerRightKeyDown()) //if(input->isPlayerRightKeyDown() && !medusaStatus)
		direction += r;
	if(input->isPlayerLeftKeyDown()) //if(input->isPlayerLeftKeyDown() && !medusaStatus)
		direction -= r;
	if(input->wasJumpKeyPressed() && !medusaStatus)
	{
		if(audioSource != NULL && !audioSource->isPlaying() && controller->canJump())
		{
			audioSource->setPosition(controller->getGhostObject()->getWorldTransform().getOrigin().getX(),
				controller->getGhostObject()->getWorldTransform().getOrigin().getY(),
				controller->getGhostObject()->getWorldTransform().getOrigin().getZ());
			audioSource->play();
		}
		controller->jump();
	}
	#pragma endregion

	//DBOUT(controller->canJump());
	float currentPlayerSpeed = 0;
	if(!controller->onGround())
	{
		currentPlayerSpeed = (playerSpeed + (playerSpeed * (MOBILITY_MULTIPLIER * mobilityStatus)));
	}
	else
	{
		currentPlayerSpeed = (playerSpeed + (playerSpeed * (MOBILITY_MULTIPLIER * mobilityStatus))) * (1.0f - medusaPercent);
	}
	controller->setWalkDirection(direction * currentPlayerSpeed);

	btVector3 pos = controller->getGhostObject()->getWorldTransform().getOrigin();
	XMFLOAT3 cPos(pos.getX(), pos.getY() + 1, pos.getZ());
	playerCamera->SetPosition(cPos);

	#pragma region Audio
	listener->setPosition(cPos.x, cPos.y, cPos.z);
	listener->setOrientation(-playerCamera->GetLook().x, -playerCamera->GetLook().y, -playerCamera->GetLook().z, playerCamera->GetUp().x, playerCamera->GetUp().y, playerCamera->GetUp().z);

	#pragma endregion
	//XMFLOAT4 unit(0.0f, 0.0f, 0.0f, 1.0f);
	
	/*XMFLOAT3 pos;
	XMStoreFloat3(&pos, tempPosition);
	playerCamera->SetPosition(pos);
	XMStoreFloat4(&position, tempPosition);*/

	/*
	wstring temp = L"";
	stringstream ss;
	ss << "Player position - X: " << position.x << ", Y: " << position.y << ", Z: " << position.z << endl;
	char * cstr = new char [ss.str().length()+1];
	std::strcpy (cstr, ss.str().c_str());

	OutputDebugStringA(cstr);

	delete cstr;
	*/
	playerCamera->UpdateViewMatrix();
	*leftCamera = *playerCamera;
	*rightCamera = *playerCamera;

	float halfIPD = 0.032000002f;
	if(riftMan->isRiftConnected())
		halfIPD = riftMan->getHMDInfo().EyeDistance * eyeDist;

	XMFLOAT3 translation = XMFLOAT3(playerCamera->GetRight().x * halfIPD, playerCamera->GetRight().y * halfIPD,playerCamera->GetRight().z * halfIPD);

	leftCamera->SetPosition(playerCamera->GetPosition().x - translation.x,
						    playerCamera->GetPosition().y - translation.y,
							playerCamera->GetPosition().z - translation.z);
	
	rightCamera->SetPosition(playerCamera->GetPosition().x + translation.x,
						     playerCamera->GetPosition().y + translation.y,
							 playerCamera->GetPosition().z + translation.z);
	leftCamera->UpdateViewMatrix();
	rightCamera->UpdateViewMatrix();
}

void Player::OnResize(float aspectRatio)
{
	if(riftMan->isRiftConnected())
	{
		riftMan->calcStereo();
		playerCamera->SetLens(0.25f*MathHelper::Pi/*(riftMan->getStereo().GetYFOVRadians()*/, riftMan->getStereo().GetAspect(), 0.01, 1000.0f);
	}
	else
		playerCamera->SetLens(0.25f*MathHelper::Pi, aspectRatio, 0.01f, 1000.0f);
	playerCamera->UpdateViewMatrix();
	
}

XMMATRIX Player::ViewProj() const
{
	return playerCamera->ViewProj();
}

Camera* Player::GetCamera()
{
	return playerCamera;
}

Camera* Player::GetLeftCamera()
{
	return leftCamera;
}

Camera* Player::GetRightCamera()
{
	return rightCamera;
}


void Player::resetStatuses() 
{	
	if(!medusaStatus)
	{
		medusaPercent = 0;
	}
	medusaStatus = false;
	mobilityStatus = false;
	leapStatus = false; 
}

void Player::increaseMedusaPercent()
{
	if(medusaStatus && medusaPercent < 1.0f)
	{
		medusaPercent = .9f;
	}
}

void Player::setMobilityStatus(bool newStatus) { mobilityStatus = newStatus; }
void Player::setMedusaStatus(bool newStatus) { medusaStatus = newStatus; }
void Player::setLeapStatus(bool newStatus) { leapStatus = newStatus; }

bool Player::getMobilityStatus() { return mobilityStatus; }
bool Player::getMedusaStatus() { return medusaStatus; }
bool Player::getLeapStatus() { return leapStatus; }

XMFLOAT4 Player::getPosition()
{
	btVector3 pos = controller->getGhostObject()->getWorldTransform().getOrigin();
	return  XMFLOAT4(pos.getX(), pos.getY(), pos.getZ(), 1);
}

AudioListener* Player::getListener()
{
	return listener;
}

void Player::setPosition(float setX, float setY, float setZ)
{
	btVector3 playerPos(setX, setY, setZ);

	controller->getGhostObject()->getWorldTransform().setOrigin(playerPos);
}

btKinematicCharacterController* Player::getController()
{
	return controller;
}

btVector3 Player::getCameraPosition()
{
	XMFLOAT3 pos = playerCamera->GetPosition();
	return btVector3(pos.x, pos.y, pos.z);
}

Player::~Player(void)
{
	physicsMan->removeCharacterController(controller);
	delete listener;
	delete playerCamera;
	delete audioSource;
}
