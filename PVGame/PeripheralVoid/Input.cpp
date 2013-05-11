// Programming 2D Games
// Copyright (c) 2011 by: 
// Charles Kelly
// input.cpp v1.0

#include "Input.h"

//=============================================================================
// default constructor
//=============================================================================
Input::Input()
{
    // clear key down array
    for (size_t i = 0; i < inputNS::KEYS_ARRAY_LEN; i++)
        keysDown[i] = false;
    // clear key pressed array
    for (size_t i = 0; i < inputNS::KEYS_ARRAY_LEN; i++)
        keysPressed[i] = false;

	// Clear gamepad arrays.
	for (size_t i = 0; i < inputNS::GAMEPAD_ARRAY_LEN; i++)
        gamepadButtonsDown[i] = false;
    for (size_t i = 0; i < inputNS::GAMEPAD_ARRAY_LEN; i++)
        gamepadButtonsPressed[i] = false;

	// Clear mouse button arrays.
	for (size_t i = 0; i < inputNS::MOUSE_BUTTON_ARRAY_LEN; i++)
        mouseButtonsDown[i] = false;
    for (size_t i = 0; i < inputNS::MOUSE_BUTTON_ARRAY_LEN; i++)
        mouseButtonsPressed[i] = false;

    newLine = true;                     // start new line
    textIn = "";                        // clear textIn
    charIn = 0;                         // clear charIn

    // mouse data
    mouseX = 0;                         // screen X
    mouseY = 0;                         // screen Y
    mouseRawX = 0;                      // high-definition X
    mouseRawY = 0;                      // high-definition Y
    mouseLButton = false;               // true if left mouse button is down
    mouseMButton = false;               // true if middle mouse button is down
    mouseRButton = false;               // true if right mouse button is down
    mouseX1Button = false;              // true if X1 mouse button is down
    mouseX2Button = false;              // true if X2 mouse button is down

    for(int i=0; i<MAX_CONTROLLERS; i++)
    {
        controllers[i].vibrateTimeLeft = 0;
        controllers[i].vibrateTimeRight = 0;
    }

	screenWidth = 1280;
	screenHeight = 800;
}

//=============================================================================
// destructor
//=============================================================================
Input::~Input()
{
    if(mouseCaptured)
        ReleaseCapture();               // release mouse
}

//=============================================================================
// Initialize mouse and controller input
// Set capture=true to capture mouse
// Throws GameError
//=============================================================================
void Input::initialize(HWND hwnd, bool capture)
{
    try{
        mouseCaptured = capture;

        // register high-definition mouse
        Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
        Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
        Rid[0].dwFlags = RIDEV_INPUTSINK;   
        Rid[0].hwndTarget = hwnd;
        RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

        if(mouseCaptured)
		{
            SetCapture(hwnd);           // capture mouse
		}
        // Clear controllers state
        ZeroMemory( controllers, sizeof(ControllerState) * MAX_CONTROLLERS );

        checkControllers();             // check for connected controllers

		window = hwnd;
    }
    catch(...)
    {
       //"Error initializing input system"
    }
}

//=============================================================================
// Set true in the keysDown and keysPessed array for this key
// Pre: wParam contains the virtual key code (0--255)
//=============================================================================
void Input::keyDown(WPARAM wParam)
{
    // make sure key code is within buffer range
    if (wParam < inputNS::KEYS_ARRAY_LEN)
    {
		if(keysDown[wParam] == false)
			keysPressed[wParam] = true; // update keysPressed array
		else
			keysPressed[wParam] = false;

		keysDown[wParam] = true;    // update keysDown array
    }
}

//=============================================================================
// Set false in the keysDown array for this key
// Pre: wParam contains the virtual key code (0--255)
//=============================================================================
void Input::keyUp(WPARAM wParam)
{
    // make sure key code is within buffer range
    if (wParam < inputNS::KEYS_ARRAY_LEN)
	{
        // update state table
        keysDown[wParam] = false;
		keysPressed[wParam] = false;
	}
}

//=============================================================================
// Save the char just entered in textIn string
// Pre: wParam contains the char
//=============================================================================
void Input::keyIn(WPARAM wParam)
{
    if (newLine)                            // if start of new line
    {
        textIn.clear();
        newLine = false;
    }

    if (wParam == '\b')                     // if backspace
    {
        if(textIn.length() > 0)             // if characters exist
            textIn.erase(textIn.size()-1);  // erase last character entered
    }
    else
    {
        textIn += wParam;                   // add character to textIn
        charIn = wParam;                    // save last char entered
    }

    if ((char)wParam == '\r')               // if return    
        newLine = true;                     // start new line
}

