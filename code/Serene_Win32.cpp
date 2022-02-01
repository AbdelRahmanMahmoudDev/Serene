
#include "Serene_Core.h"
#include "Serene_Platform.h"
#include "Serene_Game.h"

#include <Windows.h>
#include <stdio.h>
#include <wrl.h>
#include <xaudio2.h>
#include <Xinput.h>

#include "3rd_Party\glad\glad\glad.h"
#include "3rd_Party\glad\glad\glad.c"
#include "Serene_OpenGL.h"

#include "Serene_Win32.h"

global b32 Win32IsRunning; 
global u32 Win32KeyCode;
global b32 Win32WasAltKeyDown;
global b32 Win32WasKeyDown;
global b32 Win32IsKeyDown;
global LARGE_INTEGER PerfFrequency;



typedef HGLRC WINAPI wglCreateContextAttribsARBPtr(HDC hdc, HGLRC hShareContext, const int* attribList);
wglCreateContextAttribsARBPtr* wglCreateContextAttribsARB;

typedef BOOL WINAPI wglChoosePixelFormatARBPtr(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList,
                                               UINT nMaxFormats, int* piFormats, UINT* mNumFormats);
wglChoosePixelFormatARBPtr* wglChoosePixelFormatARB;

typedef BOOL wglSwapIntervalEXTPtr(int interval);
wglSwapIntervalEXTPtr* wglSwapIntervalEXT;

typedef int wglGetSwapIntervalEXTPtr(void);
wglGetSwapIntervalEXTPtr* wglGetSwapIntervalEXT;

typedef DWORD xinput_get_state_ptr(DWORD dwUserIndex, XINPUT_STATE *pState);
xinput_get_state_ptr* xinput_get_state;
#define XInputGetState xinput_get_state

typedef DWORD xinput_set_state_ptr(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);
xinput_set_state_ptr* xinput_set_state;
#define XInputSetState xinput_set_state

// Initialize OpenGL for Windows
// Windows by default installs OpenGL 1.1, modern OpenGL is available on the dedicated graphics driver
// and must be loaded on Windows
// This process, as per the documentation is:
// Create a dummy render context to load the necessary extensions to create a full render context with extensions
// Set the current context
internal void
Win32InitOpenGL(Win32_OpenGLContext *opengl_context)
{
	WNDCLASSEXA DummyWindow = {};
	DummyWindow.cbSize = sizeof(DummyWindow);
	DummyWindow.hInstance = GetModuleHandleW(0);
	DummyWindow.lpfnWndProc = DefWindowProcA;
	DummyWindow.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	DummyWindow.lpszClassName = "DummyWindow";
	RegisterClassExA(&DummyWindow);
	HWND DummyWindowHandle = CreateWindowExA(0, DummyWindow.lpszClassName, "Serene: Platform Test App", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, DummyWindow.hInstance, NULL);
	if (DummyWindowHandle)
	{
		HDC DummyDC = GetDC(DummyWindowHandle);
		HGLRC DummyOpenGLContext = 0;
		PIXELFORMATDESCRIPTOR DummyDescriptor =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
			PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
			32,                   // Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                   // Number of bits for the depthbuffer
			8,                    // Number of bits for the stencilbuffer
			0,                    // Number of Aux buffers in the framebuffer.
			0,
			0,
			0, 0, 0
		};
		u32 pixelFormatResult = ChoosePixelFormat(DummyDC, &DummyDescriptor);
		Assert(pixelFormatResult != 0);
		u32 isPixelFormat = SetPixelFormat(DummyDC, pixelFormatResult, &DummyDescriptor);
		Assert(isPixelFormat == TRUE);
		DummyOpenGLContext = wglCreateContext(DummyDC);
		Assert(DummyOpenGLContext != NULL);
		u32 contextResult = wglMakeCurrent(DummyDC, DummyOpenGLContext);
		Assert(contextResult == TRUE);
        
		// This is unnecessary here, since the driver functions are loaded
		// into the game dll instead
#if 0
		u32 gladStatus = gladLoadGL();
		Assert(gladStatus != 0);
#endif
		wglCreateContextAttribsARB = (wglCreateContextAttribsARBPtr*)wglGetProcAddress("wglCreateContextAttribsARB");
		wglChoosePixelFormatARB = (wglChoosePixelFormatARBPtr*)wglGetProcAddress("wglChoosePixelFormatARB");
		wglSwapIntervalEXT = (wglSwapIntervalEXTPtr*)wglGetProcAddress("wglSwapIntervalEXT");
		wglGetSwapIntervalEXT = (wglGetSwapIntervalEXTPtr*)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglMakeCurrent(DummyDC, 0);
		wglDeleteContext(DummyOpenGLContext);
		ReleaseDC(DummyWindowHandle, DummyDC);
		DestroyWindow(DummyWindowHandle);
		i32 pixelFormatAttribs[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};
		
		i32 pixelFormat = 0;
		u32 numberOfFormats = 0;
		wglChoosePixelFormatARB(opengl_context->Win32DeviceContext, pixelFormatAttribs, 0, 1, &pixelFormat, &numberOfFormats);
		Assert(numberOfFormats != 0);
		PIXELFORMATDESCRIPTOR DesiredDescriptor = {};
		DescribePixelFormat(opengl_context->Win32DeviceContext, pixelFormat, sizeof(DesiredDescriptor), &DesiredDescriptor);
		if (!SetPixelFormat(opengl_context->Win32DeviceContext, pixelFormat, &DesiredDescriptor))
		{
			Assert(false);
		}
		i32 openGLVersion[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, opengl_context->MajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, opengl_context->MinorVersion,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};
		opengl_context->OpenGLDeviceContext = wglCreateContextAttribsARB(opengl_context->Win32DeviceContext, 0, openGLVersion);
		Assert(opengl_context->OpenGLDeviceContext != NULL);
		b32 desiredContextResult = wglMakeCurrent(opengl_context->Win32DeviceContext, opengl_context->OpenGLDeviceContext);
		Assert(desiredContextResult == TRUE);
	}
	else
	{
		Assert(false);
	}
}


