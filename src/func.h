#ifndef __FUNC_H
#define __FUNC_H

// Much needed functions
#include "types.h"

extern U32 get_file_length(int fd, U32 cur_file_pos);
extern int open_file_buffer(CCCSTR file_name, BOOL writeable, char ** buf);
extern void *ec_malloc(U32 bytes);
extern char* strers(char *base_ptr, const char *ins_ptr, U32 erase_len);
extern char* strins(char *base_ptr, const char *ins_ptr, const char* const new_str);
extern BOOL find_memory_section(const char* const pathname, pmaps map_info);
extern BOOL is_heap(void *buf);
extern U32 file_to_list(const char *file_buffer, char ***lnk_list);
extern U32 line_count(const char *file_buffer);
extern BOOL is_number(const char let);
extern BOOL is_valid_file_char(const char let);

#endif
