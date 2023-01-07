#include "azpch.h"
#include "window.h"

#include <GLFW/glfw3.h>


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


Bool Window::poll() const
{
	if( !glfwWindowShouldClose( m_pWindow ) )
	{
		glfwPollEvents();
		return true;
	}

	return false;
}


Uvec2 Window::get_framebuffer_size() const
{
	Int32 width, height;
	glfwGetFramebufferSize( m_pWindow, &width, &height );

	return { width, height };
}