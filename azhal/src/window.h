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

		Bool poll() const;

		AZHAL_FORCE_INLINE void* get() const
		{
			return static_cast< void* >( m_pWindow );
		}

		Uvec2 get_framebuffer_size() const;

	private:
		GLFWwindow* m_pWindow;

		String m_name;
		Uint32 m_width;
		Uint32 m_height;
	};

	using WindowPtr = std::unique_ptr<Window>;
}