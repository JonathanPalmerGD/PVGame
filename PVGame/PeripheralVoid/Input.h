// Programming 2D Games
// Copyright (c) 2011 by: 
// Charles Kelly
// input.h v1.0

#ifndef _INPUT_H                // Prevent multiple definitions if this 
#define _INPUT_H                // file is included in more than one place
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <WindowsX.h>
#include <string>
#include <XInput.h>


// for high-definition mouse
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC      ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE     ((USHORT) 0x02)
#endif
//--------------------------

namespace inputNS
{
    const int KEYS_ARRAY_LEN = 256;     // size of key arrays
    const int GAMEPAD_ARRAY_LEN = 14;	// How many Gamepad buttons there are.
	const int MOUSE_BUTTON_ARRAY_LEN = 3; // Only concerned with 3 main mouse buttons.

    // what values for clear(), bit flag
    const UCHAR KEYS_DOWN = 1;
    const UCHAR KEYS_PRESSED = 2;
    const UCHAR MOUSE = 4;
    const UCHAR TEXT_IN = 8;
    const UCHAR KEYS_MOUSE_TEXT = KEYS_DOWN + KEYS_PRESSED + MOUSE + TEXT_IN;
}

const float GAMEPAD_THUMBSTICK_DEADZONE = (float)(0.19f * 0X7FFF); // default to 60% of range as deadzone
const DWORD GAMEPAD_TRIGGER_DEADZONE = 30;                         // trigger range 0-255
const DWORD MAX_CONTROLLERS = 4;                                   // Maximum number of controllers supported by XInput

// Bit corresponding to gamepad button in state.Gamepad.wButtons
const DWORD GAMEPAD_DPAD_UP        = 0x0001;
const DWORD GAMEPAD_DPAD_DOWN      = 0x0002;
const DWORD GAMEPAD_DPAD_LEFT      = 0x0004;
const DWORD GAMEPAD_DPAD_RIGHT     = 0x0008;
const DWORD GAMEPAD_START_BUTTON   = 0x0010;
const DWORD GAMEPAD_BACK_BUTTON    = 0x0020;
const DWORD GAMEPAD_LEFT_THUMB     = 0x0040;
const DWORD GAMEPAD_RIGHT_THUMB    = 0x0080;
const DWORD GAMEPAD_LEFT_SHOULDER  = 0x0100;
const DWORD GAMEPAD_RIGHT_SHOULDER = 0x0200;
const DWORD GAMEPAD_A              = 0x1000;
const DWORD GAMEPAD_B              = 0x2000;
const DWORD GAMEPAD_X              = 0x4000;
const DWORD GAMEPAD_Y              = 0x8000;

enum GamepadButtons { DPadUp = 0, DPadDown, DPadLeft, DPadRight, StartButton, BackButton, LeftThumbButton, RightThumbButton, LeftShoulderButton, RightShoulderButton, ButtonA, ButtonB, ButtonX, ButtonY };
enum MouseButtons { MouseLeft = 0, MouseMiddle, MouseRight }; // Realistically we'll only need these 3 mouse buttons at most.

struct ControllerState
{
    XINPUT_STATE        state;
    XINPUT_VIBRATION    vibration;
    float               vibrateTimeLeft;    // mSec
    float               vibrateTimeRight;   // mSec
    bool                connected;
};

class Input
{
private:
    bool keysDown[inputNS::KEYS_ARRAY_LEN];     // true if specified key is down
    bool keysPressed[inputNS::KEYS_ARRAY_LEN];  // true if specified key was pressed
	bool gamepadButtonsDown[inputNS::GAMEPAD_ARRAY_LEN];     // true if specified gamepad button is down
	bool gamepadButtonsPressed[inputNS::GAMEPAD_ARRAY_LEN];  // true if specified gamepad button was pressed
	bool mouseButtonsDown[inputNS::MOUSE_BUTTON_ARRAY_LEN];
	bool mouseButtonsPressed[inputNS::MOUSE_BUTTON_ARRAY_LEN];

    std::string textIn;                         // user entered text
    char charIn;                                // last character entered
    bool newLine;                               // true on start of new line
    int  mouseX, mouseY;                        // mouse screen coordinates
    int  mouseRawX, mouseRawY;                  // high-definition mouse data
    RAWINPUTDEVICE Rid[1];                      // for high-definition mouse
    bool mouseCaptured;                         // true if mouse captured
    bool mouseLButton;                          // true if left mouse button down
    bool mouseMButton;                          // true if middle mouse button down
    bool mouseRButton;                          // true if right mouse button down
    bool mouseX1Button;                         // true if X1 mouse button down
    bool mouseX2Button;                         // true if X2 mouse button down
    ControllerState controllers[MAX_CONTROLLERS];    // state of controllers
	HWND window;
	int screenWidth;
	int screenHeight;
public:
    // Constructor
    Input();

    // Destructor
    virtual ~Input();

    // Initialize mouse and controller input.
    // Throws GameError
    // Pre: hwnd = window handle
    //      capture = true to capture mouse.
    void initialize(HWND hwnd, bool capture);

