// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
  nacquire (0)
{
  lock_record.clear();
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, lock_protocol::status &r)
{
  pthread_mutex_lock(&mutex);
  while(true) {
  	std::map<lock_protocol::lockid_t, int>::iterator it = lock_record.find(lid);
  	if(it == lock_record.end()) {
  	  lock_record[lid] = clt;
  	  break;
  	}
  	pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);
  r = lock_protocol::OK;
  return lock_protocol::OK;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, lock_protocol::status &r)
{
  pthread_mutex_lock(&mutex);
  std::map<lock_protocol::lockid_t, int>::iterator it  = lock_record.find(lid);
  if(it == lock_record.end() || lock_record[lid] != clt) {
  	pthread_mutex_unlock(&mutex);
  	r = lock_protocol::NOENT;
  	return lock_protocol::NOENT;
  }
  lock_record.erase(it);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);
  r = lock_protocol::OK;
  return lock_protocol::OK;
}