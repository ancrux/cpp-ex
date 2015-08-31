#ifndef _TASKPOOL_H_
#define _TASKPOOL_H_

#include "ace/Task.h"
#include "ace/Synch.h"

#include "AIO_Config.h"

#if defined (ACE_HAS_THREADS) 


// *************************************************************
//  TaskPool is ACE_Task resposible for :
//  1. creation and deletion of
//     Proactor and Proactor thread pool
//  2. running Proactor event loop
// *************************************************************

/**
 * @class TaskPool
 *
 * TaskPool plays role for Proactor threads pool
 *
 * TaskPool is ACE_Task resposible for:
 * 1. Creation and deletion of Proactor/TP_Reactor thread pool
 * 2. Running Proactor/TP_Reactor event loop
 */
class TaskPool : public ACE_Task< ACE_MT_SYNCH >
{
public:
	static int disable_signal(int sigmin, int sigmax);

public:
  TaskPool (AIO_Config& cfg);

  virtual int svc (void);

  int start (void);
  int stop  (void);
  int stop_and_free (void);

  int enable_event_loop (void);
  int signal_main (void);
  int wait_signal (void);

  int should_stop (void) const { return flg_stop_;}

  AIO_Config&  config(void) {return this->cfg_;}

protected:
  virtual ~TaskPool();

  virtual int create_demultiplexor(void)=0;
  virtual int delete_demultiplexor(void)=0;
  virtual int run_event_loop(u_int thr_num)=0;
  virtual int end_event_loop(void)=0;


  AIO_Config& cfg_;
  ACE_SYNCH_RECURSIVE_MUTEX lock_;
  ACE_Thread_Semaphore sem_main_;
  ACE_Thread_Semaphore sem_work_;

  int   flg_stop_;
  u_int thr_seq_num_;
};

#endif  // (ACE_HAS_THREADS) 

#endif // _TASKPOOL_H_

