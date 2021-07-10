#include "Utils/Logger/Logger.h"

#include <fstream>
#include <type_traits>

namespace Expanse::Log
{
	struct ILogSink
	{
		virtual ~ILogSink() = default;
		virtual void Write(const std::string& msg) = 0;
	};

	class TextFileSink : public ILogSink
	{
	public:
		TextFileSink(const std::string& filename)
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

		template<typename Sink, typename... Args> requires std::is_base_of_v<ILogSink, Sink>
		void AddSink(Args&&... args)
		{
			sinks.push_back(std::make_unique<Sink>(std::forward<Args>(args)...));
		}

		void Write(const std::string& msg)
		{
			for (auto& sink : sinks) {
				sink->Write(msg);
			}
		}

	private:
		std::vector<std::unique_ptr<ILogSink>> sinks;
	};

	/**********************************************************************************/

	static Logger logger;

	void init()
	{
		logger.AddSink<TextFileSink>("log.txt");
	}

	void message(const std::string& msg)
	{
		logger.Write(msg);
	}
}