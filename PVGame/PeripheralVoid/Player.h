#pragma once

#include "PhysicsManager.h"
#include "RenderManager.h"
#include "Common\Camera.h"
#include "Common\xnacollision.h"
#include "Input.h"
#include "Audio\AudioListener.h"
#include <string>

#include <memory>

using namespace std;
using namespace XNA;

class Player
{
	public:
		Player(PhysicsManager* pm, RenderManager* rm);
		virtual ~Player(void);
		void OnResize(float aspectRatio);
		void Update(float dt, Input* aInput);
		XMMATRIX ViewProj() const;
		
		void resetStatuses();
		void increaseMedusaPercent();

		void setMobilityStatus(bool newStatus);
		void setMedusaStatus(bool newStatus);
		void setLeapStatus(bool newStatus);
		void setPosition(float setX, float setY, float setZ);

		Camera* GetCamera();
		btKinematicCharacterController* getController();
		bool getMobilityStatus();
		bool getMedusaStatus();
		bool getLeapStatus();
		XMFLOAT4 getPosition();
		btVector3 getCameraPosition();
	private:
		bool leapStatus;
		bool mobilityStatus;
		bool medusaStatus;
		float medusaPercent;
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
		RenderManager* renderMan;
		btKinematicCharacterController* controller;

		AudioListener* listener;
		AudioSource* audioSource;
		//Camera movement variables
		float mTheta;
		float mPhi;
		POINT mLastMousePos;
};

