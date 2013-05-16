#include "Player.h"

Player::Player(PhysicsManager* pm, RenderManager* rm, RiftManager* riftM) 
	: PIXELS_PER_SEC(1.4f), LOOK_SPEED(3.5f)
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

	//OCULUS RIFT
	riftMan = riftM;

	//Set the orientation of the camera
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f);
	physicsMan = pm;
	playerCamera = new Camera(physicsMan, riftMan, 1.3333334f);
	playerCamera->LookAt(aPos, target, aUp);
	playerCamera->UpdateViewMatrix();

	//Set up the character controller
	controller = physicsMan->createCharacterController( .4f, 1.0f, .1f);
	//controller = physicsMan->createCharacterController( 1.0f, .3f, .025f);
	controller->setGravity(30.0f);
	controller->setJumpSpeed(9.0f);
	controller->setMaxJumpHeight(50.0f);
	controller->setMaxSlope(3.0f * 3.1415f);

	medusaStatus = false;
	leapStatus = false;
	mobilityStatus = false;
	winStatus = false;
	hephStatus = false;
	medusaPercent = 0.0f;
	winPercent = 0.0f;

	renderMan = rm;

	//Set up audio
	listener = new AudioListener();
	listener->mute();
	audioSource = new AudioSource();
	audioSource->initialize("Audio\\Jump.wav", AudioSource::WAV);

	//Rift Head Tracking variables
	EyeYaw = 0;
	EyePitch = 0;
	EyeRoll = 0;
	yaw = 0;
}

///////////////////////////////////////////////////
// Update()
//
// Handle crest effects and user input
///////////////////////////////////////////////////
void Player::Update(float dt, Input* input)
{
	if(controller->onGround())
	{
		if(leapStatus)
		{
			controller->setMaxJumpHeight(40.0f);
			controller->setJumpSpeed(8.5f);
		}
		else
		{
			controller->setJumpSpeed(5.5f);
			controller->setMaxJumpHeight(20.0f);
		}
	}
	
	//playerSpeed = (physicsMan->getStepSize()) * PIXELS_PER_SEC;
	playerSpeed = dt * PIXELS_PER_SEC;
	//controller->setGravity(1750.0f*dt);
	controller->setGravity(650.0f * physicsMan->getStepSize());
	controller->setFallSpeed(250.0f * dt);
	camLookSpeed = dt * LOOK_SPEED;
	this->HandleInput(input);
	controller->updateAction(physicsMan->getWorld(), dt);
}