// This function initializes XAudio2. This library requires a number of steps described by the Microsoft documentation
// 1.Initialize the COM Interface. This API relies heavily on COM and can't be used without it.
// 2.Load the XAudio2 dll based on the available version. XAudio2 is readily available on all windows installations and doesn't need redistribution
// 3.Retrieve an XAudio2 interface object
// 4.Create a mastering voice, which is a representation of the audio hardware
internal void
Win32InitXAudio2(Win32SoundOutput *Win32Sound)
{
	HRESULT debugResult = 0;
	debugResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	Assert(SUCCEEDED(debugResult));
	HMODULE XAudio2Library = LoadLibraryA("XAUDIO2_9.DLL");
    if (!XAudio2Library)
    {
        XAudio2Library = LoadLibraryA("XAUDIO2_8.DLL");
    }
    if (!XAudio2Library)
    {
        XAudio2Library = LoadLibraryA("XAUDIO2_7.DLL");
    }
    if(!XAudio2Library)
    {
        MessageBoxA(NULL, "Couldn't find supported version of XAudio2. Audio support disabled!", "Warning", MB_OK | MB_ICONWARNING);
        return;
    }
    debugResult = XAudio2Create(&Win32Sound->XAudioObject, 0, XAUDIO2_DEFAULT_PROCESSOR);
    Assert(SUCCEEDED(debugResult));
    debugResult = Win32Sound->XAudioObject->CreateMasteringVoice(&Win32Sound->Win32MasterVoice);
    Assert(SUCCEEDED(debugResult));
    Win32Sound->NumberOfChannels = 2; 
    Win32Sound->SamplesPerSecond = 44100;
    Win32Sound->BitsPerSample = 16;
    Win32Sound->BytesPerSample = 16 / 8;
    Win32Sound->SoundBufferSize = ((u64)Win32Sound->SamplesPerSecond * (u64)Win32Sound->BytesPerSample);
    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = Win32Sound->NumberOfChannels;
    waveFormat.nSamplesPerSec = Win32Sound->SamplesPerSecond;
    waveFormat.wBitsPerSample = Win32Sound->BitsPerSample;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    debugResult = Win32Sound->XAudioObject->CreateSourceVoice(&Win32Sound->Win32SourceVoice, &waveFormat);
    Assert(SUCCEEDED(debugResult));
}

