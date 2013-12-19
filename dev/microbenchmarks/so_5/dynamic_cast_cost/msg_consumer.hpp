class msg_t
	{
	public :
		virtual ~msg_t();
	};

class concrete_message_t : public msg_t
	{};

class message_consumer_t
	{
	public :
		message_consumer_t();

		void
		handle( const msg_t & msg );

	private :
		unsigned long m_messages_received;
	};
