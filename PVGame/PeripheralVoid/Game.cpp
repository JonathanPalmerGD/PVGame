// Programming 2D Games
// Copyright (c) 2011 by: 
// Charles Kelly
// Chapter 5 game.cpp v1.0

#include "Game.h"

// The primary class should inherit from Game class

//=============================================================================
// Constructor
//=============================================================================
Game::Game()
{
    input = new Input();        // initialize keyboard input immediately
    // additional initialization is handled in later call to input->initialize()
    paused = false;             // game is not paused
    graphics = NULL;
	audio = NULL;
    initialized = false;
	currentElement = COUNT;

	// New random seed, huzzah!
	srand ( (unsigned int)(time(NULL)) );
}

//=============================================================================
// Destructor
//=============================================================================
Game::~Game()
{
    deleteAll();                // free all reserved memory
    ShowCursor(true);           // show cursor
}

//=============================================================================
// Window message handler
//=============================================================================
LRESULT Game::messageHandler( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if(initialized)     // do not process messages if not initialized
    {
        switch( msg )
        {
            case WM_DESTROY:
                PostQuitMessage(0);        //tell Windows to kill this program
                return 0;
            case WM_KEYDOWN: case WM_SYSKEYDOWN:    // key down
                input->keyDown(wParam);
                return 0;
            case WM_KEYUP: case WM_SYSKEYUP:        // key up
                input->keyUp(wParam);
                return 0;
            case WM_CHAR:                           // character entered
                input->keyIn(wParam);
                return 0;
            case WM_MOUSEMOVE:                      // mouse moved
                input->mouseIn(lParam);
                return 0;
            case WM_INPUT:                          // raw mouse data in
                input->mouseRawIn(lParam);
                return 0;
            case WM_LBUTTONDOWN:                    // left mouse button down
                input->setMouseLButton(true);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_LBUTTONUP:                      // left mouse button up
                input->setMouseLButton(false);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_MBUTTONDOWN:                    // middle mouse button down
                input->setMouseMButton(true);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_MBUTTONUP:                      // middle mouse button up
                input->setMouseMButton(false);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_RBUTTONDOWN:                    // right mouse button down
                input->setMouseRButton(true);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_RBUTTONUP:                      // right mouse button up
                input->setMouseRButton(false);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_XBUTTONDOWN: case WM_XBUTTONUP: // mouse X button down/up
                input->setMouseXButton(wParam);
                input->mouseIn(lParam);             // mouse position
                return 0;
            case WM_DEVICECHANGE:                   // check for controller insert
                input->checkControllers();
                return 0;
        }
    }
    return DefWindowProc( hwnd, msg, wParam, lParam );    // let Windows handle it
}