/*
Sets up the XAUDIO2 sound buffer and submits it to the source  voice
@TODO: Currently, the audio we use is a simple wave that requires a single source voice, later on when we build the audio mixer, this will change
*/
internal void
Win32SubmitAudio(Win32SoundOutput *Win32Sound)
{
    HRESULT debugResult = 0;
    Win32Sound->SoundBuffer = {};
    Win32Sound->SoundBuffer.Flags = XAUDIO2_END_OF_STREAM;
    Win32Sound->SoundBuffer.AudioBytes = TruncateU64(Win32Sound->SoundBufferSize);
    Win32Sound->SoundBuffer.pAudioData = Win32Sound->SoundData;
    Win32Sound->SoundBuffer.PlayBegin = 0;
    Win32Sound->SoundBuffer.PlayLength = 0;
    Win32Sound->SoundBuffer.LoopBegin = 0;
    Win32Sound->SoundBuffer.LoopLength = 0;
    Win32Sound->SoundBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    Win32Sound->SoundBuffer.pContext = NULL;
    debugResult = Win32Sound->Win32SourceVoice->SubmitSourceBuffer(&Win32Sound->SoundBuffer);       
    Assert(SUCCEEDED(debugResult));
}

/*
Initialize XInput
Grab the dll from disk(ships with Windows)
If there is no version, a warning is displayed
Import 2 functions needed to use the API
*/
internal void
Win32InitXInput()
{
	
	HMODULE XInputLibrary = LoadLibraryA("Xinput1_4.dll");
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("Xinput9_1_0.dll");
	}
	if(!XInputLibrary)
	{
		XInputLibrary = LoadLibraryA("Xinput1_3.dll");
	}
	if(!XInputLibrary)
	{
		MessageBoxA(NULL, "Could not find a supported version of XInput! Controller support disabled!", "Warning", MB_OK | MB_ICONWARNING);
	}
	XInputGetState = (xinput_get_state_ptr*)GetProcAddress(XInputLibrary, "XInputGetState");
	XInputSetState = (xinput_set_state_ptr*)GetProcAddress(XInputLibrary, "XInputSetState");
}

//Process digital buttons for XInput and Create input state for the game layer
/*
@Param(DWORD ButtonState) Bitfield for gamepad buttons
@Param(GameButtonState* OldState) The state of the controller in the last poll
@Param(GameButtonState* NewState) The state of the controller in the new poll
@Param(DWORD ButtonBit) bit to be masked in XINPUT_GAMEPAD structure
If the button bit was high, then the button state(Ended press) is true
If the Old controller press state not equal to the new controller press state, then a transition happended
*/
internal void
Win32ProcessDigitalButton(DWORD ButtonState, GameButtonState *OldState, GameButtonState *NewState, DWORD ButtonBit)
{
	NewState->EndedPress = ((ButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedPress != NewState->EndedPress) ? 1 : 0;
}

internal void
Win32ProcessKeyboardButton(GameButtonState *NewState, b32 KeyState)
{
	if(NewState->EndedPress != KeyState)
	{
		NewState->EndedPress = KeyState;
		++NewState->HalfTransitionCount;
	}
}

internal f32
Win32ProcessStickValue(SHORT value, SHORT deadzone)
{
	f32 result = 0;
    
	if(value < -deadzone)
	{
		result = (f32)((value + deadzone) / (32768.0f - deadzone));
	}
	else if(value > deadzone)
	{
		result = (f32)((value - deadzone) / (32767.0f - deadzone));
	}
    
	return result;
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DebugPlatformFreeFileMemory)
{
	if(FreeableMemory)
	{
		VirtualFree(FreeableMemory, 0, MEM_RELEASE);
	}
	else
	{
		//Log
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DebugPlatformReadEntireFile)
{
	DebugPlatformReadFileResult Result = {};
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = TruncateU64(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if(Result.Content)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
				{
					Result.ContentSize = FileSize32;
				}
				else
				{
					//ReadFile Failed
					ThreadContext thread = {};
					DebugPlatformFreeFileMemory(&thread, Result.Content);
                    
                    
                    
                    
					Result.Content = 0;
				}
			}
			else
			{
				//File allocation failure
			}
		}
		else
		{
			//Couldn't get file size
		}
        
		CloseHandle(FileHandle);
	}
	else
	{
		//Couldn't Create file handle
	}
    
	return Result;
}



DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DebugPlatformWriteEntireFile)
{
	b32 Result = false;
    
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, WriteableMemory, MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
			//Couldn't write stuff
		}
		CloseHandle(FileHandle);
	}
	else
	{
		//File handle error
	}
	return Result;
}

