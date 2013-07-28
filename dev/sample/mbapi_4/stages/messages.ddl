{type sample_mbapi_4::ping_t
	{attr m_timestamp
		{of std::string}
	}
	{attr m_num
		{of oess_2::uint_t}
	}

	{attr m_stages_passed
		{stl-vector}
		{of std::string}
	}
}

{type sample_mbapi_4::pong_t
	{attr m_timestamp
		{of std::string}
	}
	{attr m_num
		{of oess_2::uint_t}
	}
	{attr m_stages_passed
		{stl-vector}
		{of std::string}
	}
}