////////////////////////////////////////////////////////////
// HandleInput()
//
// Handles input from the user that affects the player
///////////////////////////////////////////////////////////
void Player::HandleInput(Input* input)
{
	#pragma region Camera Input
	
	#pragma region Oculus Rift
	if(riftMan->isUsingRift() && riftMan->isRiftConnected())
	{
		//Get head orientation from rift
		Quatf hmdOrient = riftMan->getOrientation();
		hmdOrient.GetEulerAngles<Axis_Y, Axis_Z, Axis_X, OVR::RotateDirection::Rotate_CCW, OVR::HandedSystem::Handed_R>(&EyeYaw, &EyeRoll, &EyePitch);
		//DBOUT(EyeYaw);
		//DBOUT(EyePitch);
		//DBOUT(EyeRoll);
		//DBOUT("");

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
			yaw += camLookSpeed / 2;
		}
		if (input->isCameraLeftKeyDown())
		{
			yaw += -camLookSpeed / 2;
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

		//Set rotation of the player
		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));

		//Clamp angles reprenting rotations
		mPhi = MathHelper::Clamp(dy, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);
		mTheta = MathHelper::Clamp(dx, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);

		//playerCamera->UpdateViewMatrix();
	
		input->centerMouse();

		mLastMousePos.x = input->getMouseX();
		mLastMousePos.y = input->getMouseY();
		#pragma endregion

		#pragma region Keyboard
		// Now check for keyboard/gamepad input.
		if (input->isCameraUpKeyDown())
		{
			playerCamera->Pitch( -camLookSpeed/2);
		}
		if (input->isCameraDownKeyDown())
		{
			playerCamera->Pitch(camLookSpeed/2);
		}

		if (input->isCameraLeftKeyDown())
		{
			float angle = -camLookSpeed / 2;
			playerCamera->RotateY(angle);
			XMMATRIX R = XMMatrixRotationY(angle);

			XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
			XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
			XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
		}
		if (input->isCameraRightKeyDown())
		{
			float angle = camLookSpeed / 2;
			playerCamera->RotateY(angle);
			XMMATRIX R = XMMatrixRotationY(angle);

			XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
			XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
			XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
		}
		#pragma endregion

		#pragma region Gamepad
		btVector3 direction(0,0,0);
		btVector3 pitch(0.0f, 1.0f, 0.0f);
		btVector3 yaw(1.0f, 0.0f, 0.0f);
		float xScale = 1.0f;
		float zScale = 1.0f;
		if(input->gamepadConnected(0))
		{
			float lX = (float)input->getGamepadThumbRX(0);
			float lY = (float)input->getGamepadThumbRY(0);
			if(lX > GAMEPAD_THUMBSTICK_DEADZONE || lX < -GAMEPAD_THUMBSTICK_DEADZONE)
				direction += yaw * (lX / 30000.0f) * camLookSpeed/2;
			if(lY > GAMEPAD_THUMBSTICK_DEADZONE || lY < -GAMEPAD_THUMBSTICK_DEADZONE)
				direction += pitch * (lY/ 30000.0f) * camLookSpeed/2;
		
			////Scale speed of player look based on thumbsticks
			//if(lX > GAMEPAD_THUMBSTICK_DEADZONE || lX < -GAMEPAD_THUMBSTICK_DEADZONE)
			//	xScale = ((float)(input->getGamepadThumbLX(0)) / 30000.0f);
			//if(lY > GAMEPAD_THUMBSTICK_DEADZONE || lY < -GAMEPAD_THUMBSTICK_DEADZONE)
			//	zScale = ((float)(input->getGamepadThumbLY(0)) / 30000.0f);
		}

		playerCamera->Pitch(-direction.getY());
		playerCamera->RotateY(direction.getX());
		XMMATRIX rot = XMMatrixRotationY(direction.getX());

		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), rot));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), rot));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), rot));
		#pragma endregion
	}
	#pragma endregion

	#pragma region Player Movement
	
	btVector3 direction(0,0,0);
	btVector3 forward(fwd.x, fwd.y, fwd.z);
	btVector3 r(right.x, right.y, right.z);

	#pragma region Keyboard
	if(input->isPlayerUpKeyDown()) //if(input->isPlayerUpKeyDown() && !medusaStatus)
		direction += forward;
	if(input->isPlayerDownKeyDown()) //if(input->isPlayerDownKeyDown() && !medusaStatus)
		direction -= forward;
	if(input->isPlayerRightKeyDown()) //if(input->isPlayerRightKeyDown() && !medusaStatus)
		direction += r;
	if(input->isPlayerLeftKeyDown()) //if(input->isPlayerLeftKeyDown() && !medusaStatus)
		direction -= r;
	#pragma endregion
	
	#pragma region Gamepad
	float xScale = 1.0f;
	float zScale = 1.0f;
	if(input->gamepadConnected(0))
	{
		float lX = (float)input->getGamepadThumbLX(0);
		float lY = (float)input->getGamepadThumbLY(0);
		if(lX > GAMEPAD_THUMBSTICK_DEADZONE || lX < -GAMEPAD_THUMBSTICK_DEADZONE)
			direction += r * (lX / 30000.0f);
		if(lY > GAMEPAD_THUMBSTICK_DEADZONE || lY < -GAMEPAD_THUMBSTICK_DEADZONE)
			direction += forward * (lY/ 30000.0f);
		
		if(direction.length2() > 0)
			direction = direction.normalize();

		//Scale speed of player based on thumbsticks
		if(lX > GAMEPAD_THUMBSTICK_DEADZONE || lX < -GAMEPAD_THUMBSTICK_DEADZONE)
			xScale = ((float)(input->getGamepadThumbLX(0)) / 30000.0f);
		if(lY > GAMEPAD_THUMBSTICK_DEADZONE || lY < -GAMEPAD_THUMBSTICK_DEADZONE)
			zScale = ((float)(input->getGamepadThumbLY(0)) / 30000.0f);

	}
	else if(direction.length2() > 0)
		direction = direction.normalize();
	#pragma endregion

	#pragma region Jump
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
	#pragma endregion
	
	#pragma region Final Calculation and Movement
	//calculate final speed
	currentPlayerSpeed = currentPlayerSpeed * sqrt((xScale * xScale) + (zScale * zScale));
	//if(currentPlayerSpeed > .18203889f)//clamp speed
	//	currentPlayerSpeed = 0.18203889f;
	direction *= currentPlayerSpeed;
	controller->setWalkDirection(direction);

	//Set the camera's position
	btVector3 pos = controller->getGhostObject()->getWorldTransform().getOrigin();
	XMFLOAT3 cPos(pos.getX(), pos.getY() + 0.737, pos.getZ());
	playerCamera->SetPosition(cPos);
	#pragma endregion
	#pragma endregion
	
	#pragma region Audio
	listener->setPosition(cPos.x, cPos.y, cPos.z);
	listener->setOrientation(-playerCamera->GetLook().x, -playerCamera->GetLook().y, -playerCamera->GetLook().z, playerCamera->GetUp().x, playerCamera->GetUp().y, playerCamera->GetUp().z);
	#pragma endregion

	#pragma region HEAD MODELING
	if(riftMan->isUsingRift() && riftMan->isRiftConnected())
	{
		// Rotate and position View Camera, using YawPitchRoll in BodyFrame coordinates
		Matrix4f rollPitchYaw = Matrix4f::RotationY(EyeYaw-yaw) * Matrix4f::RotationX(EyePitch) *
								Matrix4f::RotationZ(EyeRoll);
		Vector3f mUp      = rollPitchYaw.Transform(Vector3f(playerCamera->GetUp().x, playerCamera->GetUp().y, playerCamera->GetUp().z));
		Vector3f mForward = rollPitchYaw.Transform(Vector3f(playerCamera->GetLook().x, playerCamera->GetLook().y, playerCamera->GetLook().z));
		float headBaseToEyeHeight     = 0.15f;  // Vertical height of eye from base of head
		float headBaseToEyeProtrusion = 0.09f;  // Distance forward of eye from base of head

		// Minimal head modelling.
		Vector3f eyeCenterInHeadFrame(0.0f, headBaseToEyeHeight, -headBaseToEyeProtrusion);
		Vector3f shiftedEyePos = Vector3f(playerCamera->GetPosition().x, playerCamera->GetPosition().y, playerCamera->GetPosition().z) + rollPitchYaw.Transform(eyeCenterInHeadFrame);
		shiftedEyePos.y -= eyeCenterInHeadFrame.y; // Bring the head back down to original height

		playerCamera->SetPosition(shiftedEyePos.x, shiftedEyePos.y, shiftedEyePos.z);
		//Matrix4f View = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + forward, up);
		/*playerCamera->LookAt(XMFLOAT3(shiftedEyePos.x, shiftedEyePos.y, shiftedEyePos.z), 
			XMFLOAT3(shiftedEyePos.x + mForward.x, shiftedEyePos.y + mForward.y, shiftedEyePos.z + mForward.z),
			XMFLOAT3(mUp.x, mUp.y, mUp.z));*/
	}
