#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "child.h"

/* Initialize the child and the message used 
  for communication between it and its parent. */
void
init_child (struct child *child, pid_t pid)
{
  struct thread *t;
  child->pid = pid;
  t = thread_get (pid);
  init_message (&child->msg, 0);
  t->msg = &(child->msg);
  sema_up (&t->sema);
}

/* Removes child from the current thread. */
void
remove_child (struct child *child)
{
  struct thread *c = thread_get (child->pid);
  if (c != NULL)
    c->msg = NULL;
  list_remove (&child->elem);
  free (child);
}

/* Gets the child with id = pid from children list of current thread
  if found, or NULL otherwise. */
struct child *
get_child (pid_t pid)
{
  struct list_elem *e;
  struct thread *parent = thread_current ();
  for (e = list_begin (&parent->children); e != list_end (&parent->children);
       e = list_next (e))
    {
      struct child *c = list_entry (e, struct child, elem);
      if (c->pid == pid)
        return c;
    }
  return NULL;
}

/* Removes all children of the current thread. */
void
remove_children (void)
{
   struct thread *cur = thread_current ();
   struct list_elem *e = list_begin (&cur->children);
   while (e != list_end (&cur->children))
     {
       struct child *child = list_entry (e, struct child, elem);
       e = list_next (e);
       remove_child (child);
     }
}

/* Initializes the message. */
void
init_message (struct message *msg, int content)
{
  sema_init (&msg->sema, content);
}

/* Receive message if exist or waits for it . */
int
receive_message (struct message *msg)
{
  sema_down (&msg->sema);
  return msg->content;
}

/* Sends a message content to a thread of
   those waiting for a message. */
void
send_message (struct message *msg, int content)
{
  msg->content = content;
  sema_up (&msg->sema);
}
