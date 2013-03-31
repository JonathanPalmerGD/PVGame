#pragma once

#include "PhysicsManager.h"
#include "Common\Camera.h"
#include "Common\xnacollision.h"
#include "Input.h"

#include <memory>

using namespace std;
using namespace XNA;

class Player
{
	public:
		Player(void);
		virtual ~Player(void);
		void OnResize(float aspectRatio);
		void Update(float dt, Input* aInput);
		XMMATRIX ViewProj() const;
		Camera* GetCamera();

		XMFLOAT4 getPosition();

	private:
		float playerSpeed;
		const float PIXELS_PER_SEC;
		Camera* playerCamera;
		XMFLOAT4 position;
		XMFLOAT3 fwd;
		XMFLOAT3 right;
		XMFLOAT3 up;
		void HandleInput(Input* input);
};

