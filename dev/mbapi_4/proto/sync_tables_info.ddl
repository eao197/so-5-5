{type {extensible}{opt_flag} mbapi_4::proto::endpoint_info_t
	{attr m_name
		{of small-string}
	}
	{attr m_node_uid
		{of small-string}
	}
	{attr m_distance
		{of oess_2::uint_t}
	}
	{attr m_endpoint_stage_chain
		{small-stl-vector}
		{of std::string}
	}
}

{type mbapi_4::proto::endpoint_info_ptr_t
	{super oess_2::stdsn::shptr_t}
}

{type {extensible}{opt_flag} mbapi_4::proto::stagepoint_info_t
	{attr m_name
		{of small-string}
	}
	{attr m_endpoint_name
		{of small-string}
	}
	{attr m_node_uid
		{of small-string}
	}
	{attr m_distance
		{of oess_2::uint_t}
	}
}

{type mbapi_4::proto::stagepoint_info_ptr_t
	{super oess_2::stdsn::shptr_t}
}

{type {extensible}{opt_flag} mbapi_4::proto::sync_tables_info_t
	{super mbapi_4::proto::req_info_t}

	{attr m_endpoints
		{small-stl-vector}
		{of mbapi_4::proto::endpoint_info_ptr_t}
	}

	{attr m_stagepoints
		{small-stl-vector}
		{of mbapi_4::proto::stagepoint_info_ptr_t}
	}
}

{type mbapi_4::proto::sync_tables_resp_info_t
	{super mbapi_4::proto::resp_info_t}
}
