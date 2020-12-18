#ifndef __CRYPT_H
#define __CRYPT_H

extern void lock_callback(int mode, int type, char *file, int line);
extern unsigned long thread_id(void);
extern void init_locks(void);
extern void kill_locks(void);

#endif
