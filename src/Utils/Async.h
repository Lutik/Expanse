#pragma once

#include <future>
#include <queue>

namespace Expanse::utils
{
	class ThreadPool
	{
	public:
		ThreadPool(unsigned int thread_count = std::thread::hardware_concurrency());
		~ThreadPool();

		void Run(std::function<void()> task);
	private:
		void WorkerThread();
		std::function<void()> PopTask();
	
		std::mutex queue_mutex;
		std::condition_variable cv;
		std::queue<std::function<void()>> tasks;

		std::vector<std::thread> threads;

		std::atomic<bool> running = true;
	};

	void AsyncVoid(std::function<void()> func);

	template<typename Func, typename... Args>
	[[nodiscard]] auto Async(Func&& func, Args&&... args)
	{
		using RetType = std::invoke_result_t<Func, Args...>;
		using TaskType = std::packaged_task<RetType()>;

		auto task_ptr = std::make_shared<TaskType>(std::bind_front(std::forward<Func>(func), std::forward<Args>(args)...));
		auto result = task_ptr->get_future();

		AsyncVoid([task_ptr]{ (*task_ptr)(); });

		return result;
	}

}