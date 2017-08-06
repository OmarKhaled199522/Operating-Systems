#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "lib/user/syscall.h"

/* File descriptor type. */
typedef int fd_t;
#define FD_ERROR ((fd_t) -1)

/* Uses to ensure that only one process at a time
   is executing file system code. */
struct lock file_lock;

/* Used for a thread's files. */
struct fd_entry
{
  struct file *file;      
  fd_t fd;			  /* file descriptor. */
  struct list_elem elem;
};

void fd_close (struct fd_entry *f);

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
