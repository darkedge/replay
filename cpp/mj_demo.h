// mj_demo.h

#define MJ_DEMO_MESSAGE_CONSUMER_FUNC(name) void name(void*)
typedef MJ_DEMO_MESSAGE_CONSUMER_FUNC(mjMessageConsumerFunc);

#define MJ_DEMO_MALLOC_FUNC(name) void* name(size_t)
typedef MJ_DEMO_MALLOC_FUNC(MallocFunc);

#define MJ_DEMO_FREE_FUNC(name) void name(void*)
typedef MJ_DEMO_FREE_FUNC(FreeFunc);

#define MJ_DEMO_REALLOC_FUNC(name) void* name(void*, size_t)
typedef MJ_DEMO_REALLOC_FUNC(ReallocFunc);

void mjdSetMessageConsumer();

// Demo recording
void mjdStartRecording(int slot);
void mjdNewTick();
void mjdAddMessage();
void mjdStopPlayback();
void mjdStopRecording();

// Demo playback
void mjdPlayDemo(int slot);
void mjdPlayNextTick();
