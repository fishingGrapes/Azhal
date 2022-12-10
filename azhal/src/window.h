#pragma once

struct  GLFWwindow;

namespace azhal
{
	class Window
	{
	public:
		explicit Window( const char* name, Uint32 width, Uint32 height );
		~Window();

		Window() = delete;
		Window( const Window& ) = delete;

		Bool Poll() const;
	private:
		GLFWwindow* m_pWindow;

		std::string m_name;
		Uint32 m_width;
		Uint32 m_height;
	};

	using WindowPtr = std::unique_ptr<Window>;
}