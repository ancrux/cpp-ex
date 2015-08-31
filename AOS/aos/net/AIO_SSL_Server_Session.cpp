#include "AIO_SSL_Server_Session.h"

AIO_SSL_Server_Session::AIO_SSL_Server_Session(ACE_SSL_Context* context, int start_ssl)
:
AIO_Session(),
ssl_stream_(TRB_SSL_Asynch_Stream::ST_SERVER, context),
is_cancelling_(0),
is_safe_to_delete_(0),
ssl_(start_ssl)
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) ctor(%@):AIO_SSL_Server_Session\n"), this)); //@
}

AIO_SSL_Server_Session::~AIO_SSL_Server_Session()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) dtor(%@):AIO_SSL_Server_Session\n"), this)); //@
}

void
AIO_SSL_Server_Session::cancel()
{
	if ( ssl_ )
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);
		if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
	}
	else
	{
		AIO_Session::cancel();
	}
}

int
AIO_SSL_Server_Session::is_safe_to_delete() const
{
	if ( ssl_ )
	{
		return ( this->is_safe_to_delete_ && !this->has_pending_io() );
	}
	else
	{
		return AIO_Session::is_safe_to_delete();
	}
}

void
AIO_SSL_Server_Session::open(ACE_HANDLE handle, ACE_Message_Block& mb)
{
	if ( ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			if ( this->is_cancelling_ || this->ssl_stream_.open(*this,
				handle,
				0, // completion key,
				manager_->task().get_proactor((u_int) this->index()),
				1 ) == -1 )
			{
				this->is_safe_to_delete_ = 1;
				this->on_open_error(ACE_OS::last_error());
			}
			else
			{
				this->is_safe_to_delete_ = 0;
				this->n_op_r_ = this->n_op_w_ = 0;
				this->handle_ = handle;
				if ( this->on_open(mb) < 0 )
				{
					if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
				}
			}

			if ( this->has_pending_io() ) return;
			if ( this->is_safe_to_delete_ == 0 ) { ssl_stream_.close(); return; }
			this->on_close();
		}

		manager_->destroy_session(this);
	}
	else
	{
		AIO_Session::open(handle, mb);
	}
}

void
AIO_SSL_Server_Session::start_ssl()
{
	if ( !ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			if ( this->has_pending_io() ) return;

			if ( this->ssl_stream_.open(*this,
				this->handle_,
				0, // completion key,
				manager_->task().get_proactor((u_int) this->index()),
				1 ) == -1 )
			{
				return;
			}
			else
			{
				this->ssl_ = 1;
				this->is_safe_to_delete_ = 0;
				//if ( this->on_start_ssl() < 0 )
				//{
				//	if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
				//}
			}
			//if ( this->has_pending_io() ) return;
			//if ( this->is_safe_to_delete_ == 0 ) { ssl_stream_.close(); return; }
			//this->on_close();
		}
		//manager_->destroy_session(this);
	}
}

int
AIO_SSL_Server_Session::read(ACE_Message_Block& mb)
{
	if ( ssl_ )
	{
		if ( is_cancelling_ ) return -1;
		if ( n_op_r_ != 0 ) // > 0
		{
			mb.release();
			return n_op_r_; // don't start second read, and return # of pending read
		}

		// Inititiate read
		if ( this->ssl_stream_.read(mb, mb.space()) == -1)
		{
			mb.release();
			if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
			return -1;
		}

		++n_op_r_;

		return 0;
	}
	else
	{
		return AIO_Session::read(mb);
	}
}

void
AIO_SSL_Server_Session::handle_read_stream(const TRB_Asynch_Read_Stream::Result& result)
{
	if ( ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			--n_op_r_;

			ACE_Message_Block& mb = result.message_block();

			// on read done
			if ( result.error() == 0 )
			{
				update_last_time();
				if ( result.bytes_transferred() == 0 )
				{
					mb.release(); ssl_stream_.close();
				}
				else
				{
					int rc = this->on_read_complete(mb, result);
					if ( rc < 1 ) 
					{
						mb.release();
						if ( rc < 0 && !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
					}
				}
			}
			// on read error, close session
			else
			{
				mb.release();
				if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
			}
			
			if ( this->has_pending_io() ) return;
			if ( this->is_safe_to_delete_ == 0 ) { ssl_stream_.close(); return; }
			this->on_close();
		}
		
		manager_->destroy_session(this);
	}
	else
	{
		AIO_Session::handle_read_stream(result);
	}
}

int
AIO_SSL_Server_Session::write(ACE_Message_Block& mb)
{
	if ( ssl_ )
	{
		if ( is_cancelling_ ) return -1;
		if ( n_op_w_ != 0 ) // > 0
		{
			mb.release();
			return n_op_w_; // don't start second write, and return # of pending write
		}

		size_t n_byte = mb.length();

		if ( n_byte == 0 )
		{
			mb.release();
			if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
			return -1;
		}

		// write fail
		if ( ssl_stream_.write(mb, n_byte) == -1)
		{
			mb.release();
			if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
			return -1;
		}

		++n_op_w_;

		return 0;
	}
	else
	{
		return AIO_Session::write(mb);
	}
}

void
AIO_SSL_Server_Session::handle_write_stream(const TRB_Asynch_Write_Stream::Result& result)
{
	if ( ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			--n_op_w_;

			ACE_Message_Block& mb = result.message_block();

			// on write done
			if ( result.error() == 0 && result.bytes_transferred() > 0 )
			{
				update_last_time();
				int rc = this->on_write_complete(mb, result);
				if ( rc < 1 ) 
				{
					mb.release();
					if ( rc < 0 && !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
				}
			}
			// on write error, close session
			else
			{
				mb.release();
				if ( !is_cancelling_ ) { is_cancelling_ = 1; ssl_stream_.cancel(); }
			}

			if ( this->has_pending_io() ) return;
			if ( this->is_safe_to_delete_ == 0 ) { ssl_stream_.close(); return; }
			this->on_close();
		}

		manager_->destroy_session(this);
	}
	else
	{
		AIO_Session::handle_write_stream(result);
	}
}

void
AIO_SSL_Server_Session::resume()
{
	if ( ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			if ( !is_paused() ) return;
			n_op_r_ -= 8; n_op_w_ -= 8;
			this->on_resume();

			if ( this->has_pending_io() ) return;
			if ( this->is_safe_to_delete_ == 0 ) { ssl_stream_.close(); return; }
			this->on_close();
		}

		manager_->destroy_session(this);
	}
	else
	{
		AIO_Session::resume();
	}
}

void
AIO_SSL_Server_Session::handle_wakeup()
{
	if ( ssl_ )
	{
		{
			ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

			this->is_safe_to_delete_ = 1;

			if ( !this->is_safe_to_delete() ) return;
			this->on_close();
		}

		manager_->destroy_session(this);
	}
	else
	{
		AIO_Session::handle_wakeup();
	}
}

