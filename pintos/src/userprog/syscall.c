#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"

#define SYS_CALL_NUM 13

/* System calls handlers. */
static bool halt_handler (struct intr_frame *);
static bool exit_handler (struct intr_frame *);
static bool exec_handler (struct intr_frame *);
static bool wait_handler (struct intr_frame *);
static bool create_handler (struct intr_frame *);
static bool remove_handler (struct intr_frame *);
static bool open_handler (struct intr_frame *);
static bool filesize_handler (struct intr_frame *);
static bool read_handler (struct intr_frame *);
static bool write_handler (struct intr_frame *);
static bool seek_handler (struct intr_frame *);
static bool tell_handler (struct intr_frame *);
static bool close_handler (struct intr_frame *);

static void syscall_handler (struct intr_frame *);
static void sys_exit (int status);
static bool check_word (const void *ptr, unsigned bytes);
static bool check_args (const uint32_t *args, size_t argc);
static bool check_buffer (const void *buffer);
static fd_t get_fd (void);
static struct fd_entry* get_fd_entry (fd_t file_fd);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&file_lock);
}

/* System calls handler checks the stack pointer and
   the arguments passed and handles the system call. */
static void
syscall_handler (struct intr_frame *f)
{
  int syscall_num;
  bool success = true;

  /* Checks stack pointer. */
  if (!check_word (f->esp, 4))
    sys_exit (-1);

  syscall_num = *((int *)f->esp);

  /* Checks the system call number. */
  if (syscall_num < 0 || syscall_num >= SYS_CALL_NUM)
    sys_exit (-1);

  /* excutes the system call handler. */
  switch (syscall_num)
  {
    case SYS_HALT :
      success = halt_handler (f);
      break;
    case SYS_EXIT :
      success = exit_handler (f);
      break;
    case SYS_EXEC :
      success = exec_handler (f);
      break;
    case SYS_WAIT :
      success = wait_handler (f);
      break;
    case SYS_CREATE :
      success = create_handler (f);
      break;
    case SYS_REMOVE :
      success = remove_handler (f);
      break;
    case SYS_OPEN :
      success = open_handler (f);
      break;
    case SYS_FILESIZE :
      success = filesize_handler (f);
      break;
    case SYS_READ :
      success = read_handler (f);
      break;
    case SYS_WRITE :
      success = write_handler (f);
      break;
    case SYS_SEEK :
      success = seek_handler (f);
      break;
    case SYS_TELL :
      success = tell_handler (f);
      break;
    case SYS_CLOSE :
      success = close_handler (f);
      break;
  }

  if (!success)
    sys_exit (-1);
}

/* Terminates Pintos. */
static bool
halt_handler (struct intr_frame *f UNUSED)
{
  shutdown_power_off ();
  NOT_REACHED ();
  return true;
}

/* Terminates the current user program and
   returning status to the kernel. */
static void
sys_exit (int status)
{
  thread_current ()->exit_status = status;
  thread_exit ();
}

static bool
exit_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  sys_exit (*(int *)args);
  return true;
}

/* Runs the executable whose name is given in CMDLINE and
   returns the new process’s program id (pid). */
static bool
exec_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  if (!check_buffer (*(char **)args))
    return false;
  f->eax = process_execute (*(char **)args);
  return true;
}

/* Waits for a child process pid and retrieves the
   child’s exit status. */
static bool
wait_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  f->eax = process_wait (*(int *)args);
  return true;
}

/* Creates a new file called file initially initial size bytes
   in size. Returns true if successful, false otherwise. */
static bool
create_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 2))
    return false;
  if (!check_buffer (*(char **)args))
    return false;
  bool result;
  lock_acquire (&file_lock);
  result = filesys_create (*(char **)args, *((unsigned *)(args + 1)));
  lock_release (&file_lock);
  f->eax = result;
  return true;
}

/* Deletes the file called file.
   Returns true if successful, false otherwise.*/
static bool
remove_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  bool result;
  lock_acquire (&file_lock);
  result = filesys_remove (*(char **)args);
  lock_release (&file_lock);
  f->eax = result;
  return true;
}

/* Opens the file called file. Returns its file descriptor
   or FD_ERROR if the file could not be opened.*/
static int
sys_open (const char *file_name)
{
  struct thread *cur = thread_current ();
  struct file *file;
  struct fd_entry *f;
  lock_acquire (&file_lock);
  file = filesys_open (file_name);
  lock_release (&file_lock);
  if (file == NULL)
    return FD_ERROR;
  f = (struct fd_entry *)malloc (sizeof (struct fd_entry));
  if (f == NULL)
    {
      file_close (file);
      return FD_ERROR;
    }
  f->file = file;
  f->fd = get_fd ();
  list_push_back (&cur->files, &f->elem);
  return f->fd;
}

static bool
open_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  if (!check_buffer (*(char **)args))
    return false;
  f->eax = sys_open (*(char **)args);
  return true;
}

/* Returns the size, in bytes, of the file open as fd, or -1
   (error value) if the file not found. */
static bool
filesize_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  struct fd_entry *f2;
  int result;
  f2 = get_fd_entry (*(int *)args);
  if (f2 == NULL || f2->file == NULL)
    result = -1;
  lock_acquire (&file_lock);
  result = file_length (f2->file);
  lock_release (&file_lock);
  f->eax = result;
  return true;
}

