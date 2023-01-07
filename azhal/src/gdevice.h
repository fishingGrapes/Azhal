#pragma once

namespace gdevice
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct GDeviceInitParams
	{
		void* pWindow = nullptr;
		vk::Extent2D swapchainExtent;

		Bool areValidationLayersEnabled = false;
		vk::DebugUtilsMessageSeverityFlagBitsEXT debugMessageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;

		Bool isGpuAssistedValidationEnabled = false;
	};

	void init( const GDeviceInitParams& gdevice_init_params );
	void update();
	void shutdown();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

}