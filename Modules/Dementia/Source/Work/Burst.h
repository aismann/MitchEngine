#pragma once
#include "Dementia.h"

#if ME_PLATFORM_UWP || ME_PLATFORM_WIN64

#include <wtypes.h>
#include <functional>
#include <assert.h>


// The work item queue for each per-chunk worker thread
const int                   g_iSceneQueueSizeInBytes = 16 * 1024;
typedef unsigned char                ChunkQueue[g_iSceneQueueSizeInBytes];

static const int kMaxPerChunkThreads = 32;
static const int kMaxPendingQueueEntries = 1024;     // Max value of g_hBeginPerChunkRenderDeferredSemaphore

class Burst
{
public:
	// The different types of job in the per-chunk work queues
	enum JobType
	{
		Setup,
		Work,
		Finalize,
		Count
	};

	// The contents of the work queues depend on the job type...
	struct WorkQueueEntryBase
	{
		JobType m_iType;
	};

	struct WorkQueueEntrySetup : public WorkQueueEntryBase
	{
		std::function<void(int Index)> m_callBack;
	};

	struct LambdaWorkEntry : public WorkQueueEntryBase
	{
		std::function<void(int Index)> m_callBack;
	};

	struct WorkQueueEntryFinalize : public WorkQueueEntryBase
	{
		std::function<void(int Index)> m_callBack;
	};

	Burst() = default;

	int GetPhysicalProcessorCount();
	void InitializeWorkerThreads();
	static unsigned int _BurstThread(LPVOID lpParameter);

	void PrepareWork();
	void PrepareWork(std::function<void(int Index)> PerThreadPrep);

	void AddWork2(Burst::LambdaWorkEntry& NewWork, int InSize);

	void FinalizeWork();
	void FinalizeWork(std::function<void(int Index)> PerThreadFinal);


	ChunkQueue& GetChunkQueue(int i);
	int GetChunkQueueOffset(int i);

	void SetChunkOffset(int i, int size);

	void GenerateChunks(std::size_t size, std::size_t num, std::vector<std::pair<int, int>>& OutChunks);

	HANDLE GetBeginSemaphore(int i);

	int GetNumThreads();

private:
	int ThreadCount = 0;

};
#endif
