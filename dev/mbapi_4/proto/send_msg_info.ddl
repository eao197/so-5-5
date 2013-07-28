{type {extensible}{opt_flag} mbapi_4::proto::send_msg_info_t
	{super mbapi_4::proto::req_info_t}

	{attr m_from
		{of small-string}
	}
	{attr m_to
		{of small-string}
	}
	{attr m_current_stage
		{of small-string}
	}
	{attr m_stagpoint_owner
		{of oess_2::uchar_t}
	}
	{attr m_oess_id
		{of small-string}
	}
	{attr m_payload
		{of small-string}
	}
}

{type mbapi_4::proto::send_msg_resp_info_t
	{super mbapi_4::proto::resp_info_t}
}
