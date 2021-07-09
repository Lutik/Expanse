#include "Utils/Logger/Logger.h"

#include <fstream>

namespace Expanse::Log
{
	struct ILogOutput
	{
		virtual ~ILogOutput() = default;
		virtual void Write(const std::string& msg) = 0;
	};

	class TextFileLogOutput : public ILogOutput
	{
	public:
		TextFileLogOutput(const std::string& filename)
		{
			out.open(filename, std::ios::out);
		}

		~TextFileLogOutput()
		{
			out.close();
		}

		void Write(const std::string& msg) override
		{
			out << msg;
		}

	private:
		std::ofstream out;
	};

	/**********************************************************************************/

	class Logger
	{
	public:

		template<typename Output, typename... Args>
		void AddOutput(Args&&... args)
		{
			outputs.push_back(std::make_unique<Output>(std::forward<Args>(args)...));
		}

		void Write(const std::string& msg)
		{
			for (auto& out : outputs) {
				out->Write(msg);
			}
		}

	private:
		std::vector<std::unique_ptr<ILogOutput>> outputs;
	};

	/**********************************************************************************/

	static Logger logger;

	void init()
	{
		logger.AddOutput<TextFileLogOutput>("log.txt");
	}

	void message(const std::string& msg)
	{
		logger.Write(msg);
	}
}