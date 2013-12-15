#pragma once

//#include "PhysicsManager.h"
#include "Common\d3dApp.h"
#include "Common\d3dx11effect.h"
#include "Common\MathHelper.h"
#include "Player.h"
#include "Crest.h"
//#include <boost/thread.hpp>
//#include "Turret.h"
#include "tinyxml2.h"
#include "FileLoader.h"
#include "GameObject.h"
#include "Room.h"
#include "Audio/AL/al.h"
#include "Audio/AL/alc.h"
#include <vector>
#include "RiftManager.h"

using namespace tinyxml2;

class PVGame : public D3DApp
{
public:
		PVGame(HINSTANCE hInstance);
		virtual ~PVGame(void);

		bool Init(char* args);
		bool LoadContent();
		bool LoadXML();
		void OnResize();
		void UpdateScene(float dt);
		void ListenSelectorChange();
		void SetMenuColors();
		void DrawScene();
		void OnMouseMove(WPARAM btnState, int x, int y);
		void ToggleDevMode();
		bool getDevMode();

		void HandleOptions();
		void WriteOptions();
		void ReadOptions();
		void ApplyOptions();

		void SaveCurrentRoom();
		void ReadCurrentRoom();
		void ResetRoomToStart();

		void SpawnPlayer();
	private:
		void BuildFX();
		void BuildVertexLayout();
		void BuildRooms(Room* startRoom, const char* dontLoadRoom);
		void ClearRooms();
		void SortGameObjects();

		bool devMode;
		Player*	player;
		Room* currentRoom;
		int selector;

		float mTheta;
		float mPhi;
		POINT mLastMousePos;
		XMFLOAT3 playerPosition;

		float cWidth;
		float cHeight;
		UINT32 color1;
		UINT32 color2;
		UINT32 color3;
		UINT32 color4;
		float lgSize;
		float xmdSize;
		float medSize;
		float smlSize;
		float color[4];
		float colortwo[4];
		float colorthree[4];

		unsigned int gameState;
		PhysicsManager* physicsMan;
		vector<GameObject*> gameObjects;
		vector<GameObject*> proceduralGameObjects;
		vector<Room*> loadedRooms;

		ALCdevice* audioDevice;
		ALCcontext* audioContext;
		AudioSource* audioSource;
		AudioSource* audioWin;

		/////////////////////////////////////////////////////////////////////
		//Oculus Rift
		RiftManager* riftMan;
		/////////////////////////////////////////////////////////////////////

		long double VOLUME;
		bool FULLSCREEN;
		bool OCULUS;
		bool VSYNC;
		bool LOOKINVERSION;
		long double MOUSESENSITIVITY;
};