    // Save key down state
    void keyDown(WPARAM);

    // Save key up state
    void keyUp(WPARAM);

    // Save the char just entered in textIn string
    void keyIn(WPARAM);

    // Returns true if the specified VIRTUAL KEY is down, otherwise false.
    bool isKeyDown(UCHAR vkey) const;

    // Return true if the specified VIRTUAL KEY has been pressed in the most recent frame.
    // Key presses are erased at the end of each frame.
    bool wasKeyPressed(UCHAR vkey) const;

    // Return true if any key was pressed in the most recent frame.
    // Key presses are erased at the end of each frame.
    bool anyKeyPressed() const;

    // Clear the specified key press
    void clearKeyPress(UCHAR vkey);

    // Clear specified input buffers where what is any combination of
    // KEYS_DOWN, KEYS_PRESSED, MOUSE, TEXT_IN or KEYS_MOUSE_TEXT.
    // Use OR '|' operator to combine parmeters.
    void clear(UCHAR what);

    // Clears key, mouse and text input data
    void clearAll() {clear(inputNS::KEYS_MOUSE_TEXT);}

    // Clear text input buffer
    void clearTextIn() {textIn.clear();}

    // Return text input as a string
    std::string getTextIn() {return textIn;}

    // Return last character entered
    char getCharIn()        {return charIn;}

    // Reads mouse screen position into mouseX, mouseY
    void mouseIn(LPARAM);

	void centerMouse();

	void setScreen(int width, int height);

    // Reads raw mouse data into mouseRawX, mouseRawY
    // This routine is compatible with a high-definition mouse
    void mouseRawIn(LPARAM);

    // Save state of mouse button
    void setMouseLButton(bool b) { mouseLButton = b; }

    // Save state of mouse button
    void setMouseMButton(bool b) { mouseMButton = b; }

    // Save state of mouse button
    void setMouseRButton(bool b) { mouseRButton = b; }

    // Save state of mouse button
    void setMouseXButton(WPARAM wParam) {mouseX1Button = (wParam & MK_XBUTTON1) ? true:false;
                                         mouseX2Button = (wParam & MK_XBUTTON2) ? true:false;}
    // Return mouse X position
    int  getMouseX()        const { return mouseX; }

    // Return mouse Y position
    int  getMouseY()        const { return mouseY; }

    // Return raw mouse X movement. Left is <0, Right is >0
    // Compatible with high-definition mouse.
    int  getMouseRawX()     const { return mouseRawX; }

    // Return raw mouse Y movement. Up is <0, Down is >0
    // Compatible with high-definition mouse.
    int  getMouseRawY()     const { return mouseRawY; }

    // Return state of left mouse button.
    bool getMouseLButton()  const { return mouseLButton; }

    // Return state of middle mouse button.
    bool getMouseMButton()  const { return mouseMButton; }

    // Return state of right mouse button.
    bool getMouseRButton()  const { return mouseRButton; }

    // Return state of X1 mouse button.
    bool getMouseX1Button() const { return mouseX1Button; }

    // Return state of X2 mouse button.
    bool getMouseX2Button() const { return mouseX2Button; }

    // Update connection status of game controllers.
    void checkControllers();

    // Save input from connected game controllers.
    void readControllers();