//Builds a string of the executable path + FileName
internal void
Win32BuildEXEPath(Win32State *Win32_State, char *FileName, char *DestinationFilePath)
{
	CatStrings(Win32_State->OnePastLastSlash - Win32_State->EXEFilePath, Win32_State->EXEFilePath,
			   StringLength(FileName), FileName,
			   StringLength(DestinationFilePath), DestinationFilePath);
}

internal void
Win32BeginRecordingInput(Win32State *Win32_State, i32 InputRecordingIndex)
{
	Win32_State->InputRecordingIndex = InputRecordingIndex;
    
	char GamePlaybackPath[WIN32_MAX_DIR];
	Win32BuildEXEPath(Win32_State, "GamePlayback.spb", GamePlaybackPath);
	Win32_State->RecordingHandle = CreateFileA(GamePlaybackPath, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    
	DWORD BytesToWrite = (DWORD)Win32_State->TotalSize;
	Assert(Win32_State->TotalSize == BytesToWrite); //This is due to down-cast
	DWORD BytesWritten;
	WriteFile(Win32_State->RecordingHandle, Win32_State->GameMemoryBlock, BytesToWrite, &BytesWritten, 0);
}

internal void
Win32EndRecordingInput(Win32State *Win32_State)
{
	CloseHandle(Win32_State->RecordingHandle);
	Win32_State->InputRecordingIndex = 0;
}

internal void
Win32BeginInputPlayback(Win32State *Win32_State, i32 InputPlayingIndex)
{
	Win32_State->InputPlayingIndex = InputPlayingIndex;
	
	char GamePlaybackPath[WIN32_MAX_DIR];
	Win32BuildEXEPath(Win32_State, "GamePlayback.spb", GamePlaybackPath);
	Win32_State->PlaybackHandle = CreateFileA(GamePlaybackPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    
	DWORD BytesToRead = (DWORD)Win32_State->TotalSize;
	Assert(Win32_State->TotalSize == BytesToRead); //This is due to down-cast
	DWORD BytesRead;
	ReadFile(Win32_State->PlaybackHandle, Win32_State->GameMemoryBlock, BytesToRead, &BytesRead, 0);
}

internal void
Win32EndInputPlayback(Win32State *Win32_State)
{
	CloseHandle(Win32_State->PlaybackHandle);
	Win32_State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(Win32State *Win32_State, GameInput *NewInput)
{
	DWORD BytesWritten;
	WriteFile(Win32_State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlaybackInput(Win32State *Win32_State, GameInput *NewInput)
{
	DWORD BytesRead = 0;
	if(ReadFile(Win32_State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
	{
		if(BytesRead == 0)
		{			
			//At end of stream, loop back to beginning
			i32 PlayingIndex = Win32_State->InputPlayingIndex;
			Win32EndInputPlayback(Win32_State);
			Win32BeginInputPlayback(Win32_State, PlayingIndex);
			ReadFile(Win32_State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
		}
	}
}

// Taken from Raymond Chen's blog
// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
internal void
Win32ToggleFullscreen(HWND hwnd)
{
    DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
    
    if (dwStyle & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(hwnd, &g_wpPrev) &&
            GetMonitorInfo(MonitorFromWindow(hwnd,
                                             MONITOR_DEFAULTTOPRIMARY), &mi))
        {
            SetWindowLong(hwnd, GWL_STYLE,
                          dwStyle & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,	
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(hwnd, GWL_STYLE,
                      dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(hwnd, &g_wpPrev);
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}	

internal void
Win32ProcessMessages(Win32State *Win32_State, GameController *KeyboardController)
{
    //Input polling
    MSG Message;
    while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                Win32IsRunning = false;
            }break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                /* As per the Win32 documentation:
                 * Key codes are stored in the wParam of a MSG 
                 * The key states (is key down, was key down, was alt key down)
                 * are stored in a bitmask, which is available in the documentation.
                 */
                Win32KeyCode = (u32)Message.wParam;
                Win32IsKeyDown = ((u32)Message.lParam & (1 << 31)) == 0;
                Win32WasKeyDown = ((u32)Message.lParam & (1 << 30)) != 0;
                Win32WasAltKeyDown = ((u32)Message.lParam & (1 << 29)) != 0;
                
                if (Win32WasKeyDown != Win32IsKeyDown)
                {
                    switch (Win32KeyCode) 
                    {
                        case VK_F4:
                        {
                            if (Win32WasAltKeyDown)
                            {
                                Win32IsRunning = false;
                            }
                        }break;
                        
                        case VK_RETURN:
                        {
                            if(Win32IsKeyDown)
                            {
                                if(Win32WasAltKeyDown)
                                {
                                    Win32ToggleFullscreen(Message.hwnd);
                                }
                            }
                        }break;
                        
                        case 'W':
                        {
                            Win32ProcessKeyboardButton(&KeyboardController->DPadUp, Win32IsKeyDown);
                        }break;
                        
                        case 'A':
                        {
                            Win32ProcessKeyboardButton(&KeyboardController->DPadLeft, Win32IsKeyDown);
                        }break;
                        
                        case 'S':
                        {
                            Win32ProcessKeyboardButton(&KeyboardController->DPadDown, Win32IsKeyDown);
                        }break;
                        
                        case 'D':
                        {
                            Win32ProcessKeyboardButton(&KeyboardController->DPadRight, Win32IsKeyDown);
                        }break;
                        
#if SERENE_INTERNAL
                        case 'L':
                        {
                            if(Win32IsKeyDown)
                            {
                                if(Win32_State->InputRecordingIndex == 0)
                                {
                                    Win32BeginRecordingInput(Win32_State, 1);
                                }
                                else
                                {
                                    Win32EndRecordingInput(Win32_State);
                                    Win32BeginInputPlayback(Win32_State, 1);
                                }
                            }
                        }break;
#endif
                    }
                }
            }break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            }break;
        }
        
    }
}


// Query the clock for the elapsed time
internal LARGE_INTEGER
Win32GetWallClock()
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return Result;
}


// Get seconds elapsed in a frame
internal f64
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	f64 ElapsedTime = ((f64)End.QuadPart - (f64)Start.QuadPart);
	f64 Result = (f64)(ElapsedTime / (f64)PerfFrequency.QuadPart);
	return Result;
}

// Get the last time the file was modified
internal FILETIME
Win32GetLastWriteTime(char *FileName)
{
	FILETIME Result = {};
    
	WIN32_FILE_ATTRIBUTE_DATA Data;
	if(GetFileAttributesExA(FileName, GetFileExInfoStandard, &Data))
	{
		Result = Data.ftLastWriteTime;
	}
    
	return Result;
}


// Copy dll to write to it while original one is locked
internal Win32GameCode
Win32LoadGameDLL(char *SourceDLL, char *TempDLL)
{
	Win32GameCode Result = {};
    
	Result.LastWriteTime = Win32GetLastWriteTime(SourceDLL);
    
	CopyFileA(SourceDLL, TempDLL, FALSE); 
	Result.GameLibrary = LoadLibraryA(TempDLL);
    
	if(Result.GameLibrary)
	{
		Result.GameGenerateAudio = (game_generate_audio*)GetProcAddress(Result.GameLibrary, "GameGenerateAudio");
		Result.GameUpdate = (game_update*)GetProcAddress(Result.GameLibrary, "GameUpdate");
        
		Result.IsDLLValid = (Result.GameGenerateAudio && Result.GameUpdate);
	}
	else
	{
		//Failed to load dll
	}
    
	if(!Result.IsDLLValid)
	{
		Result.GameGenerateAudio = 0;
		Result.GameUpdate = 0;
	}
    
	return Result;
}

internal void
Win32UnloadGameDLL(Win32GameCode *GameCode)
{
	if(GameCode->GameLibrary)
	{
		FreeLibrary(GameCode->GameLibrary);
		GameCode->GameLibrary = 0;
	}
    
	GameCode->IsDLLValid = false;
	GameCode->GameUpdate = 0;
	GameCode->GameGenerateAudio = 0;
}

LRESULT CALLBACK
WindowsCallBack(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{ 
	LRESULT result = 0;
    
	switch (msg)
	{	
        case WM_ACTIVATEAPP:
        {
#if 0
            if(wParam == TRUE)
            {
                SetLayeredWindowAttributes(handle, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(handle, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        }break;
        
        case WM_CLOSE:
        {
            Win32IsRunning = false;
        }break;
        
        case WM_DESTROY:
        {
            Win32IsRunning = false;
        }break;
        
        default:
        {
            result = DefWindowProcA(handle, msg, wParam, lParam);
        }break;
	}
    
	return result;
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	QueryPerformanceFrequency(&PerfFrequency);
    
	u32 DesiredSchedulerMS = 1;
	b32 IsSleepGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
	Win32State Win32_State = {};
	GetModuleFileNameA(NULL, Win32_State.EXEFilePath, sizeof(Win32_State.EXEFilePath));
	for(char* Scan = Win32_State.EXEFilePath; *Scan; ++Scan)
	{
		if(*Scan == '\\')
		{
			Win32_State.OnePastLastSlash = Scan + 1;
		}
	}
    
	char SourceGameDLLFullPath[WIN32_MAX_DIR];
	Win32BuildEXEPath(&Win32_State , "Serene_Game.dll", SourceGameDLLFullPath);
    
	char TempGameDLLFullPath[WIN32_MAX_DIR];
	Win32BuildEXEPath(&Win32_State, "Serene_Game_temp.dll", TempGameDLLFullPath);
    
	char AssetPath[WIN32_MAX_DIR];
	Win32BuildEXEPath(&Win32_State, "..\\data", AssetPath);
    
	GameAssetPath asset_path = {};
	asset_path.AssetPath = AssetPath;
    
	Win32InitXInput();
    
	Win32SoundOutput Win32Sound = {};
	Win32InitXAudio2(&Win32Sound);
    
    
	WNDCLASSEXA Window = {};
	Window.hInstance = hInstance;
	Window.cbSize = sizeof(Window);
	Window.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	Window.lpfnWndProc = WindowsCallBack;
	Window.cbClsExtra = 0;
	Window.cbWndExtra = 0;
	Window.hIcon = LoadIconA(hInstance, IDI_APPLICATION);
	Window.hCursor = LoadCursorA(NULL, IDC_ARROW);
	Window.lpszMenuName = NULL;
	Window.lpszClassName = "MainWindow";
	Window.hIconSm = LoadIconA(Window.hInstance, IDI_APPLICATION);
    
	RegisterClassExA(&Window);
    
	RECT DrawableRect = {};
	DrawableRect.right = 1280;
	DrawableRect.bottom = 720;
    
	Win32WindowData window_data = {};
	window_data.Width = DrawableRect.right;
	window_data.Height = DrawableRect.bottom;
    
	DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	AdjustWindowRect(&DrawableRect, window_style, 0);
    
	HWND WindowHandle = CreateWindowExA(0, "MainWindow", "Serene: Platform Test App",
									    window_style,
		                                CW_USEDEFAULT,
										CW_USEDEFAULT,
										DrawableRect.right - DrawableRect.left,
										DrawableRect.bottom - DrawableRect.top,
										0, 0, hInstance, 0);
    
	ShowWindow(WindowHandle, nCmdShow);
	UpdateWindow(WindowHandle);
    
	// Init OpenGL
	Win32_OpenGLContext opengl_render_context = {};
	opengl_render_context.Win32DeviceContext = GetDC(WindowHandle);
	opengl_render_context.MajorVersion = 4;
	opengl_render_context.MinorVersion = 6;
	Win32InitOpenGL(&opengl_render_context);
	wglSwapIntervalEXT(0);
    
	if (WindowHandle)
	{
		//Placeholder thread thing
		ThreadContext Thread = {};
        
		HDC TempContext = GetDC(WindowHandle);
		i32 RetrievedRefreshRate = GetDeviceCaps(TempContext, VREFRESH);
		ReleaseDC(WindowHandle, TempContext);
        
		//TODO: Find a way to query this on the platform
		u32 MonitorRefreshRate = 0;
		if(RetrievedRefreshRate > 1)
		{
			MonitorRefreshRate = RetrievedRefreshRate; 
		}
		else
		{
			MonitorRefreshRate = 60;
		}
		u32 GameRefreshRate = MonitorRefreshRate; //Try to target monitor refresh rate (30 frames at minimum)
		f32 TargetSecondsPerFrame = 1.0f / (f32)GameRefreshRate;
        
#ifdef SERENE_INTERNAL
		LPVOID BaseAddress = (LPVOID)TERABYTES(2);
#else
		LPVOID BaseAddress = 0;
#endif
        
        
		GameMemory Memory = {};
		Memory.DebugPlatformFreeFileMemory = DebugPlatformFreeFileMemory;
        
		Memory.DebugPlatformReadEntireFile = DebugPlatformReadEntireFile;
		Memory.DebugPlatformWriteEntireFile = DebugPlatformWriteEntireFile;
		Memory.PermanentStorageSize = GIGABYTES(2);
		Memory.TransientStorageSize = GIGABYTES(2);
		Win32_State.TotalSize = Memory.TransientStorageSize + Memory.PermanentStorageSize;
        
		Win32_State.GameMemoryBlock = VirtualAlloc(BaseAddress, Win32_State.TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		Memory.PermanentStorage = Win32_State.GameMemoryBlock;
		Memory.TransientStorage = ((u8*)Memory.PermanentStorage + Memory.PermanentStorageSize);
		Assert(Memory.PermanentStorage && Memory.TransientStorage);
        
		// Allocate audio memory
		// TODO(abdo): This should be pooled with other disk i/o
		Win32Sound.SoundData = (u8*)VirtualAlloc(0, Win32Sound.SoundBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		Assert(Win32Sound.SoundData);
        
		GameSoundOutput GameAudio = {};
		GameAudio.BitsPerSample    = Win32Sound.BitsPerSample;
		GameAudio.BytesPerSample   = Win32Sound.BytesPerSample;
		GameAudio.NumberOfChannels = Win32Sound.NumberOfChannels;
		GameAudio.SamplesPerSecond = Win32Sound.SamplesPerSecond;
		GameAudio.SoundBufferSize  = Win32Sound.SoundBufferSize;
		GameAudio.SoundData 	   = Win32Sound.SoundData;
		GameAudio.IsBufferFilled   = false;
		
		Win32SubmitAudio(&Win32Sound);
        Win32Sound.Win32SourceVoice->Start();
        
		GameRendererDimensions game_renderer_dimensions = {};
		game_renderer_dimensions.ScreenWidth = window_data.Width;
		game_renderer_dimensions.ScreenHeight = window_data.Height;
        
		//Setup Input
		GameInput Input[2] = {};
		GameInput* OldInput = &Input[0];
		GameInput* NewInput = &Input[1];
        
		Win32GameCode GameCode = Win32LoadGameDLL(SourceGameDLLFullPath, TempGameDLLFullPath);
        
		i64 LastCycleCount = __rdtsc();
		LARGE_INTEGER LastCounter = Win32GetWallClock();
		Win32IsRunning = true;
		while (Win32IsRunning)
		{
			NewInput->TargetSecondsPerFrame = TargetSecondsPerFrame;
			FILETIME SourceDLLWriteTime = Win32GetLastWriteTime(SourceGameDLLFullPath);
			if(CompareFileTime(&GameCode.LastWriteTime, &SourceDLLWriteTime) != 0)
			{
				Win32UnloadGameDLL(&GameCode);
				GameCode = Win32LoadGameDLL(SourceGameDLLFullPath, TempGameDLLFullPath);
			}
            
			GameController* OldKeyboardController = GetController(OldInput, 0);
			GameController* NewKeyboardController = GetController(NewInput, 0);
			*NewKeyboardController = {};
			
			NewKeyboardController->IsConnected = true;
            
			for(i32 ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
			{
				NewKeyboardController->Buttons[ButtonIndex].EndedPress = OldKeyboardController->Buttons[ButtonIndex].EndedPress;
			}
            
			Win32ProcessMessages(&Win32_State, NewKeyboardController);
            
			//Gamepad input
			u32 MaxControllerCount = XUSER_MAX_COUNT;
			if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
			{
				MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
			}
            
			XINPUT_STATE ControllerState;
			for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
			{
				DWORD GamePadControllerIndex = ControllerIndex + 1;
				GameController* NewController = GetController(NewInput, GamePadControllerIndex);
				GameController* OldController = GetController(OldInput, GamePadControllerIndex);
                
				if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
				{
					XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;
					NewController->IsConnected = true;
					NewController->IsAnalog = OldController->IsAnalog;
					
					//Digital input
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadUp, &NewController->DPadUp, XINPUT_GAMEPAD_DPAD_UP);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadDown, &NewController->DPadDown, XINPUT_GAMEPAD_DPAD_DOWN);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadLeft, &NewController->DPadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->DPadRight, &NewController->DPadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
                    
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Start, &NewController->Start, XINPUT_GAMEPAD_START);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Back, &NewController->Back, XINPUT_GAMEPAD_BACK);
                    
                    //LS, RS (L3, R3)
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->LeftThumb, &NewController->LeftThumb, XINPUT_GAMEPAD_LEFT_THUMB);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->RightThumb, &NewController->RightThumb, XINPUT_GAMEPAD_LEFT_THUMB);
                    
					//(L1, R1)
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->LeftShoulder, &NewController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->RightShoulder, &NewController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER);
                    
					//A, B, X, Y
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->A, &NewController->A, XINPUT_GAMEPAD_A);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->B, &NewController->B, XINPUT_GAMEPAD_B);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->X, &NewController->X, XINPUT_GAMEPAD_X);
					Win32ProcessDigitalButton(Pad->wButtons, &OldController->Y, &NewController->Y, XINPUT_GAMEPAD_Y);
					
                    
					//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
					//#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
				    //#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30 (Do we really need this)??
					NewController->IsAnalog = true;
					//Analog input
					//L2
					NewController->LeftTriggerAverage = Pad->bLeftTrigger / 255.0f;
                    
					//R2
					NewController->RightTriggerAverage = Pad->bRightTrigger / 255.0f;
                    
					//Left thumb x axis
					NewController->LeftStickAverageX = Win32ProcessStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    
					//left thumb y axis
					NewController->LeftStickAverageY = Win32ProcessStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                    
					//right thumb x axis
					NewController->RightStickAverageX = Win32ProcessStickValue(Pad->sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
                    
					//right thumb y axis
					NewController->RightStickAverageY = Win32ProcessStickValue(Pad->sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
					NewController->IsAnalog = false;
				}
				else
				{
					//Controller is not connected
					NewController->IsConnected = false;
				}
			}
            
			//Lazy mouse handling
			POINT MousePos;
			GetCursorPos(&MousePos);
			ScreenToClient(WindowHandle, &MousePos);
			NewInput->MouseX = MousePos.x;
			NewInput->MouseY = MousePos.y;
			NewInput->MouseZ = 0; // Scroll wheel
			Win32ProcessKeyboardButton(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
			Win32ProcessKeyboardButton(&NewInput->MouseButtons[1], GetKeyState(VK_RBUTTON) & (1 << 15));
			Win32ProcessKeyboardButton(&NewInput->MouseButtons[2], GetKeyState(VK_MBUTTON) & (1 << 15));
			Win32ProcessKeyboardButton(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
			Win32ProcessKeyboardButton(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
            
			//Debug looped playback
			if(Win32_State.InputRecordingIndex)
			{
				Win32RecordInput(&Win32_State, NewInput);
			}
            
			if(Win32_State.InputPlayingIndex)
			{
				Win32PlaybackInput(&Win32_State, NewInput);
			}
            
			if(GameCode.GameUpdate && GameCode.GameGenerateAudio)
			{
				GameCode.GameUpdate(&Thread, &Memory, &game_renderer_dimensions, NewInput, &GameAudio, &asset_path);
			}
            
			BOOL swap_result = SwapBuffers(opengl_render_context.Win32DeviceContext);
			DWORD error_code = GetLastError();
			//Swap input states to always get latest state
			GameInput* temp = NewInput;
			NewInput = OldInput;
			OldInput = temp;
            
			/*
			Performance Calculation
			TODO: Better way to log performance numbers
			*/
            
			LARGE_INTEGER WorkCounter = Win32GetWallClock();
			f64 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
            
			f64 SecondsElapsedForFrame = WorkSecondsElapsed;
			if(SecondsElapsedForFrame < TargetSecondsPerFrame)
			{
				if(IsSleepGranular)
				{
					DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                    
					if(SleepMS > 0)
					{
						Sleep(SleepMS);
					}
				}
                
				while(SecondsElapsedForFrame < TargetSecondsPerFrame)
				{
					SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
				}
			}
			else
			{
				//Log
			}
            
			i64 EndCycleCount = __rdtsc();
			LARGE_INTEGER EndCounter = Win32GetWallClock();
            
			f64 MegaCyclesElapsed = ((f64)EndCycleCount - (f64)LastCycleCount) / (1000.0f * 1000.0f);
			f64 MilliSecondsElapsed = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
			f64 FramesPerSecond = 1.0f / Win32GetSecondsElapsed(LastCounter, EndCounter);
            
			char buffer[512];
			sprintf_s(buffer, sizeof(buffer), "ms/frame: %.2f	Mc/frame: %.2f	FPS: %.2f\n", MilliSecondsElapsed, MegaCyclesElapsed, FramesPerSecond);
			OutputDebugStringA(buffer);
            
			LastCounter = EndCounter;
			LastCycleCount = EndCycleCount;
		}
	}
	else
	{
		//Logging
		OutputDebugStringA("Failed to create Window!");
	}
	return 0;
}