//=============================================================================
// Returns true if the specified VIRTUAL KEY is down, otherwise false.
//=============================================================================
bool Input::isKeyDown(UCHAR vkey) const
{
    if (vkey < inputNS::KEYS_ARRAY_LEN)
        return keysDown[vkey];
    else
        return false;
}

//=============================================================================
// Return true if the specified VIRTUAL KEY has been pressed in the most recent
// frame. Key presses are erased at the end of each frame.
//=============================================================================
bool Input::wasKeyPressed(UCHAR vkey) const
{
    if (vkey < inputNS::KEYS_ARRAY_LEN)
        return keysPressed[vkey];
    else
        return false;
}

//=============================================================================
// Return true if any key was pressed in the most recent frame.
// Key presses are erased at the end of each frame.
//=============================================================================
bool Input::anyKeyPressed() const
{
    for (size_t i = 0; i < inputNS::KEYS_ARRAY_LEN; i++)
	{
        if(keysPressed[i] == true)
            return true;
	}
    return false;
}

//=============================================================================
// Clear the specified key press
//=============================================================================
void Input::clearKeyPress(UCHAR vkey)
{
    if (vkey < inputNS::KEYS_ARRAY_LEN)
        keysPressed[vkey] = false;
}

//=============================================================================
// Clear specified input buffers
// See input.h for what values
//=============================================================================
void Input::clear(UCHAR what)
{
    if(what & inputNS::KEYS_DOWN)       // if clear keys down
    {
        for (size_t i = 0; i < inputNS::KEYS_ARRAY_LEN; i++)
            keysDown[i] = false;
    }
    if(what & inputNS::KEYS_PRESSED)    // if clear keys pressed
    {
        for (size_t i = 0; i < inputNS::KEYS_ARRAY_LEN; i++)
            keysPressed[i] = false;
    }
    if(what & inputNS::MOUSE)           // if clear mouse
    {
        mouseX = 0;
        mouseY = 0;
        mouseRawX = 0;
        mouseRawY = 0;
    }
    if(what & inputNS::TEXT_IN)
        clearTextIn();
}

//=============================================================================
// Reads mouse screen position into mouseX, mouseY
//=============================================================================
void Input::mouseIn(LPARAM lParam)
{
    mouseX = GET_X_LPARAM(lParam); 
    mouseY = GET_Y_LPARAM(lParam);
}

void Input::centerMouse()
{
	POINT p;
	p.x = screenWidth/2;
	p.y = screenHeight/2;
	mouseX = p.x;
	mouseY = p.y;
	ClientToScreen(window, &p);
	SetCursorPos(p.x,p.y);
}

//=============================================================================
// Reads raw mouse data into mouseRawX, mouseRawY
// This routine is compatible with a high-definition mouse
//=============================================================================
void Input::mouseRawIn(LPARAM lParam)
{
    UINT dwSize = 40;
    static BYTE lpb[40];
    
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
                    lpb, &dwSize, sizeof(RAWINPUTHEADER));
    
    RAWINPUT* raw = (RAWINPUT*)lpb;
    
    if (raw->header.dwType == RIM_TYPEMOUSE) 
    {
        mouseRawX = raw->data.mouse.lLastX;
        mouseRawY = raw->data.mouse.lLastY;
    } 
}

//=============================================================================
// Check for connected controllers
//=============================================================================
void Input::checkControllers()
{
    DWORD result;
    for( DWORD i = 0; i <MAX_CONTROLLERS; i++)
    {
        result = XInputGetState(i, &controllers[i].state);
        if(result == ERROR_SUCCESS)
            controllers[i].connected = true;
        else
            controllers[i].connected = false;
    }
}

