#include "Utils/Logger/Logger.h"

#include <fstream>
#include <mutex>

namespace Expanse::Log
{
	class TextFileSink : public ILogSink
	{
	public:
		explicit TextFileSink(const std::string& filename)
			: out(filename, std::ios::out)
		{
		}

		void Write(const std::string& msg) override
		{
			out << msg << std::endl;
		}

	private:
		std::ofstream out;
	};

	/**********************************************************************************/

	class Logger
	{
	public:

		void AddSink(std::shared_ptr<ILogSink> sink)
		{
			std::scoped_lock lock(mtx);
			sinks.push_back(sink);
		}

		void RemoveSink(std::shared_ptr<ILogSink> sink)
		{
			std::scoped_lock lock(mtx);
			std::erase(sinks, sink);
		}

		void Write(const std::string& msg)
		{
			std::scoped_lock lock(mtx);
			for (auto& sink : sinks) {
				sink->Write(msg);
			}
		}

	private:
		std::mutex mtx;
		std::vector<std::shared_ptr<ILogSink>> sinks;
	};

	/**********************************************************************************/

	static Logger logger;

	void init()
	{
		logger.AddSink( std::make_shared<TextFileSink>("log.txt") );
	}

	void add_sink(std::shared_ptr<ILogSink> sink)
	{
		logger.AddSink(sink);
	}

	void remove_sink(std::shared_ptr<ILogSink> sink)
	{
		logger.RemoveSink(sink);
	}

	void message(const std::string& msg)
	{
		logger.Write(msg);
	}
}