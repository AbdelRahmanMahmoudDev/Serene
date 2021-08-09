#include "Serene_Game.h"
#include "Serene_Intrinsics.h"
#include "Serene_Math.h"
#include "Serene_Tiles.cpp"

// The game layer is currently only responsible for filling the sound buffer
// Sound play and update is managed by the OS
extern "C" GAME_GENERATE_AUDIO(GameGenerateAudio)
{
    i16 amplitude = 32760;
	f32 frequency = 441.0f;
	double t = (2 * Pi * frequency) / (SoundOutput->SamplesPerSecond * SoundOutput->NumberOfChannels); // wave phase
	for(i32 sampleIndex = 0; sampleIndex < SoundOutput->SoundBufferSize; ++sampleIndex)
	{
#if 0
		SoundOutput->SoundData[sampleIndex] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
		SoundOutput->SoundData[sampleIndex+1] = (u8)copysign(1.0, (amplitude * sin(t * sampleIndex)));
#else
		SoundOutput->SoundData[sampleIndex] = 0;
#endif
	}
}

internal void DrawRectangle(GameBackBuffer* Buffer, v2 MinPos, v2 MaxPos, 
                            f32 Red, f32 Green, f32 Blue)
{
	i32 IntegerMinX = Roundf32Toi32(MinPos.x);
	i32 IntegerMinY = Roundf32Toi32(MinPos.y);
	i32 IntegerMaxX = Roundf32Toi32(MaxPos.x);
	i32 IntegerMaxY = Roundf32Toi32(MaxPos.y);
	
	if(IntegerMinX < 0)
	{
		IntegerMinX = 0;
	}
    
	if(IntegerMinY < 0)
	{
		IntegerMinY = 0;
	}
    
	if(IntegerMaxX > Buffer->BitmapWidth)
	{
		IntegerMaxX = Buffer->BitmapWidth;
	}
    
	if(IntegerMaxY > Buffer->BitmapHeight)
	{
		IntegerMaxY = Buffer->BitmapHeight;	
	}
    
	
    // AA RR GG BB
	// Alpha is 24 bits to the left (not used)
	// Red is 16 bits to the left
	// Green is 8 bits to the left
	// Blue is 0 bits to the left
	
    u32 Color = ((Roundf32Tou32(Red * 255.0f) << 16) |
                 (Roundf32Tou32(Green * 255.0f) << 8) |
                 (Roundf32Tou32(Blue * 255.0f) << 0));
    	u8* Row = ((u8*)Buffer->BitmapMemory) +
    (IntegerMinY * Buffer->Pitch) +
    (IntegerMinX * Buffer->BytesPerPixel);
    
	for(i32 Y = IntegerMinY;
        Y < IntegerMaxY;
        ++Y)
	{
		u32* Pixel = (u32*)Row;
		for(i32 X = IntegerMinX;
            X < IntegerMaxX;
            ++X)
		{
			*Pixel++ = Color;
		}
		Row+= Buffer->Pitch;
	}
}


