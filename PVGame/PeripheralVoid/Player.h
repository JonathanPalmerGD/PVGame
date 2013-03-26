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
		void HandleInput(Input* input);
		void OnResize(float aspectRatio);
		XMMATRIX ViewProj() const;
		Camera* GetCamera();

	private:
		Camera* playerCamera;
		XMFLOAT4 position;
		XMFLOAT3 fwd;
		XMFLOAT3 right;
		XMFLOAT3 up;
		float PIXELS_PER_SEC; // Arbitrary speed value.
};

