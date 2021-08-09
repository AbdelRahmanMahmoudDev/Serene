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

struct Win32BackBuffer
{
	void* BitmapMemory;
	BITMAPINFO BitmapInfo;
	HBITMAP BitmapHandle;
	i32 BitmapWidth;
	i32 BitmapHeight;
	i32 BytesPerPixel;
	i32 Pitch;
};

struct Win32WindowDimensions
{
	i32 Width;
	i32 Height;
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

	char EXEFileName[WIN32_MAX_DIR];
	char* OnePastLastSlash;
};

struct Win32GameCode
{
	HMODULE GameLibrary;
	FILETIME LastWriteTime;
	game_generate_audio* GameGenerateAudio;
	game_update*		 GameUpdate;
	b32 IsDLLValid;
};

#define SERENE_WIN32_H
#endif