extern "C" GAME_UPDATE(GameUpdate)
{
#define TILE_WIDTH_COUNT 16
#define TILE_HEIGHT_COUNT 9
#define TILE_WIDTH  60
#define TILE_HEIGHT  60

	   	// 9 rows, 16 columns
	local_persist u32 TileChunk00[256][256] = 
	{
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
        {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
	};

	// Memory
	Assert(sizeof(GameState) <= Memory->PermanentStorageSize);
	GameState* State = (GameState*)Memory->PermanentStorage;
	if(!Memory->IsInitialized)
	{
		// Placeholder thread thing
		ThreadContext thread = {};
        
		// Setting up memory arenas
		InitializeArena(&State->WorldArena,
					 	Memory->PermanentStorageSize - sizeof(GameState),
						(u8*)Memory->PermanentStorage + sizeof(GameState));

		State->world = PushStruct(&State->WorldArena, World);
		World *world = State->world;
		world->Tiles = PushStruct(&State->WorldArena, TileMap);
		TileMap *tile_map = world->Tiles;
		
		
		tile_map->ChunkShift = 8;
		tile_map->ChunkMask = (1 << tile_map->ChunkShift) - 1;
		tile_map->ChunkDimm = 1 << tile_map->ChunkShift;
		tile_map->ChunkCountX = 32;
		tile_map->ChunkCountY = 32;
		tile_map->TileChunks = PushArray(&State->WorldArena, tile_map->ChunkCountX * tile_map->ChunkCountY, TileChunk);

		for(i32 Y = 0;
			Y < tile_map->ChunkCountY;
			++Y)
			{
				for (i32 X = 0;
					 X < tile_map->ChunkCountX;
					 ++X)
					 {
						 tile_map->TileChunks[Y * tile_map->ChunkCountX + X].Data =
						 PushArray(&State->WorldArena, tile_map->ChunkDimm * tile_map->ChunkDimm, u32);
					 }
			}

		tile_map->TileWidthInMeters = 1.4f;
		tile_map->TileWidthInPixels = 60;
		tile_map->MetersToPixels = (f32)tile_map->TileWidthInPixels / (f32)tile_map->TileWidthInMeters;
		tile_map->WidthCount = TILE_WIDTH_COUNT;
		tile_map->HeightCount = TILE_HEIGHT_COUNT;

		// The origin of the world is designed to be at the bottom left
		// This means the x origin remains the same
		// But the y origin must be moved from the top left to the bottom left
		// the bottom left is the top left offset by the bitmap height
		// This is because the OS creates a buffer that starts at the top left 
		tile_map->Origin = {0.0f, (f32)BackBuffer->BitmapHeight};
		tile_map->TileWidth =  TILE_WIDTH;
		tile_map->TileHeight =  TILE_HEIGHT;

		// TileChunk chunk;
		// chunk.Data = (u32*)TileChunk00;	
		// State
		State->PlayerPos.TileX = 3;
		State->PlayerPos.TileY = 3;
		State->PlayerPos.TileRelativePos.x = 5.0f;
		State->PlayerPos.TileRelativePos.y = 5.0f;

		for (i32 Y = 0;
			Y < 256;
			++Y)
			{
				for(i32 X = 0;
				    X < 256;
					++X)
					{
						i32 value = TileChunk00[Y][X] == 0 ? 0 : 1;
						SetTile(tile_map, X, Y, value);
					}
			}


		Memory->IsInitialized = true;
	}
    
	World *world = State->world;
	TileMap *tile_map = world->Tiles;

	f32 PlayerR = 0.3f;
	f32 PlayerG = 0.5f;
	f32 PlayerB = 0.2f;
	f32 PlayerHeight = 1.4f;
	f32 PlayerWidth = 0.75f * PlayerHeight;
    
	//Input
	for(u32 ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		GameController* Controller0 = &Input->Controllers[ControllerIndex];
		if(Controller0->IsAnalog)
		{
			//Analog processing
		}
		else
		{
			//Digital processing
            v2 PlayerAcceleration = {};
			if(Controller0->DPadUp.EndedPress)
			{
				PlayerAcceleration.y = 1.0f;
			}
            
			if(Controller0->DPadDown.EndedPress)
			{
				PlayerAcceleration.y = -1.0f;
			}
            
			if(Controller0->DPadLeft.EndedPress)
			{
				PlayerAcceleration.x = -1.0f;
			}
            
			if(Controller0->DPadRight.EndedPress)
			{
				PlayerAcceleration.x = 1.0f;
			}
			PlayerAcceleration *= 4.0f;

			// Poor man's friction
			PlayerAcceleration += -0.9f * State->PlayerVelocity;
			

			WorldPosition NewPlayerPos = State->PlayerPos;
			NewPlayerPos.TileRelativePos = (
											 (0.5f * PlayerAcceleration * Square(Input->TargetSecondsPerFrame)) +
											 (State->PlayerVelocity * Input->TargetSecondsPerFrame) + 
											 NewPlayerPos.TileRelativePos
										   );

			State->PlayerVelocity = PlayerAcceleration * Input->TargetSecondsPerFrame + State->PlayerVelocity;
			NewPlayerPos = RecanonicalizePosition(world->Tiles, NewPlayerPos);
            
			WorldPosition PlayerLeft = NewPlayerPos;
			PlayerLeft.TileRelativePos.x -= 0.5f * PlayerWidth;
			PlayerLeft = RecanonicalizePosition(world->Tiles, PlayerLeft);

			WorldPosition PlayerRight = NewPlayerPos;
			PlayerRight.TileRelativePos.y += 0.5f * PlayerWidth;
			PlayerRight = RecanonicalizePosition(world->Tiles, PlayerRight);
#if 1
			if(IsWorldPointEmpty(world->Tiles, NewPlayerPos) &&
			   IsWorldPointEmpty(world->Tiles, PlayerLeft) &&
			   IsWorldPointEmpty(world->Tiles, PlayerRight))
			{
				State->PlayerPos = NewPlayerPos;
			}
			else
			{
				// Collision logic
				v2 Normal = {0.0f, 1.0f};
				State->PlayerVelocity -= (2 * DotProd(State->PlayerVelocity, Normal) * Normal);
			}
#else
			State->PlayerPos = NewPlayerPos;
#endif
		}
	}
    
	// Audio
	if(!SoundOutput->IsBufferFilled)
	{
		ThreadContext thread = {};
		GameGenerateAudio(&thread, SoundOutput);
		SoundOutput->IsBufferFilled = true;
	}

	v2 screen_center = {0.5f * (f32)BackBuffer->BitmapWidth, 0.5f * (f32)BackBuffer->BitmapHeight};

	// Render
    for(i32 RelRow = -10;
        RelRow < 10;
        ++RelRow)
    {
        for(i32 RelColumn = -20;
            RelColumn < 20;
            ++RelColumn)
        {
			i32 Column = State->PlayerPos.TileX + RelColumn;
			i32 Row = State->PlayerPos.TileY + RelRow;
			u32 TileID = GetTile(world->Tiles, Column, Row);
            f32 Gray = 0.5f;

            if(TileID == 1)
            {
                Gray = 1.0f;
            }
            
			#if 1
			if(Column == State->PlayerPos.TileX && Row == State->PlayerPos.TileY)
			{
				Gray = 0.0f;
			}
			#endif

			// This is really confusing
			// I want to see how this can be simplified
			v2 tile_center = {screen_center.x - world->Tiles->MetersToPixels*State->PlayerPos.TileRelativePos.x + ((f32)RelColumn)*world->Tiles->TileWidthInPixels,
							  screen_center.y + world->Tiles->MetersToPixels*State->PlayerPos.TileRelativePos.y - ((f32)RelRow)*world->Tiles->TileWidthInPixels};
			v2 MinTile = {tile_center.x - 0.5f*tile_map->TileWidthInPixels,
						  tile_center.y - 0.5f*tile_map->TileWidthInPixels};

			v2 MaxTile = {tile_center.x + 0.5f*tile_map->TileWidthInPixels,
			 			  tile_center.y + 0.5f*tile_map->TileWidthInPixels};

			v2 StartVertex = {MinTile.x,
			 				  MaxTile.y};
			v2 EndVertex = {MaxTile.x,
			 				MinTile.y};

            DrawRectangle(BackBuffer, MinTile, MaxTile, Gray, Gray, Gray);
        }
    }
    

	f32 PlayerLeft = screen_center.x - 0.5f*world->Tiles->MetersToPixels*PlayerWidth;

	f32 PlayerTop = screen_center.y - world->Tiles->MetersToPixels*PlayerHeight;

	DrawRectangle(BackBuffer,
				  V2(PlayerLeft, PlayerTop),
				  V2(PlayerLeft + world->Tiles->MetersToPixels*PlayerHeight,
				     PlayerTop + world->Tiles->MetersToPixels*PlayerHeight),
                 PlayerR, PlayerG, PlayerB);
}