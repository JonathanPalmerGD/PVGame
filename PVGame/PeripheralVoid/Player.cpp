#include "Player.h"

Player::Player(PhysicsManager* pm, RenderManager* rm) 
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
	playerCamera = new Camera(physicsMan);
	playerCamera->LookAt(aPos, target, aUp);
	playerCamera->UpdateViewMatrix();
	
	controller = physicsMan->createCharacterController( 1.0f, .8f, .1f);
	controller->setGravity(30.0f);
	controller->setJumpSpeed(15.0f);
	controller->setMaxJumpHeight(50.0f);
	controller->setMaxSlope(3.0f * 3.1415f);

	medusaStatus = false;
	leapStatus = false;
	mobilityStatus = false;
	medusaPercent = 0.0f;

	renderMan = rm;

	listener = new AudioListener();
	listener->mute();
	audioSource = new AudioSource();
	audioSource->initialize("Audio\\Jump.wav", AudioSource::WAV);
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
	// Basic mouse camera controls
	/*if (input->getMouseY() < renderMan->GetClientHeight() * .2)
	{
		playerCamera->Pitch(-camLookSpeed / 2);
	}

	if (input->getMouseY() >= renderMan->GetClientHeight() * .8)
	{
		playerCamera->Pitch(camLookSpeed / 2);
	}*/

	//if (input->getMouseX() < renderMan->GetClientWidth() * .2)
	//{
	//	float angle = -camLookSpeed / 2;
	//	playerCamera->RotateY(angle);
	//	XMMATRIX R = XMMatrixRotationY(angle);

	//	XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
	//	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	//	XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
	//	//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
	//}

	//if (input->getMouseX() >= renderMan->GetClientWidth() * .8)
	//{
	//	float angle = camLookSpeed / 2;
	//	playerCamera->RotateY(angle);
	//	XMMATRIX R = XMMatrixRotationY(angle);

	//	XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
	//	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	//	XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
	//	//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
	//}

		// Make each pixel correspond to a quarter of a degree.
	//float dx = XMConvertToRadians(0.25f*static_cast<float>(input->getMouseX() - mLastMousePos.x));
	//float dy = XMConvertToRadians(0.25f*static_cast<float>(input->getMouseY() - mLastMousePos.y));

	//// Update angles based on input to orbit camera around box.
	//mTheta += dx;
	//mPhi   += dy;
	//playerCamera->Pitch(dy/2); // Rotate the camera  up/down.

	//playerCamera->RotateY(dx / 2); // Rotate ABOUT the y-axis. So really turning left/right.
	//XMMATRIX R = XMMatrixRotationY(dx/2);

	//XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
	//XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	//XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
	//mPhi = MathHelper::Clamp(dy, -0.10f * MathHelper::Pi, 0.05f * MathHelper::Pi);

	playerCamera->UpdateViewMatrix();

	mLastMousePos.x = input->getMouseX();
	mLastMousePos.y = input->getMouseY();

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
	if(input->isJumpKeyPressed() && !medusaStatus)
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

	if(input->wasKeyPressed('M'))
	{
		if(listener->isMuted())
			listener->unmute();
		else
			listener->mute();
	}
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
}

void Player::OnResize(float aspectRatio)
{
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
