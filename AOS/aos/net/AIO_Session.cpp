#include "AIO_Session.h"
#include "AIO_Acceptor.h"

AIO_Session::AIO_Session()
:
lock_(),
n_op_r_(-1),
n_op_w_(-1),
handle_(ACE_INVALID_HANDLE),
ssl_(0),
index_(-1),
manager_(0),
last_op_(ACE_OS::gettimeofday())
{
	//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) ctor(%@):AIO_Session\n"), this)); //@
}

AIO_Session::~AIO_Session()
{
	//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) dtor(%@):AIO_Session\n"), this)); //@
	delete ssl_;
}

int
AIO_Session::is_safe_to_delete() const
{
	return !this->has_pending_io();
}

void
AIO_Session::cancel()
{
	ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);
	stream_.cancel();
	stream_.close();

}

void
AIO_Session::addresses(const ACE_INET_Addr& peer, const ACE_INET_Addr& local)
{
	//ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);
	this->peer_ = peer;
	/*
	ACE_TCHAR str_peer[256];
	ACE_TCHAR str_local[256];

	if ( 0 != peer.addr_to_string(str_peer, sizeof (str_peer)/sizeof (ACE_TCHAR)) )
		ACE_OS::strcpy(str_peer, ACE_TEXT("Unknown"));

	if ( 0 != local.addr_to_string(str_local, sizeof (str_local)/sizeof (ACE_TCHAR)) )
		ACE_OS::strcpy(str_local, ACE_TEXT("Unknown"));

	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) Remote=%s Local=%s (this: %@)\n"), str_peer, str_local, this));

	return;
	*/
}

void
AIO_Session::open(ACE_HANDLE handle, ACE_Message_Block& mb)
{
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

		// int i_proactor = manager_->n_session(); // this causes deadlock. (circular wait).
		// because acceptor.handle_time_out() lock acceptor's mutex and 
		// wait for session's mutex when calling session.check_timeout().
		// at the same time, session.open() lock session's mutex and wait
		// for acceptor's mutex() when calling acceptor.n_session()!!
		if ( this->stream_.open(*this,
			handle,
			0, // completion key,
			manager_->task().get_proactor((u_int) this->index()),
			1 ) == -1 )
		{
			this->on_open_error(ACE_OS::last_error());
		}
		else
		{
			this->n_op_r_ = this->n_op_w_ = 0;
			this->handle_ = handle;
			this->on_open(mb);
		}

		if ( !this->is_safe_to_delete() ) return;
		this->on_close();
	}

	manager_->destroy_session(this);
}

int
AIO_Session::read(ACE_Message_Block& mb)
{
	if ( n_op_r_ != 0 ) // > 0
	{
		mb.release();
		return n_op_r_; // return # of pending read
	}

	// Inititiate read
	if ( this->stream_.read(mb, mb.space()) == -1)
	{
		mb.release();
		stream_.cancel(); stream_.close();
		return -1;
	}

	++n_op_r_;

	return 0;
}

void
AIO_Session::handle_read_stream(const TRB_Asynch_Read_Stream::Result& result)
{
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);
		//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session handle_read(this: %@)\n"), this));//@

		--n_op_r_;

		ACE_Message_Block& mb = result.message_block();

		// on read done
		if ( result.error() == 0 )
		{
			update_last_time();
			if ( result.bytes_transferred() == 0 )
			{
				mb.release();
			}
			else
			{
				int rc = this->on_read_complete(mb, result);
				if ( rc < 1 ) mb.release();
			}
		}
		// on read error, close session
		else
		{
			mb.release();
			stream_.cancel(); stream_.close();
		}

		if ( !this->is_safe_to_delete() ) return;
		this->on_close();
	}
	
	manager_->destroy_session(this);
}

int
AIO_Session::write(ACE_Message_Block& mb)
{
	if ( n_op_w_ != 0 ) // > 0
	{
		mb.release();
		return n_op_w_; // return # of pending write
	}

	size_t n_byte = mb.length();

	if ( n_byte == 0 )
	{
		mb.release();
		return -1;
	}

	// write fail
	if ( stream_.write(mb, n_byte) == -1)
	{
		mb.release();
		stream_.cancel(); stream_.close();
		return -1;
	}

	++n_op_w_;

	return 0;
}

void
AIO_Session::handle_write_stream(const TRB_Asynch_Write_Stream::Result& result)
{
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);
		//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session handle_write(this: %@)\n"), this));//@

		--n_op_w_;

		ACE_Message_Block& mb = result.message_block();

		// on write done
		if ( result.error() == 0 && result.bytes_transferred() > 0 )
		{
			update_last_time();
			int rc = this->on_write_complete(mb, result);
			if ( rc < 1 ) mb.release();
		}
		// on write error, close session
		else
		{
			mb.release();
			stream_.cancel(); stream_.close();
		}

		if ( !this->is_safe_to_delete() ) return;
		this->on_close();
	}

	manager_->destroy_session(this);
}

void
AIO_Session::resume()
{
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_);

		if ( !is_paused() ) return;
		n_op_r_ -= 8; n_op_w_ -= 8;
		this->on_resume();

		if ( !this->is_safe_to_delete() ) return;
		this->on_close();
	}

	manager_->destroy_session(this);
}

void
AIO_Session::handle_wakeup()
{
}

void
AIO_Session::check_timeout()
{
	//ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session check_timeout(this=%@,r=%d,w=%d)\n"),
	//			this,
	//			this->n_op_r_,
	//			this->n_op_w_
	//			)); //@
	//if ( !has_pending_io() ) return; //if ( n_op_r_ < 0 || n_op_w_ < 0 ) return;
	{
		ACE_GUARD(ACE_SYNCH_MUTEX, monitor, this->lock_); //? remove this?

		if ( this->timeout_ == ACE_Time_Value::zero ) return;

		ACE_Time_Value diff_time = ACE_OS::gettimeofday() - this->last_op_;
		if ( diff_time < this->timeout_ ) return;
	}
	this->on_timeout();
}

int
AIO_Session::on_read_complete(ACE_Message_Block& mb, const TRB_Asynch_Read_Stream::Result& result)
{
	return 0;
}

int
AIO_Session::on_write_complete(ACE_Message_Block& mb, const TRB_Asynch_Write_Stream::Result& result)
{
	return 0;
}

int
AIO_Session::on_open(ACE_Message_Block& mb_open)
{
	return 0;
}

void
AIO_Session::on_close()
{
}

void
AIO_Session::on_timeout()
{
	ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%t) AIO_Session cancel(this: %@)\n"), this));//@
	this->cancel();
}

void
AIO_Session::on_resume()
{
}

void
AIO_Session::on_pause()
{
}

void
AIO_Session::on_open_error(int err_no)
{
	ACE_ERROR((LM_ERROR,
	ACE_TEXT ("%%s p\n"),
	"this", // this
	ACE_TEXT ("TRB_SSL_Asynch_Stream::open")));
}
