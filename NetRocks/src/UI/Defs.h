#pragma once
#include <string>
#include <mutex>

enum XferKind
{
	XK_COPY,
	XK_MOVE
};

enum XferDirection
{
	XK_DOWNLOAD,
	XK_UPLOAD
};

enum XferDefaultOverwriteAction
{
	XDOA_ASK,
	XDOA_SKIP,
	XDOA_RESUME,
	XDOA_OVERWRITE,
	XDOA_OVERWRITE_IF_NEWER,
	XDOA_CREATE_DIFFERENT_NAME
};


struct ProgressStateStats
{
	unsigned long long file_complete = 0, file_total = 0;
	unsigned long long all_complete = 0, all_total = 0;
	unsigned long long count_complete = 0, count_total = 0;
	clock_t total_start = 0, current_start = 0;
	clock_t total_paused = 0, current_paused = 0;
};

struct IAbortableOperationsHost
{
	virtual void ForcefullyAbort() = 0;
};

struct ProgressState
{
	std::mutex mtx;
	ProgressStateStats stats;
	std::string path;
	bool paused = false, aborting = false, finished = false;
	IAbortableOperationsHost *ao_host = nullptr;

	void Reset()
	{
		stats = ProgressStateStats();
		path.clear();
		paused = aborting = finished = false;
	}
};