//=============================================================================
// Read state of connected controllers and mouse.
//=============================================================================
void Input::readControllers()
{
    DWORD result;
    for( DWORD i = 0; i <MAX_CONTROLLERS; i++)
    {
        if(controllers[i].connected)
        {
            result = XInputGetState(i, &controllers[i].state);
            if(result == ERROR_DEVICE_NOT_CONNECTED)    // if controller disconnected
			{
                controllers[i].connected = false;
				return;
			}
			else // Setup ALL the controls. I don't think there's a way to loop through each button and get the state - if there is, do that, please.
			{
				if (getGamepadDPadUp(i)) gamepadButtonDown(DPadUp);
				else gamepadButtonUp(DPadUp);

				if (getGamepadDPadDown(i)) gamepadButtonDown(DPadDown);
				else gamepadButtonUp(DPadDown);

				if (getGamepadDPadLeft(i)) gamepadButtonDown(DPadLeft);
				else gamepadButtonUp(DPadLeft);

				if (getGamepadDPadRight(i)) gamepadButtonDown(DPadRight);
				else gamepadButtonUp(DPadRight);

				if (getGamepadStart(i)) gamepadButtonDown(StartButton);
				else gamepadButtonUp(StartButton);

				if (getGamepadBack(i)) gamepadButtonDown(BackButton);
				else gamepadButtonUp(BackButton);

				if (getGamepadLeftThumb(i)) gamepadButtonDown(LeftThumbButton);
				else gamepadButtonUp(LeftThumbButton);

				if (getGamepadRightThumb(i)) gamepadButtonDown(RightThumbButton);
				else gamepadButtonUp(RightThumbButton);

				if (getGamepadLeftShoulder(i)) gamepadButtonDown(LeftShoulderButton);
				else gamepadButtonUp(LeftShoulderButton);

				if (getGamepadRightShoulder(i)) gamepadButtonDown(RightShoulderButton);
				else gamepadButtonUp(RightShoulderButton);

				if (getGamepadA(i)) gamepadButtonDown(ButtonA);
				else gamepadButtonUp(ButtonA);

				if (getGamepadB(i)) gamepadButtonDown(ButtonB);
				else gamepadButtonUp(ButtonB);

				if (getGamepadX(i)) gamepadButtonDown(ButtonX);
				else gamepadButtonUp(ButtonX);

				if (getGamepadY(i)) gamepadButtonDown(ButtonY);
				else gamepadButtonUp(ButtonY);
			}
        }
    }

	if (getMouseLButton()) mouseButtonDown(MouseLeft);
	else mouseButtonUp(MouseLeft);

	if (getMouseMButton()) mouseButtonDown(MouseMiddle);
	else mouseButtonUp(MouseMiddle);

	if (getMouseRButton()) mouseButtonDown(MouseRight);
	else mouseButtonUp(MouseRight);
}

//=============================================================================
// Vibrate connected controllers
//=============================================================================
void Input::vibrateControllers(float frameTime)
{
    for(int i=0; i < MAX_CONTROLLERS; i++)
    {
        if(controllers[i].connected)
        {
            controllers[i].vibrateTimeLeft -= frameTime;
            if(controllers[i].vibrateTimeLeft < 0)
            {
                controllers[i].vibrateTimeLeft = 0;
                controllers[i].vibration.wLeftMotorSpeed = 0;
            }
            controllers[i].vibrateTimeRight -= frameTime;
            if(controllers[i].vibrateTimeRight < 0)
            {
                controllers[i].vibrateTimeRight = 0;
                controllers[i].vibration.wRightMotorSpeed = 0;
            }
            XInputSetState(i, &controllers[i].vibration);
        }
    }
}

/* ***************************************************************************************************
	/*
	/*
	/* Custom button code! Supports mouse or Controllers!! Assumes we only have 1 controller, in slot '0'. 
	/* Also includes custom mouse button code.
	/*
	/*
	/* ****************************************************************************************************/