#pragma endregion

	playerCamera->UpdateViewMatrix();
}

///////////////////////////////////////////////////////////////////////
// OnResize()
//
// Handle resizing of the world
// The way it resizes changes based on whether the rift is connected or not
///////////////////////////////////////////////////////////////////////////////
void Player::OnResize(float aspectRatio)
{
	if(riftMan->isUsingRift())
	{
		riftMan->calcStereo(); //Poll for rift
		playerCamera->SetLens(riftMan->getStereo().GetYFOVRadians(), riftMan->getStereo().GetAspect(), 0.01f, 100.0f);
	}
	else
	{
		playerCamera->SetLens(0.25f*MathHelper::Pi, aspectRatio, 0.01f, 100.0f);
		if(EyeRoll != 0)
		{
			playerCamera->setRotation(EyeYaw-yaw, EyePitch, 0);
		}
	}
	
	playerCamera->UpdateViewMatrix();
}

Camera* Player::GetCamera()
{
	return playerCamera;
}

//////////////////////////////////////////////
// resetStatuses()
//
// Set crest effects to their default state
//////////////////////////////////////////////
void Player::resetStatuses(bool leaveHeph) 
{	
	if(!medusaStatus)
		medusaPercent = 0;
	if(!winStatus)
		winPercent = 0;
	if(!leaveHeph)
		hephStatus = false;
	medusaStatus = false;
	mobilityStatus = false;
	leapStatus = false; 
	winStatus = false;
}

///////////////////////////////////////////////////////////////
// increaseMedusaPercent()
//
// increases the amount the medusa crest affects the player
// up to a certain amount
///////////////////////////////////////////////////////////////
void Player::increaseMedusaPercent()
{
	if(medusaStatus && medusaPercent < 1.0f)
	{
		medusaPercent = .9f;
	}
}

//////////////////////////////////////////////////////////////
// increaseWinPercent()
//
// increases the amount of blur the win crest is causing
//////////////////////////////////////////////////////////////
void Player::increaseWinPercent()
{
	if(winStatus && winPercent < 1.0f)
	{
		winPercent += 0.005;
	}
}

void Player::resetWinPercent()
{
	winPercent = 0.0f;
}

#pragma region ACCESSORS AND MUTATORS
void Player::setMobilityStatus(bool newStatus) { mobilityStatus = newStatus; }
void Player::setMedusaStatus(bool newStatus) { medusaStatus = newStatus; }
void Player::setLeapStatus(bool newStatus) { leapStatus = newStatus; }
void Player::setWinStatus(bool newStatus) { winStatus = newStatus; }
void Player::setHephStatus(bool newStatus) { hephStatus = newStatus; }

bool Player::getMobilityStatus() { return mobilityStatus; }
bool Player::getMedusaStatus() { return medusaStatus; }
bool Player::getLeapStatus() { return leapStatus; }
bool Player::getWinStatus() { return winStatus; }
bool Player::getHephStatus() { return hephStatus; }

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
#pragma endregion

Player::~Player(void)
{
	physicsMan->removeCharacterController(controller);
	delete listener;
	delete playerCamera;
	delete audioSource;
}
