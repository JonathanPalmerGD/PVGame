#pragma once

#include "PhysicsManager.h"
#include "Common\Camera.h"
#include "Common\xnacollision.h"
#include "Input.h"
#include "Audio\AudioListener.h"

#include <memory>

using namespace std;
using namespace XNA;

class Player
{
	public:
		Player(PhysicsManager* pm);
		virtual ~Player(void);
		void OnResize(float aspectRatio);
		void Update(float dt, Input* aInput);
		XMMATRIX ViewProj() const;
		Camera* GetCamera();

		XMFLOAT4 getPosition();
		btVector3 getCameraPosition();
	private:
		float playerSpeed;
		float camLookSpeed;
		const float PIXELS_PER_SEC;
		const float LOOK_SPEED;
		Camera* playerCamera;
		XMFLOAT4 position;
		XMFLOAT3 fwd;
		XMFLOAT3 right;
		XMFLOAT3 up;
		void HandleInput(Input* input);

		PhysicsManager* physicsMan;
		btKinematicCharacterController* controller;

		AudioListener* listener;
};