//If the index game pad is connected.
bool Input::gamepadConnected(int i)
{
	// make sure key code is within buffer range
	if (controllers[i].connected)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Set that the corresponding gamepad button is down.
void Input::gamepadButtonDown(int buttonIndex)
{
    // make sure key code is within buffer range
    if (buttonIndex < inputNS::GAMEPAD_ARRAY_LEN)
    {
		if(gamepadButtonsDown[buttonIndex] == false)
			gamepadButtonsPressed[buttonIndex] = true; 
		else
			gamepadButtonsPressed[buttonIndex] = false;

		gamepadButtonsDown[buttonIndex] = true;    
    }
}

// Set that the corresponding gamepad button is up.
void Input::gamepadButtonUp(int buttonIndex)
{
    // make sure key code is within buffer range
    if (buttonIndex < inputNS::GAMEPAD_ARRAY_LEN)
	{
        gamepadButtonsDown[buttonIndex] = false;
		gamepadButtonsPressed[buttonIndex] = false;
	}
}

// Set that the corresponding mouse button is down.
void Input::mouseButtonDown(int buttonIndex)
{
    // make sure key code is within buffer range
    if (buttonIndex < inputNS::MOUSE_BUTTON_ARRAY_LEN)
    {
		if(mouseButtonsDown[buttonIndex] == false)
			mouseButtonsPressed[buttonIndex] = true; 
		else
			mouseButtonsPressed[buttonIndex] = false;

		mouseButtonsDown[buttonIndex] = true;    
    }
}

// Set that the corresponding mouse button is up.
void Input::mouseButtonUp(int buttonIndex)
{
    // make sure key code is within buffer range
    if (buttonIndex < inputNS::MOUSE_BUTTON_ARRAY_LEN)
	{
        mouseButtonsDown[buttonIndex] = false;
		mouseButtonsPressed[buttonIndex] = false;
	}
}

// Returns true if the corresponding gamepad button is being held down.
bool Input::isGamepadButtonDown(int buttonIndex) const
{
	if (controllers[0].connected == false)
		return false;

	if (buttonIndex < inputNS::GAMEPAD_ARRAY_LEN)
        return gamepadButtonsDown[buttonIndex];
    else
        return false;
}

// Returns true if the corresponding gamepad button was just pressed.
bool Input::wasGamepadButtonPressed(int buttonIndex) const
{
	if (controllers[0].connected == false)
		return false;

    if (buttonIndex < inputNS::GAMEPAD_ARRAY_LEN)
        return gamepadButtonsPressed[buttonIndex];
    else
        return false;
}

// Returns true if the corresponding mouse button is being held down.
bool Input::isMouseButtonDown(int buttonIndex) const
{
	if (buttonIndex < inputNS::MOUSE_BUTTON_ARRAY_LEN)
        return mouseButtonsDown[buttonIndex];
    else
        return false;
}

// Returns true if the corresponding mouse button was just pressed.
bool Input::wasMouseButtonPressed(int buttonIndex) const
{
	if (buttonIndex < inputNS::MOUSE_BUTTON_ARRAY_LEN)
        return mouseButtonsPressed[buttonIndex];
    else
        return false;
}

/*
 *
 *	Abstract input methods! Return true if corresponding mouse / gamepad buttons are true.
 *
 */

// True if the player selected something with left mouse button or A button on gamepad.
bool Input::isSelectionPressed() const
{
	return (wasMouseButtonPressed(MouseLeft) || wasGamepadButtonPressed(ButtonA));
}

// True if the player hits backspace.
bool Input::isBackPressed() const
{
	return (wasKeyPressed(VK_BACK));
}

// True if the player hits escape or select button on gamepad.
bool Input::isQuitPressed() const
{
	return (wasKeyPressed(VK_ESCAPE) || wasGamepadButtonPressed(BackButton));
}

// True if the player hits space or start button on gamepad.
bool Input::isStartPressed() const
{
	return (wasKeyPressed(VK_SPACE) || wasGamepadButtonPressed(StartButton));
}

// PLAYER MOVEMENT.
// True if the player hits W or up on the left joystick on gamepad.
bool Input::isPlayerUpKeyDown() const
{
	return (isKeyDown('W') );
}

// True if the player hits S or down on the left joystick on gamepad.
bool Input::isPlayerDownKeyDown() const
{
	return (isKeyDown('S') );
}

// True if the player hits A or left on the left joystick on gamepad.
bool Input::isPlayerLeftKeyDown() const
{
	return (isKeyDown('A'));
}

// True if the player hits D or right on the left joystick on gamepad.
bool Input::isPlayerRightKeyDown() const
{
	return (isKeyDown('D'));
}


// CAMERA MOVEMENT.
// True if the player hits right arrow or right on the right joystick on gamepad.
bool Input::isCameraRightKeyDown() const
{
	return (isKeyDown(VK_RIGHT) );  
}

// True if the player hits left arrow or left on the right joystick on gamepad.
bool Input::isCameraLeftKeyDown() const
{
	return (isKeyDown(VK_LEFT)  );
}

// True if the player hits up arrow or up on the right joystick on gamepad.
bool Input::isCameraUpKeyDown() const
{
	return (isKeyDown(VK_UP) );
}

// True if the player hits down arrow or down on the right joystick on gamepad.
bool Input::isCameraDownKeyDown() const
{
	return (isKeyDown(VK_DOWN));
}


// MENU MOVEMENT
// True if the player hits up arrow key or up on the DPad or up on Left thumbstick.
bool Input::wasMenuUpPressed() const
{
	return (wasKeyPressed(VK_UP) || wasGamepadButtonPressed(DPadUp) || (getGamepadThumbLY(0) > (int)(GAMEPAD_THUMBSTICK_DEADZONE)));
}

// True if the player hits down arrow key or down on the DPad or down on Left thumbstick.
bool Input::wasMenuDownPressed() const
{
	return (wasKeyPressed(VK_DOWN) || wasGamepadButtonPressed(DPadDown) || (getGamepadThumbLY(0) < -(int)(GAMEPAD_THUMBSTICK_DEADZONE)));
}

// True if return or A button on gamepad is pressed.
bool Input::wasMenuSelectPressed() const
{
	return (wasKeyPressed(VK_RETURN) || wasGamepadButtonPressed(ButtonA));
}

bool Input::wasJumpKeyPressed() const
{
	return (wasKeyPressed(VK_SPACE) || wasGamepadButtonPressed(ButtonA));
}

// Returns true if 'o' or a TBD gamepad button is pressed.
bool Input::isOculusButtonPressed() const
{
	return (wasKeyPressed('O'));
}