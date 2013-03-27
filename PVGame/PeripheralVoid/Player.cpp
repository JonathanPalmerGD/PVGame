#include "Player.h"

Player::Player(void)
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

	playerCamera = new Camera();
	playerCamera->LookAt(aPos, target, aUp);
	playerCamera->UpdateViewMatrix();
	PIXELS_PER_SEC = 0.001f;
}

void Player::HandleInput(Input* input)
{
	XMVECTOR tempPosition = XMLoadFloat4(&position);

	// Checking for position movement input, will want to move to separate method later.
	if (input->isPlayerUpKeyDown())
		tempPosition = XMVectorAdd(tempPosition, XMLoadFloat3(&fwd) * PIXELS_PER_SEC);
	if (input->isPlayerDownKeyDown())
		tempPosition = XMVectorAdd(tempPosition, -XMLoadFloat3(&fwd) * PIXELS_PER_SEC);
	if (input->isPlayerLeftKeyDown())
		tempPosition = XMVectorAdd(tempPosition, -XMLoadFloat3(&right) * PIXELS_PER_SEC);
	if (input->isPlayerRightKeyDown())
		tempPosition = XMVectorAdd(tempPosition, XMLoadFloat3(&right) * PIXELS_PER_SEC);

	// Now check for camera input.
	if (input->isCameraUpKeyDown())
	{
		playerCamera->Pitch(-0.0001f);
	}
	if (input->isCameraDownKeyDown())
	{
		playerCamera->Pitch(0.0001f);
	}

	if (input->isCameraLeftKeyDown())
	{
		float angle = -0.0005f;
		playerCamera->RotateY(angle);
		XMMATRIX R = XMMatrixRotationY(angle);

		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
		//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
	}
	if (input->isCameraRightKeyDown())
	{
		float angle = 0.0005f;
		playerCamera->RotateY(angle);
		XMMATRIX R = XMMatrixRotationY(angle);

		XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
		XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
		XMStoreFloat3(&fwd, XMVector3TransformNormal(XMLoadFloat3(&fwd), R));
		//TransformOrientedBox(boundingBox.get(), boundingBox.get(), 1.0f, XMQuaternionRotationMatrix(R), XMVECTOR());
	}

	XMFLOAT4 unit(0.0f, 0.0f, 0.0f, 1.0f);
	
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, tempPosition);
	playerCamera->SetPosition(pos);
	XMStoreFloat4(&position, tempPosition);

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
	playerCamera->SetLens(0.25f*MathHelper::Pi, aspectRatio, 1.0f, 1000.0f);
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

Player::~Player(void)
{
	delete playerCamera;
}
