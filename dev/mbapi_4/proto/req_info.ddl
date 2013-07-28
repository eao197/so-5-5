{type mbapi_4::proto::req_info_t
	{attr m_req_id
		{of oess_2::int_t}
	}
}

{type mbapi_4::proto::req_info_ptr_t
	{super oess_2::stdsn::shptr_t}
}

{type mbapi_4::proto::resp_info_t
	{super mbapi_4::proto::req_info_t}

	{attr m_ret_code
		{of oess_2::int_t}
	}

	{attr m_error_msg
		{of small-string}
	}
}

{type mbapi_4::proto::unknown_resp_info_t
	{super mbapi_4::proto::resp_info_t}
}

