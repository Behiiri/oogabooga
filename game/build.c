

///
// Build config stuff

#define INITIAL_PROGRAM_MEMORY_SIZE MB(5)

// You might want to increase this if you get a log warning saying the temporary storage was overflown.
// In many cases, overflowing the temporary storage should be fine since it just wraps back around and
// allocations made way earlier in the frame are likely not used anymore.
// This might however not always be the case, so it's probably a good idea to make sure you always have
// enough temporary storage for your game.
#define TEMPORARY_STORAGE_SIZE MB(2) 

// Enable VERY_DEBUG if you are having memory bugs to detect things like heap corruption earlier.
// #define VERY_DEBUG 1

typedef struct Context_Extra {
    int monkee;
} Context_Extra;
// This needs to be defined before oogabooga if we want extra stuff in context
#define CONTEXT_EXTRA Context_Extra

#define ENTRY_PROC entry

#include "../oogabooga/oogabooga.c"
#include "src/main.c"
