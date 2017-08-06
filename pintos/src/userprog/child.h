#ifndef USERPROG_CHILD_H
#define USERPROG_CHILD_H

#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/user/syscall.h"

/* message structure. */
struct message
{
  struct semaphore sema;
  int content;
};

/* Child structure. */
struct child
{
  pid_t pid;			        /* Child id. */
  struct message msg;   		/* Child message. */
  struct list_elem elem;
};

/* Child functions */
void init_child (struct child *child, pid_t pid);
void remove_child (struct child *child);
void remove_children (void);
struct child * get_child (pid_t pid);

/* Message functions */
void init_message (struct message *msg, int content);
int receive_message (struct message *msg);
void send_message (struct message *msg, int content);


#endif /* userprog/child.h */
