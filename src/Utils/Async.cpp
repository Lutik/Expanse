#include "Async.h"

namespace Expanse::utils
{
	ThreadPool::ThreadPool(unsigned int thread_count)
		: threads(thread_count)
	{
		for (auto& worker_thread : threads)
		{
			worker_thread = std::thread(&ThreadPool::WorkerThread, this);
		}
	}

	ThreadPool::~ThreadPool()
	{
		running = false;
		cv.notify_all();

		for (auto& worker_thread : threads)
		{
			worker_thread.join();
		}
	}

	void ThreadPool::Run(std::function<void()> task)
	{
		{
			std::scoped_lock lock(queue_mutex);
			tasks.push(std::move(task));
		}
		cv.notify_one();
	}

	std::function<void()> ThreadPool::PopTask()
	{
		std::function<void()> task;

		std::unique_lock lock(queue_mutex);

		auto can_stop = [this](){ return !tasks.empty() || !running; };
		cv.wait(lock, can_stop);	

		if (running) {
			task = std::move(tasks.front());
			tasks.pop();
		}

		return task;
	}

	void ThreadPool::WorkerThread()
	{
		while (running)
		{
			if (auto task = PopTask()) {
				task();
			}
		}
	}



	static ThreadPool thread_pool;

	void AsyncVoid(std::function<void()> func)
	{
		thread_pool.Run(func);
	}
}