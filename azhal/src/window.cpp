#include "azpch.h"
#include "window.h"

#include <GLFW/glfw3.h>

namespace azhal
{
	Window::Window( const AnsiChar* name, Uint32 width, Uint32 height )
		: m_pWindow( nullptr )
		, m_name( name )
		, m_width( width )
		, m_height( height )
	{
		Int32 result = glfwInit();
		AZHAL_FATAL_ASSERT( result, "Failed to initialize GLFW" );

		glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
		// TODO: handle resizing
		glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

		m_pWindow = glfwCreateWindow( m_width, m_height, m_name.data(), nullptr, nullptr );
		AZHAL_FATAL_ASSERT( m_pWindow != nullptr, "Failed to create GLFWwindow" );
	}

	Window::~Window()
	{
		glfwDestroyWindow( m_pWindow );
		glfwTerminate();
	}

	/// <summary>
	/// Polls for input events
	/// </summary>
	/// <returns>whether the window is open or not</returns>
	Bool Window::Poll() const
	{
		if( !glfwWindowShouldClose( m_pWindow ) )
		{
			glfwPollEvents();
			return true;
		}

		return false;
	}
}