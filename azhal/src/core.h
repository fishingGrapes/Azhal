#pragma once

namespace azhal
{
	class Window;

	void init( const Window& window,
		Bool is_validation_layers_enabled = false, vk::DebugUtilsMessageSeverityFlagBitsEXT debug_severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose,
		Bool is_gpu_assisted_validation_enabled = false
	);

	void shutdown();

}