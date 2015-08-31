#ifndef _PROACTOR_TASKPOOL_H_
#define _PROACTOR_TASKPOOL_H_

#include "TaskPool.h"
#include "TProactor/Proactor.h"
#include <vector>



class Proactor_TaskPool : public TaskPool
{
public:
  Proactor_TaskPool (AIO_Config& cfg);
  virtual ~Proactor_TaskPool();

 
  TRB_Proactor * get_proactor(u_int idx);

protected:
  virtual int create_demultiplexor(void);
  virtual int delete_demultiplexor(void);
  virtual int run_event_loop(u_int thr_num);
  virtual int end_event_loop(void);

private:
  TRB_Proactor * create_proactor (u_int idx); 

  typedef std::vector<TRB_Proactor *> ProactorVector;
  
  ProactorVector proactor_vect_;
};


#endif // _PROACTOR_TASKPOOL_H_

