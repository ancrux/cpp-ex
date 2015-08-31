#include "SMTP_Mailer_Domain_Queue.h"

namespace asio {

SMTP_Mailer_Domain_Queue::SMTP_Mailer_Domain_Queue(SMTP_Mailer* mailer)
:
mailer_(mailer),
n_ref_conn_(0)
{
}

SMTP_Mailer_Domain_Queue::~SMTP_Mailer_Domain_Queue()
{
	this->clear_reference_connections();
}

void
SMTP_Mailer_Domain_Queue::clear_reference_connections()
{
	if ( n_ref_conn_ > 0 && mailer_ )
	{
		ACE_GUARD(ACE_Thread_Mutex, guard, mailer_->lock());

		SMTP_Mailer::CONNECTIONS& pool = mailer_->pool();
		for(SMTP_Mailer::CONNECTIONS::iterator iter = pool.begin(); iter != pool.end();)
		{
			SMTP_Mailer_Connection* conn = iter->second;
			if ( conn->queue() == this )
			{
				delete conn;
				pool.erase(iter++);
			}
			else
				++iter;
		}
	}
}

} // namespace asio
