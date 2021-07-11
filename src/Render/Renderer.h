#pragma once

namespace Expanse
{
	class OpenGLRenderer
	{
	public:
		OpenGLRenderer();

		void ClearFrame();

	private:
		void LogOpenGLInfo();
	};
}