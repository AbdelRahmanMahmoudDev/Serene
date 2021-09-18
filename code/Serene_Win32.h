#if !defined(SERENE_WIN32_H)

struct Win32SoundOutput
{
	Microsoft::WRL::ComPtr<IXAudio2> XAudioObject;
	IXAudio2MasteringVoice* Win32MasterVoice;
	IXAudio2SourceVoice* Win32SourceVoice;
    XAUDIO2_BUFFER SoundBuffer;
	u64 SoundBufferSize;
	u16 NumberOfChannels;
	u32 SamplesPerSecond;
	u32 BytesPerSample;
	u16 BitsPerSample;
	u8* SoundData;
};

struct Win32WindowData
{
	u32 Width;
	u32 Height;
};

#define WIN32_MAX_DIR MAX_PATH
struct Win32State
{
    u64 TotalSize;
    void *GameMemoryBlock;
    
    HANDLE RecordingHandle;
    i32 InputRecordingIndex;

    HANDLE PlaybackHandle;
    i32 InputPlayingIndex;    

	char EXEFilePath[WIN32_MAX_DIR];
	char* OnePastLastSlash;
};

struct Win32GameCode
{
	HMODULE GameLibrary;
	FILETIME LastWriteTime;
	game_generate_audio *GameGenerateAudio;
	game_update *GameUpdate;
	b32 IsDLLValid;
};

struct Win32_OpenGLContext
{
	HDC Win32DeviceContext;
	HGLRC OpenGLDeviceContext;
	i32 MajorVersion;
	i32 MinorVersion;
};

#define SERENE_WIN32_H
#endif