/* Reads size bytes from the file open as fd into buffer.
   Returns the number of bytes actually read, or -1
   if the file could not be read. */
static int
sys_read (int fd, void *buffer_, unsigned size)
{
  int result;
  uint8_t *buffer = (uint8_t *)buffer_;
  struct fd_entry *f;
  if (fd == STDOUT_FILENO)
    return -1;
  if (fd == STDIN_FILENO)
    {
      unsigned i;
      for (i = 0; i < size; i++)
  {
    *(buffer + i) = input_getc();
  }
      return size;
    }
  f = get_fd_entry (fd);
  if (f == NULL || f->file == NULL)
    return -1;
  lock_acquire (&file_lock);
  result = file_read (f->file, buffer, size);
  lock_release (&file_lock);
  return result;
}

static bool
read_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 3))
    return false;
  void *buffer = *(char **)(args + 1);
  uint32_t size = *(args + 2);
  if (!check_word (buffer, size))
    return false;
  f->eax = sys_read (*(int*)args, buffer, size);
  return true;
}

/* Writes size bytes from buffer to the open file fd.
   Returns the number of bytes actually written, or -1
   (error value) if it is read-only file.*/
static int
sys_write (int fd, void *buffer_, unsigned size)
{
  int result;
  uint8_t *buffer = (uint8_t *)buffer_;
  struct fd_entry *f;
  if (fd == STDIN_FILENO)
    return -1;
  if (fd == STDOUT_FILENO)
    {
      putbuf((char *)buffer, size);
      return (int)size;
    }
  f = get_fd_entry (fd);
  if (f == NULL || f->file == NULL)
    return -1;
  lock_acquire (&file_lock);
  result = file_write (f->file, buffer, size);
  lock_release (&file_lock);
  return result;
}

static bool
write_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 3))
    return false;
  void *buffer = *(char **)(args + 1);
  uint32_t size = *(args + 2);
  if (!check_word (buffer, size))
    return false;
  f->eax = sys_write (*(int*)args, buffer, size);
  return true;
}

/* Changes the next byte to be read or written in open file fd
   to position. */
static bool
seek_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  unsigned position;
  int fd;
  struct fd_entry *fn;
  if (!check_args (args, 2))
    return false;
  position = *(unsigned *)(args + 1);
  if ((int)position < 0)
    return false;
  fd = *(int *)args;
  fn = get_fd_entry (fd);
  if (fn == NULL || fn->file == NULL)
    return false;
  struct fd_entry *f2;
  f2 = get_fd_entry (fd);
  lock_acquire (&file_lock);
  file_seek (f2->file, position);
  lock_release (&file_lock);
  return true;
}

/* Returns the position of the next byte to be read or
   written in open file fd, or -1 if file not found. */
static bool
tell_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  struct fd_entry *f2;
  unsigned result;
  f2 = get_fd_entry (*(int *)args);
  if (f2 == NULL || f2->file == NULL)
    result = -1;
  lock_acquire (&file_lock);
  result = file_tell (f2->file);
  lock_release (&file_lock);
  f->eax = result;
  return true;
}

/* Closes file descriptor fd. */
static bool
close_handler (struct intr_frame *f)
{
  uint32_t *args = (uint32_t *)(f->esp + 4);
  if (!check_args (args, 1))
    return false;
  struct fd_entry *f2 = get_fd_entry (*(int *)args);
  if (f2 == NULL)
    return true;
  fd_close (f2);
  return true;
}

/* Returns a file descriptor to use for a file. */
static fd_t
get_fd (void)
{
  static fd_t next_fd = 2;
  return next_fd++;
}

/* Gets the fd entry whose fd = fd. */
static struct fd_entry*
get_fd_entry (fd_t fd)
{
  struct list_elem *e;
  struct thread *cur = thread_current ();
  for (e = list_begin (&cur->files); e != list_end (&cur->files);
       e = list_next (e))
    {
      struct fd_entry *f = list_entry (e, struct fd_entry, elem);
      if (f->fd == fd)
  	return f;
    }
  return NULL;
}

/* Gets a byte at user virtual address UADDR.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_byte (const uint8_t *uaddr)
{
  if(!is_user_vaddr(uaddr))
    return -1;
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Checks the word. */
static bool
check_word (const void *ptr, unsigned bytes)
{
  unsigned i;
  for (i = 0; i < bytes; ++i)
  {
    if (get_byte((uint8_t *)ptr + i) == -1)
      return false;
  }
  return true;
}

/* Checks all the arguments. */
static bool
check_args (const uint32_t *args, size_t argc)
{
  size_t i;
  for (i = 0; i < argc; i++)
    {
      if (!check_word (args + i, 4))
        return false;
    }
  return true;
}

/* Checks the buffer block. */
static bool
check_buffer (const void *buffer)
{
  unsigned i = 0;
  int ch = 1;
  while (ch != '\0' && ch != -1)
    {
      ch = get_byte ((uint8_t*)buffer + i);
      ++i;
    }
  if (ch == -1)
    return false;
  return true;
}

