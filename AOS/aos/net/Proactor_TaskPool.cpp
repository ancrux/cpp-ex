#include "Proactor_TaskPool.h"


#if defined (ACE_WIN32) 

#  include "TProactor/WIN32_Proactor.h"

#else  /* defined (ACE_WIN32) */

#  include "TProactor/POSIX_Proactor.h"
#  include "TProactor/POSIX_CB_Proactor.h"
#  include "TProactor/SUN_Proactor.h"

#endif /* defined (ACE_WIN32) */

// *****************************************************************
//
// *****************************************************************

Proactor_TaskPool::Proactor_TaskPool (AIO_Config& cfg)
: TaskPool      (cfg)
, proactor_vect_()
{
}

Proactor_TaskPool::~Proactor_TaskPool (void)
{
  this->stop_and_free();
}

TRB_Proactor *
Proactor_TaskPool::get_proactor(u_int idx)
{
    size_t count = proactor_vect_.size();

    if (count == 0)
        return 0;

    idx = idx % count;

    return proactor_vect_.at(idx);
}

int
Proactor_TaskPool::run_event_loop(u_int thr_num)
{
  int rc = -1;

  TRB_Proactor * proactor = get_proactor (thr_num);

  if (proactor != 0)
    rc = proactor->proactor_run_event_loop ();
  
  return rc;
}

int
Proactor_TaskPool::end_event_loop(void)
{
  int rc = 0;

  ProactorVector::iterator itr1 = proactor_vect_.begin();
  ProactorVector::iterator itr2 = proactor_vect_.end ();
  
  for (u_int i=0; itr1 != itr2; ++i, ++itr1)
  {
      TRB_Proactor * proactor = *itr1;

      ACE_DEBUG ((LM_DEBUG,
        ACE_TEXT ("(%t) Proactor %d calling end event loop\n"),
        i));

      if (proactor && proactor->proactor_end_event_loop () < 0)
      {
          rc = -1;
      }
  }

  return rc;
}


int
Proactor_TaskPool::create_demultiplexor(void)
{
    u_int proactors = (u_int) cfg_.n_demultiplexor();
    u_int threads   = (u_int) cfg_.n_thread();

    if (threads == 0)       
        threads = 1;

    if (proactors == 0)
        proactors = 1;

    if (proactors > threads)
        proactors = threads;
    
    for (u_int i=0; i < proactors; ++i)
    {
        TRB_Proactor * proactor = create_proactor(i);
        if (!proactor)
        {
            return -1;
        }
        proactor_vect_.push_back (proactor);
    }

    return 0;
}

TRB_Proactor *
Proactor_TaskPool::create_proactor(u_int idx)
{
  const ACE_TCHAR * str_type = ACE_TEXT(" DEFAULT");


#if defined (ACE_WIN32) && !defined (ACE_HAS_WINCE)

  TRB_WIN32_Proactor * proactor_impl = 0;

  str_type = ACE_TEXT(" WIN32");

  ACE_NEW_RETURN (proactor_impl,
                  TRB_WIN32_Proactor (this->cfg_.n_thread()),
                  0);

  
  
#elif defined (ACE_HAS_AIO_CALLS) || defined (ACE_HAS_AIO_EMULATION)

	AIO_Type aio_type = this->cfg_.aio_type();
	size_t max_op = this->cfg_.max_aio_op();
	int sig_num = this->cfg_.signal_n();
	int leader_flg = this->cfg_.leader_type();
	//int start_aio_flg = this->cfg_.start_aio_type();

  if (idx > 0 && aio_type == SUN) 
  {
      // aiowait() allows only single SUN_Proactor
      aio_type = POLL;
  }

  TRB_POSIX_Proactor * proactor_impl = 0;

  switch (aio_type)
    {
    case AIOCB:
      str_type = ACE_TEXT(" AIOCB");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_POSIX_AIOCB_Proactor (max_op,
                                                sig_num,
                                                leader_flg),
                      0);
      break;

    case SIG:
      str_type = ACE_TEXT(" SIG");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_POSIX_SIG_Proactor (max_op,
                                              sig_num,
                                              leader_flg),
                      0);
      break;

    case SUN:
      str_type = ACE_TEXT(" SUN");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_SUN_Proactor (max_op,
                                        sig_num,
                                        leader_flg),
                      0);
      break;
      
    case CB:
      str_type = ACE_TEXT(" CB-SGI");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_POSIX_CB_Proactor (max_op,
                                             sig_num,
                                             leader_flg),
                      0);
      break;

    case SELECT:
      str_type = ACE_TEXT(" SELECT");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_Select_Proactor (max_op,
                                          sig_num,
                                          leader_flg),
                      0);
      break;

    case POLL:
      str_type = ACE_TEXT(" POLL");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_Poll_Proactor (max_op,
                                          sig_num,
                                          leader_flg),
                      0);
      break;


    case EPOLL:
      str_type = ACE_TEXT(" EPOLL");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_Event_Poll_Proactor (max_op,
                                          sig_num,
                                          leader_flg),
                      0);
      break;
    case DEVPOLL:
      str_type = ACE_TEXT(" DEVPOLL");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_Dev_Poll_Proactor (max_op,
                                          sig_num,
                                          leader_flg),
                      0);
      break;


    //case LINUX_NAIO:
    //  str_type = ACE_TEXT(" LINUX_NAIO");
    //  ACE_NEW_RETURN (proactor_impl,
    //                  TRB_LINUX_Proactor (max_op,
    //                                      sig_num,
    //                                      leader_flg),
    //                  0);
    //  break;
      
    case SUNPORT:
      str_type = ACE_TEXT(" SUNPORT");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_SUN_Port_Proactor (max_op,
                                          sig_num,
                                          leader_flg),
                      0);
      break;

    case DUMMY:
      str_type = ACE_TEXT(" DUMMY");
      ACE_NEW_RETURN (proactor_impl,
                      TRB_Dummy_Proactor (),
                      0);
    default:
      break;
  }

#endif // (ACE_WIN32) && !defined (ACE_HAS_WINCE)

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT("(%t) Create Proactor %d Type=%s\n"),
              idx,
              str_type));

  // always delete implementation  1 , not  !(proactor_impl == 0)

  TRB_Proactor * proactor = 0;
  ACE_NEW_RETURN (proactor,
                  TRB_Proactor (proactor_impl, 1 ), 
                  0);
  
  return proactor;
}

int
Proactor_TaskPool::delete_demultiplexor (void)
{
  ProactorVector::iterator itr1 = proactor_vect_.begin();
  ProactorVector::iterator itr2 = proactor_vect_.end ();
  
  for (u_int i=0; itr1 != itr2; ++i, ++itr1)
  {
      TRB_Proactor * proactor = *itr1;

      delete proactor;

      ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("(%t) Delete Proactor %d\n"),
              i));

      *itr1=0;
  }

  return 0;
}


