#ifndef __TYPES_H
#define __TYPES_H

// Macro types...

#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define GIANT   unsigned long
#define DGIANT  unsigned long long

#define U8      BYTE
#define U16     WORD
#define U32     DWORD
#define U64     GIANT
#define U128    DGIANT

#define S8      char
#define S16     short
#define S32     int
#define S64     long
#define S128    long long

#define CSTR    char *
#define CCSTR   const char *
#define CCCSTR  const char* const

#define BOOL    char
#define TRUE    0x1
#define FALSE   0x0

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct MAPS {
	U64 start, end;
	U32 offset, inode;
	char perms[8], dev[8], pathname[1024];
} maps, *pmaps;

#endif