    // Return state of specified game controller.
    const ControllerState* getControllerState(UINT n)
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return &controllers[n];
    }

    // Return state of controller n buttons.
    const WORD getGamepadButtons(UINT n) 
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return controllers[n].state.Gamepad.wButtons;
    }

    // Return state of controller n D-pad Up
    bool getGamepadDPadUp(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return ((controllers[n].state.Gamepad.wButtons&GAMEPAD_DPAD_UP) != 0);
    }

    // Return state of controller n D-pad Down.
    bool getGamepadDPadDown(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return ((controllers[n].state.Gamepad.wButtons&GAMEPAD_DPAD_DOWN) != 0);
    }

    // Return state of controller n D-pad Left.
    bool getGamepadDPadLeft(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return ((controllers[n].state.Gamepad.wButtons&GAMEPAD_DPAD_LEFT) != 0);
    }

    // Return state of controller n D-pad Right.
    bool getGamepadDPadRight(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_DPAD_RIGHT) != 0);
    }

    // Return state of controller n Start button.
    bool getGamepadStart(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_START_BUTTON) != 0);
    }

    // Return state of controller n Back button.
    bool getGamepadBack(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_BACK_BUTTON) != 0);
    }

    // Return state of controller n Left Thumb button.
    bool getGamepadLeftThumb(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_LEFT_THUMB) != 0);
    }

    // Return state of controller n Right Thumb button.
    bool getGamepadRightThumb(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_RIGHT_THUMB) != 0);
    }

    // Return state of controller n Left Shoulder button.
    bool getGamepadLeftShoulder(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_LEFT_SHOULDER) != 0);
    }

    // Return state of controller n Right Shoulder button.
    bool getGamepadRightShoulder(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_RIGHT_SHOULDER) != 0);
    }

    // Return state of controller n A button.
    bool getGamepadA(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_A) != 0);
    }

    // Return state of controller n B button.
    bool getGamepadB(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_B) != 0);
    }

    // Return state of controller n X button.
    bool getGamepadX(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_X) != 0);
    }

    // Return state of controller n Y button.
    bool getGamepadY(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return bool((controllers[n].state.Gamepad.wButtons&GAMEPAD_Y) != 0);
    }

    // Return value of controller n Left Trigger.
    BYTE getGamepadLeftTrigger(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.bLeftTrigger);
    }

    // Return value of controller n Right Trigger.
    BYTE getGamepadRightTrigger(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.bRightTrigger);
    }

    // Return value of controller n Left Thumbstick X.
    SHORT getGamepadThumbLX(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.sThumbLX);
    }

    // Return value of controller n Left Thumbstick Y.
    SHORT getGamepadThumbLY(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.sThumbLY);
    }

    // Return value of controller n Right Thumbstick X.
    SHORT getGamepadThumbRX(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.sThumbRX);
    }

    // Return value of controller n Right Thumbstick Y.
    SHORT getGamepadThumbRY(UINT n) const
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        return (controllers[n].state.Gamepad.sThumbRY);
    }

    // Vibrate controller n left motor.
    // Left is low frequency vibration.
    // speed 0=off, 65536=100 percent
    // sec is time to vibrate in seconds
    void gamePadVibrateLeft(UINT n, WORD speed, float sec) 
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        controllers[n].vibration.wLeftMotorSpeed = speed;
        controllers[n].vibrateTimeLeft = sec;
    }

    // Vibrate controller n right motor.
    // Right is high frequency vibration.
    // speed 0=off, 65536=100 percent
    // sec is time to vibrate in seconds
    void gamePadVibrateRight(UINT n, WORD speed, float sec)
    {
        if(n > MAX_CONTROLLERS)
            n=MAX_CONTROLLERS;
        controllers[n].vibration.wRightMotorSpeed = speed;
        controllers[n].vibrateTimeRight = sec;
    }

    // Vibrates the connected controllers for the desired time.
    void vibrateControllers(float frameTime);



	/* ***************************************************************************************************
	/*
	/*
	/* Custom button code! Supports mouse or Controllers!! Assumes we only have 1 controller, in slot '0'. 
	/* Now included mouse code to differentiate between holding/pressing.
	/*
	/*
	/* ****************************************************************************************************/

	//If the index game pad is connected.
	bool gamepadConnected(int);

	// Save Gamepad Button down state.
    void gamepadButtonDown(int);

    // Save Gamepad Button up state.
    void gamepadButtonUp(int);

	// Save mouse button down state.
	void mouseButtonDown(int);

	// Save mouse button up state.
	void mouseButtonUp(int);

	bool Input::isGamepadButtonDown(int) const;
	bool Input::wasGamepadButtonPressed(int) const;

	bool Input::isMouseButtonDown(int) const;
	bool Input::wasMouseButtonPressed(int) const;

	// True if the player clicked something with left mouse button or A button on gamepad.
	bool isSelectionPressed() const;

	// True if the player hits backspace.
	bool isBackPressed() const;

	// True if the player hits escape or select button on gamepad.
	bool isQuitPressed() const;

	// True if the player hits space or start button on gamepad.
	bool isStartPressed() const;


	/*
		 PLAYER MOVEMENT
	*/
	// True if the player hits W or up on the left joystick on gamepad.
	bool isPlayerUpKeyDown() const;

	// True if the player hits S or down on the left joystick on gamepad.
	bool isPlayerDownKeyDown() const;

	// True if the player hits A or left on the left joystick on gamepad.
	bool isPlayerLeftKeyDown() const;

	// True if the player hits D or right on the left joystick on gamepad.
	bool isPlayerRightKeyDown() const;


	/*
		CAMERA MOVEMENT
	*/
	// True if the player hits up arrow or up on the right joystick on gamepad.
	bool isCameraUpKeyDown() const;

	// True if the player hits down arrow or down on the right joystick on gamepad.
	bool isCameraDownKeyDown() const;

	// True if the player hits left arrow or up on the left joystick on gamepad.
	bool isCameraLeftKeyDown() const;

	// True if the player hits right arrow or right on the right joystick on gamepad.
	bool isCameraRightKeyDown() const;


	// MENU MOVEMENT
	// True if the player hits up arrow key or up on the DPad or up on Left thumbstick.
	bool wasMenuUpPressed() const;

	// True if the player hits up arrow key or down on the DPad or down on Left thumbstick.
	bool wasMenuDownPressed() const;

	// True if the player hits the left arrow key or left on the DPad or left on the left thumbstick
	bool wasMenuLeftKeyPressed() const;
	
	// True if the player hits the right arrow key or right on the DPad or right on the left thumbstick 
	bool wasMenuRightKeyPressed() const;

	// True if return or A button on gamepad is pressed.
	bool wasMenuSelectPressed() const;
	
	bool wasJumpKeyPressed() const;
	
	bool isActivateKeyDown() const;

	// Returns true if 'o' or a TBD gamepad button is pressed.
	bool isOculusButtonPressed() const;
};

#endif

