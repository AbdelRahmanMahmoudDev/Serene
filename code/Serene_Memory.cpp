internal void
InitializeArena(MemoryArena *arena, MemoryIndex size, u8 *base)
{
	arena->Base = base;
	arena->Size = size;
	arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void *
PushSize_(MemoryArena *arena, MemoryIndex size)
{
    Assert((arena->Used + size) <= arena->Size); // Don't overflow your arena
    void *Result = arena->Base + arena->Used; // Retrieve a pointer to the latest memory index before adding the memory
   	arena->Used += size; // add the new size to our index
    
    return(Result);
}