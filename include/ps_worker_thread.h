#ifndef PS_WORKER_THREAD_H
#define PS_WORKER_THREAD_H

#include <queue>

#include "ps_client_conn.h"
#include "ps_util.h"

#include "worker_thread.h"
#include "slash_mutex.h"
#include "env.h"

class PSWorkerThread : public pink::WorkerThread<PSClientConn> {
 public:
  PSWorkerThread(int cron_interval = 0);
  virtual ~PSWorkerThread();
  virtual void CronHandle();

  pink::PacketBuf *welcome_msg() {
    return welcome_msg_;
  }

  //int64_t ThreadClientList(std::vector<ClientInfo> *clients = NULL);
  bool ThreadClientKill(std::string ip_port = "");
  int ThreadClientNum();

  uint64_t thread_querynum() {
    slash::RWLock(&rwlock_, false);
    return thread_querynum_;
  }

  uint64_t last_sec_thread_querynum() {
    slash::RWLock(&rwlock_, false);
    return last_sec_thread_querynum_;
  }

  void PlusThreadQuerynum() {
    slash::RWLock(&rwlock_, true);
    thread_querynum_++;
  }

  void ResetLastSecQuerynum() {
    uint64_t cur_time_us = slash::NowMicros();
    slash::RWLock l(&rwlock_, true);
    last_sec_thread_querynum_ = ((thread_querynum_ - last_thread_querynum_) * 1000000 / (cur_time_us - last_time_us_+1));
    last_thread_querynum_ = thread_querynum_;
    last_time_us_ = cur_time_us;
  }

 private:
  slash::Mutex mutex_; // protect cron_task_
  std::queue<WorkerCronTask> cron_tasks_;

  pink::PacketBuf *welcome_msg_;

  uint64_t thread_querynum_;
  uint64_t last_thread_querynum_;
  uint64_t last_time_us_;
  uint64_t last_sec_thread_querynum_;

  void AddCronTask(WorkerCronTask task);
  bool FindClient(std::string ip_port);
  void ClientKill(std::string ip_port);
  void ClientKillAll();
};


#endif
