{type mbapi_4::proto::handshake::version_t
	{attr m_generation
		{of oess_2::uchar_t}
	}
	{attr m_branch
		{of oess_2::uchar_t}
	}
	{attr m_release
		{of oess_2::uchar_t}
	}
	{attr m_publish
		{of oess_2::uchar_t}
	}
}

{type {extensible}{opt_flag} mbapi_4::proto::handshake::compression_t
	{attr
		m_supported
		{small-stl-set}
		{of small-string}
	}
	{attr m_preferred
		{of small-string}
	}
}

{type {extensible}{opt_flag} mbapi_4::proto::handshake_info_t
	{super mbapi_4::proto::req_info_t}

	{attr m_version
		{of mbapi_4::proto::handshake::version_t}
	}
	{attr m_compression
		{of mbapi_4::proto::handshake::compression_t}
	}
	{attr m_node_uid
		{of small-string}
	}
}

{type {extensible}{opt_flag} mbapi_4::proto::handshake_resp_info_t
	{super mbapi_4::proto::resp_info_t}

	{attr m_version
		{of mbapi_4::proto::handshake::version_t}
	}
	{attr m_compression_algorithm
		{of small-string}
	}
	{attr m_node_uid
		{of small-string}
	}
}
