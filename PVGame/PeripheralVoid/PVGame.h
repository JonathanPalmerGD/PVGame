#pragma once

//#include "PhysicsManager.h"
#include "Common\d3dApp.h"
#include "Common\d3dx11effect.h"
#include "Common\MathHelper.h"
#include "Player.h"
#include "tinyxml2.h"
#include "FileLoader.h"
#include "GameObject.h"
#include "Audio/AL/al.h"
#include "Audio/AL/alc.h"
#include <vector>

using namespace tinyxml2;

class PVGame : public D3DApp
{
public:
		PVGame(HINSTANCE hInstance);
		virtual ~PVGame(void);

		bool Init();
		bool LoadContent();
		bool LoadXML();
		void OnResize();
		void UpdateScene(float dt);
		void DrawScene();
		void OnMouseMove(WPARAM btnState, int x, int y);

	private:
		void BuildGeometryBuffers();
		void BuildFX();
		void BuildVertexLayout();
		void SortGameObjects();

		Player*	player;

		float mTheta;
		float mPhi;
		POINT mLastMousePos;
		XMFLOAT3 playerPosition;

		unsigned int gameState;
		PhysicsManager* physicsMan;
		vector<GameObject*> gameObjects;

		ALCdevice* audioDevice;
		ALCcontext* audioContext;
};