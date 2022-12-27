#pragma once

struct  GLFWwindow;

namespace azhal
{
	class Window
	{
	public:
		explicit Window( const AnsiChar* name, Uint32 width, Uint32 height );
		~Window();

		Window() = delete;
		Window( const Window& ) = delete;

		Bool Poll() const;

		AZHAL_INLINE void* Get() const
		{
			return static_cast< void* >( m_pWindow );
		}

	private:
		GLFWwindow* m_pWindow;

		String m_name;
		Uint32 m_width;
		Uint32 m_height;
	};

	using WindowPtr = std::unique_ptr<Window>;
}