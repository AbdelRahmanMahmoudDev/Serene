internal void
CatStrings(i64 SourceACount, char *SourceA,
		   i64 SourceBCount, char *SourceB,
		   i64 DestinationCount, char *Destination)
{
	for(i32 index = 0; index < SourceACount; ++index)
	{
		*Destination++ = *SourceA++;
	}

	for(i32 index = 0; index < SourceBCount; ++index)
	{
		*Destination++ = *SourceB++;
	}

	//Add null termination
	*Destination++ = 0;
}


internal u32
StringLength(char *String)
{
	i32 result = 0;
	while(*String++)
	{
		result++;
	}

	return result;
}

internal void
StringCopy(char *dest, char *src, MemoryIndex offset) 
{ 
	MemoryIndex index;
for (index = 0; index < offset; ++index) 
	{
		*dest++ = *src++; 
	}
} 