//=============================================================================
// Initializes the game
// throws GameError on error
//=============================================================================
void Game::initialize(HWND hw)
{
    hwnd = hw;                                  // save window handle

    // initialize graphics
    graphics = new Graphics();
    // throws GameError
    graphics->initialize(hwnd, GAME_WIDTH, GAME_HEIGHT, FULLSCREEN);

	audio = new Audio();
	// Initialize Audio
	if (*WAVE_BANK != '\0' && *SOUND_BANK != '\0')  // if sound files defined
	{
		if( FAILED( hr = audio->initialize() ) )
		{
			if( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
				throw(GameError(gameErrorNS::FATAL_ERROR, "Failed to initialize sound system because media file not found."));
			else
				throw(GameError(gameErrorNS::FATAL_ERROR, "Failed to initialize sound system."));
		}
	}

    // initialize input, do not capture mouse
    input->initialize(hwnd, false);             // throws GameError

    // attempt to set up high resolution timer
    if(QueryPerformanceFrequency(&timerFreq) == false)
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing high resolution timer"));

    QueryPerformanceCounter(&timeStart);        // get starting time

	instructions = 0;
    initialized = true;
}

//=============================================================================
// Render game items
//=============================================================================
void Game::renderGame()
{
    //start rendering
    if (SUCCEEDED(graphics->beginScene()))
    {
        render();           // call render() in derived object

		// added - KJB - display fps
		//strcpy_s(fpsStr,"FPS: ");
		//_itoa((int)fps,fpsStr+5,10); // convert FPS to string
		//graphics->DrawTextString(10,10,fpsStr);  // display the string

		int mouseX = input->getMouseX();
		int mouseY = input->getMouseY();

		// added to display mouse SCREEN position
		
		

		//added to display mouse MAP position
		/*strcpy_s(mouseMapStr,"Mouse MAP at (");
		itoa(mouseX-offsetX,mouseMapStr + strlen(mouseMapStr),10);
		strcat_s(mouseMapStr,",");
		itoa(mouseY-offsetY,mouseMapStr + strlen(mouseMapStr),10);
		strcat_s(mouseMapStr,")");
		graphics->DrawTextString(10,35,mouseMapStr);  // display the string 
		*/

		stringstream ss;
		
		ss << "Current State: " << currentState,mapStr + strlen(mapStr);
		//graphics->DrawTextString(10,440, ss.str().c_str());  // display the string
		ss.str(std::string()); // Clears the string stream.

		//pause message
		if(paused)
		{
			ss << "PAUSED - Hit [SPACE] to continue.";
			graphics->DrawTextString(200, 300, ss.str().c_str());
			ss.str(std::string()); // Clears the string stream.
		}

		if(currentState == START)
		{
			if(instructions == 0)
			{
				ss << "Press Up or Down for instructions" << "\n" << "Press Space to Start";
				graphics->DrawTextString(10, GAME_HEIGHT / 2 + 180, D3DCOLOR_ARGB(255, 139, 0, 0), ss.str().c_str());
				ss.str(std::string()); // Clears the string stream.

				ss << "By JP, DD, MSP, JL, RC";
				graphics->DrawTextString(410, GAME_HEIGHT - 30, D3DCOLOR_ARGB(255, 139, 0, 0), ss.str().c_str());
				ss.str(std::string()); // Clears the string stream.
			}
			else if (instructions == 1)
			{
				ss << "Instructions:";
				graphics->DrawTextString(50, 25, D3DCOLOR_ARGB(255, 155, 255, 255), ss.str().c_str());
				ss.str(std::string());
				/*ss << "Open and close the menu with M" << "\n" << "Move around with WASD" << "\n" << "Left click or press A to select a tile" << "\n" << "T to switch elemental tile." << "\n" << "1 and 2 to pull and push enemies" << "\n" << "Defeat the enemies to advance" << "\n" << "Escape will quit the game" << "\n\n" << "Fire increases Strength" << "\n" << "Earth increases Defense" << "\n" << "Wind increases Speed" << "\n" << "Water can be used to HEAL" ;*/
				
				ss << "Keyboard Controls for overworld:" << endl;
				ss << "WASD control movement." << endl;
				ss << "Arrow keys control the menu." << endl;
				ss << "Push and pull are overworld abilities." << endl;
				ss << "Activate push and pull with 1 and 2" << endl;
				ss << "Right click a tile to grow a terrain feature." << endl;
				ss << "Shift right click a tile to remove a terrain feature." << endl;
				ss << "Press M to toggle and disable the menu." << endl;
				ss << "Clicking a tile changes the element of that tile" << endl;
				ss << "Press T with the menu open to cycle through the tiles." << endl;
				ss << "Y button switches the element you can create." << endl << endl;
				ss << "Controls for Battle:" << endl;
				ss << "Select a movement option and then a combat option." << endl;
				ss << "Exit flees the battle." << endl;
				ss << "Water improves healing, earth improves defense,\nFire improves offense and wind improves speed." << endl;

				graphics->DrawTextString(50, 50, D3DCOLOR_ARGB(255, 255, 255, 255), ss.str().c_str());
				ss.str(std::string()); // Clears the string stream.
			}
			else if (instructions == 2)
			{
				//if (input->getControllerState(0)->connected)
				//{

				ss << "Instructions:";
				graphics->DrawTextString(50, 25, D3DCOLOR_ARGB(255, 155, 255, 255), ss.str().c_str());
				ss.str(std::string());
					ss << "Gamepad Controls for overworld:" << endl;
					ss << "Left thumbstick moves the player." << endl;
					ss << "Right thumbstick moves the camera / cursor." << endl;
					ss << "Left bumper pulls nearby enemies." << endl;
					ss << "Right bumper pushes nearby enemies." << endl;
					ss << "Right trigger + A increases tile feature." << endl;
					ss << "Right trigger + B decreases tile feature." << endl;
					ss << "B button held down and DPad button changes direction." << endl;
					ss << "Left trigger + A increases generates battlemap" << endl;
					ss << "A button changes element of selected tile ." << endl;
					ss << "Y button switches the element you can create." << endl << endl;
					ss << "Controls for Battle:" << endl;
					ss << "Left Thumbstick and DPad change menu options." << endl;
					ss << "Right Thumbstick moves tile cursor." << endl;
					ss << "A selects menu option." << endl;
					graphics->DrawTextString(50, 50, D3DCOLOR_ARGB(255, 255, 255, 255), ss.str().c_str());
					ss.str(std::string()); // Clears the string stream.
				//}
			}
		}
		if(currentState == OVERWORLD || currentState == BATTLE)
		{
			ss << "Mouse SCREEN at (" << mouseX << "," << mouseY << ")";;
			//graphics->DrawTextString(10, 10, ss.str().c_str());  // display the string
			ss.str(std::string()); // Clears the string stream.

			//added to display the position of the screen origin RELATIVE TO THE MAP
			//**NOTE - the offset is negated because Bierre wants the vector FROM the map origin TO the screen origin. If we want the opposite, we simply take out the negative signs.**
			ss << "Map Offset (" << -offsetX << "," << -offsetY << ")";
			//graphics->DrawTextString(10, 35, ss.str().c_str());  // display the string
			ss.str(std::string()); // Clears the string stream.

			ss << "Current Element: " << currentElement;
			//graphics->DrawTextString(10, 60, ss.str().c_str());
			ss.str(std::string()); // Clears the string stream.

			ss << "Focused Tile: (" << focusedTileX << "," << focusedTileY << ")";
			//graphics->DrawTextString(10, 110, ss.str().c_str());
			ss.str(std::string()); // Clears the string stream.
		}
		if(currentState == BATTLE)
		{
			if(currentTurn == 1)
			{
				ss << "Current Turn: Player";
			}
			else
			{
				ss << "Current Turn: " << currentTurn;
			}
			graphics->DrawTextString(10, 10, ss.str().c_str());
			ss.str(std::string()); // Clears the string stream.

			ss << "Backspace - To overworld";
			graphics->DrawTextString(10, 35, ss.str().c_str());
			ss.str(std::string()); // Clears the string stream.
			
			//ss << "S: " << player->

		}

        //stop rendering
        graphics->endScene();
    }
    handleLostGraphicsDevice();

    //display the back buffer on the screen
    graphics->showBackbuffer();
}

//=============================================================================
// Handle lost graphics device
//=============================================================================
void Game::handleLostGraphicsDevice()
{
    // test for and handle lost device
    hr = graphics->getDeviceState();
    if(FAILED(hr))                  // if graphics device is not in a valid state
    {
        // if the device is lost and not available for reset
        if(hr == D3DERR_DEVICELOST)
        {
            Sleep(100);             // yield cpu time (100 mili-seconds)
            return;
        } 
        // the device was lost but is now available for reset
        else if(hr == D3DERR_DEVICENOTRESET)
        {
            releaseAll();
            hr = graphics->reset(); // attempt to reset graphics device
            if(FAILED(hr))          // if reset failed
                return;
            resetAll();
        }
        else
            return;                 // other device error
    }
}

//=============================================================================
// Toggle window or fullscreen mode
//=============================================================================
void Game::setDisplayMode(graphicsNS::DISPLAY_MODE mode)
{
    releaseAll();                   // free all user created surfaces
    graphics->changeDisplayMode(mode);
    resetAll();                     // recreate surfaces
}

//=============================================================================
// Call repeatedly by the main message loop in WinMain
//=============================================================================
void Game::run(HWND hwnd)
{
    if(graphics == NULL)            // if graphics not initialized
        return;

    // calculate elapsed time of last frame, save in frameTime
    QueryPerformanceCounter(&timeEnd);
    frameTime = (float)(timeEnd.QuadPart - timeStart.QuadPart ) / (float)timerFreq.QuadPart;

    // Power saving code, requires winmm.lib
    // if not enough time has elapsed for desired frame rate
    if (frameTime < MIN_FRAME_TIME) 
    {
        sleepTime = (DWORD)((MIN_FRAME_TIME - frameTime)*1000);
        timeBeginPeriod(1);         // Request 1mS resolution for windows timer
        Sleep(sleepTime);           // release cpu for sleepTime
        timeEndPeriod(1);           // End 1mS timer resolution
        return;
    }

    if (frameTime > 0.0)
        fps = (fps*0.99f) + (0.01f/frameTime);  // average fps
    if (frameTime > MAX_FRAME_TIME) // if frame rate is very slow
        frameTime = MAX_FRAME_TIME; // limit maximum frameTime
    timeStart = timeEnd;

	//PAUSE and UNPAUSE the game
	if(input->isStartPressed() && (currentState == OVERWORLD || currentState == BATTLE))
	{
		if(paused)
		{
			paused = false;

			if(currentState == BATTLE)
			{
				audio->stopCue(NOSTRINGS);
				audio->playCue(BATTLELOOP);
			}
			else if (currentState == OVERWORLD)
			{
				audio->stopCue(BATTLELOOP);
				audio->playCue(NOSTRINGS);
			}
		}
		else
		{
			audio->stopCue(NOSTRINGS);
			audio->stopCue(BATTLELOOP);
			paused = true;
		}
	}
    // update(), ai(), and collisions() are pure virtual functions.
    // These functions must be provided in the class that inherits from Game.
    if (!paused)                    // if not paused
    {
        update();                   // update all game items
        ai();                       // artificial intelligence
        collisions();               // handle collisions
        input->vibrateControllers(frameTime); // handle controller vibration
    }
    renderGame();                   // draw all game items
    input->readControllers();       // read state of controllers

	audio->run();                       // perform periodic sound engine tasks

    // if Alt+Enter toggle fullscreen/window
    if (input->isKeyDown(ALT_KEY) && input->wasKeyPressed(ENTER_KEY))
        setDisplayMode(graphicsNS::TOGGLE); // toggle fullscreen/window

    // if Esc key, set window mode
    //if (input->isKeyDown(ESC_KEY))
    //    setDisplayMode(graphicsNS::WINDOW); // set window mode

    // Clear input
    // Call this after all key checks are done
    input->clear(inputNS::KEYS_PRESSED);
}

//=============================================================================
// The graphics device was lost.
// Release all reserved video memory so graphics device may be reset.
//=============================================================================
void Game::releaseAll()
{
	graphics->releaseAll();
}

//=============================================================================
// Recreate all surfaces and reset all entities.
//=============================================================================
void Game::resetAll()
{}

//=============================================================================
// Delete all reserved memory
//=============================================================================
void Game::deleteAll()
{
    releaseAll();               // call onLostDevice() for every graphics item
    SAFE_DELETE(graphics);
    SAFE_DELETE(input);
	SAFE_DELETE(audio);
    initialized = false;
}