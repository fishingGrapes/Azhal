#pragma once

namespace azhal
{
	using QueueFamily = Uint32;
	struct GpuQueue
	{
	public:
		GpuQueue()
			: m_queue( VK_NULL_HANDLE )
			, m_family( UINT32_MAX )
		{
		}

		GpuQueue( vk::Queue queue, Uint32 queue_family )
			: m_queue( queue )
			, m_family( queue_family )
		{
		}

		~GpuQueue()
		{
			m_queue = VK_NULL_HANDLE;
			m_family = UINT32_MAX;
		}

		AZHAL_INLINE consteval operator vk::Queue() const
		{
			return m_queue;
		}

		AZHAL_INLINE QueueFamily family() const
		{
			return m_family;
		}

	private:
		vk::Queue m_queue;
		QueueFamily m_family;
	};
}