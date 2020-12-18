// Much needed functions
#include "types.h"
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

/*#define MAP_PTR_REVERT(ptr) ({ \
	while(*ptr != ' ') { ptr--; } \
	while(*ptr == ' ') { ptr--; } \
	while(*ptr != ' ') { ptr--; } \
}) */

#define LINE_SPLIT 6
#define LINE_SIZE 4096
#define LINE_SECTION_SIZE 1024
#define MEM_MAP "/proc/self/maps"
#define EOL '\n'

U32 get_file_length(int fd, U32 cur_file_pos);
int open_file_buffer(CCCSTR file_name, BOOL writeable, char ** buf);
void *ec_malloc(U32 bytes);
char* strers(char *base_ptr, const char *ins_ptr, U32 erase_len);
char* strins(char *base_ptr, const char *ins_ptr, const char* const new_str);
BOOL find_memory_section(const char* const pathname, pmaps map_info);
U32 line_count(const char *file_buffer);
U32 file_to_list(const char *file_buffer, char ***lnk_list);
static U32 line_length(const char *ptr);
BOOL is_number(const char let);
BOOL is_valid_file_char(const char let);

BOOL is_valid_file_char(const char let) {
    if(let >= 33 && let <= 122)
        return TRUE;

    return FALSE;
}

BOOL is_number(const char let) {
    if(let >= 48 && let <= 57)
        return TRUE;

    return FALSE;
}

static U32 line_length(const char *ptr) {
    U32 len = 0;

    while(*ptr != '\n' && *ptr != '\n') { len++; ptr++; }

    return len;
}

U32 line_count(const char *file_buffer) {
    const char *ptr = file_buffer;
    U32 count = 0;

    while((ptr = strstr(ptr, "\n")) != NULL) {
        ptr++;
        count++;
    }

    return count;
}

U32 file_to_list(const char *file_buffer, char ***lnk_list) {
    U32 total_lines = line_count(file_buffer);
    U32 line_len = 0;
    const char *ptr = file_buffer;

    // Allocate memory for ptr list
    *lnk_list = (char **)ec_malloc(sizeof(char *) * total_lines);

    for(unsigned int i = 0; i < total_lines; i++) {
        line_len = line_length(ptr);
        (*lnk_list)[i] = (char *)ec_malloc(sizeof(char) * (line_len));
        
        memcpy((*lnk_list)[i], ptr, line_len - 1);
        ptr = ptr + (line_len + 1);
    }

    return total_lines;
}

static BOOL split_line(char line_buffers[LINE_SPLIT][LINE_SECTION_SIZE], char* line_buffer) {
// Secure build sanity checks
	if(line_buffers == NULL || line_buffer == NULL)
		return FALSE;

	// Locals
	char *ptr 	= line_buffer; 	// Used to iterate and copy data from line string
	char check_char = ' ';		// char used to compare against to find end of seg

	// copy strings
	for(U32 i = 0; i < LINE_SPLIT; i++) {
		// Set check_char to detect EOL for the last string 
		if(i == (LINE_SPLIT - 1))
			check_char = EOL;

		// Copy string
		for(U32 j = 0; *ptr != check_char && j < LINE_SECTION_SIZE; j++) {
			line_buffers[i][j] = *ptr;
			ptr++;
		}

		// Make sure string is NULL terminated
		if(line_buffers[i][strlen(line_buffers[i])] == check_char)
			line_buffers[i][strlen(line_buffers[i])] = 0;	

		// Progress/exit
		while(*ptr == check_char) { 
			if(*ptr == EOL) { break; }
			ptr++;
		}	
	}

	return TRUE;
}

static S32 get_maps_line_length(const S32 fd, const U32 max_len, BOOL *is_eof) {
// Secure build sanity checks
	if(max_len == 0) { return -1; }

	if(is_eof != NULL) { *is_eof = FALSE; }

	// locals
	U32 	line_len 	= 0; // Will hold counted line length read
	char 	temp_char 	= 0; // Hold temporary character from read syscall
	S32	ret 		= 0; // Read call return capture

	do {
		// Read into temp_char
		if((ret = read(fd, &temp_char, 1)) == -1) { return -1; }
		else if(ret == 0) { 
			if(is_eof != NULL) { *is_eof = TRUE; }
			break; 
		}

		// if not eof and not error has occured...
		line_len++;
	} while(temp_char != '\n' && line_len < max_len);

	if(lseek(fd, (int)(0 - line_len), SEEK_CUR) == -1)
		return -1;

	return line_len;	
}

BOOL find_memory_section(const char* const pathname, pmaps map_info) {
	// Local - Buffers
	char line_buffer[LINE_SIZE] = {0};
	char line_buffers[LINE_SPLIT][LINE_SECTION_SIZE] = {0};
	char *address_start = NULL, *address_end = NULL;

	// Locals
	S32 	fd 	= 0;
	BOOL 	is_eof 	= FALSE;
	char 	*ptr 	= NULL;
	U32	split	= 0;

	// Open maps file
	if((fd = open(MEM_MAP, O_RDONLY)) == -1)
		return FALSE;	

	do {
		// main loop locals
		S32 line_len = 0;

		// get line length
		if((line_len = get_maps_line_length(fd, LINE_SIZE, &is_eof)) == -1) {
			close(fd);	
			return FALSE;
		}

		// if line is readable...
		if(line_len && !is_eof) {
			// Copy line data
			memset(line_buffer, 0, LINE_SIZE); // NULL buffer
			if(read(fd, line_buffer, line_len) == -1) {
				close(fd);
				return FALSE;
			}	

			// Null out line_buffers	
			for(U32 i = 0; i < LINE_SPLIT; i++)
				memset(line_buffers[i], 0, LINE_SECTION_SIZE);
	
			// Split line buffer into SPLIT_LINE strings
			if(split_line(line_buffers, line_buffer) == FALSE) {
				close(fd);
				return FALSE;
			}
		}	
	
	} while(!is_eof && strcmp(line_buffers[LINE_SPLIT - 1], pathname) != 0); 

	if(is_eof) {
		close(fd);
		return FALSE;	
	}
 
	memcpy(map_info->dev, line_buffers[3], 8);
	memcpy(map_info->pathname, line_buffers[LINE_SPLIT - 1], LINE_SECTION_SIZE);
	memcpy(map_info->perms, line_buffers[1], 8);

	ptr = line_buffers[0];
	while(*ptr != '-') ptr++;
	*ptr = 0;

	split = ptr - line_buffers[0];
	address_start = line_buffers[0];
	address_end = line_buffers[0] + (split + 1);

	map_info->start = (U64)strtol(address_start, NULL, 16);
	map_info->end = (U64)strtol(address_end, NULL, 16);

	map_info->inode = (U32)atoi(line_buffers[4]);
	map_info->offset = (U32)atoi(line_buffers[2]);

	close(fd);
	return TRUE;
}

BOOL is_heap(void *buf) {
	maps map_struct = {0};
	if(find_memory_section("[heap]", &map_struct) == FALSE)
		return FALSE;

	U64 address = (U64)buf;

	if(address >= map_struct.start && address <= map_struct.end)
		return TRUE;

	return FALSE;	
}

// Erase section of string and shrink buffer
char* strers(char *base_ptr, const char *ins_ptr, U32 erase_len) {
        char *new = NULL;
        U32 start_len = 0, end_len = 0;

        // data needed for ptr arithmatic
        start_len = ins_ptr - base_ptr;
        end_len = strlen(ins_ptr) - erase_len;

        // Allocate memory for new string
        if((new = (char *)ec_malloc(start_len + end_len + 1)) == NULL)
                return NULL;

        // Copy data
        memcpy(new, base_ptr, start_len);
        memcpy(new + start_len, base_ptr + start_len + erase_len, end_len);

        // Free old data
        free(base_ptr);

        return new;
}

// Insert string into string
char* strins(char *base_ptr, const char *ins_ptr, const char* const new_str) {
        char *new_buf = NULL;
        U32 start_len = 0, end_len = 0, new_str_len = 0, total_len = 0;

        // Calculate start and end lengths
        start_len = (ins_ptr - base_ptr) + 1;
        end_len = strlen(ins_ptr) - 1;
        new_str_len = strlen(new_str);
        total_len = start_len + end_len + new_str_len;

        // Allocate space for new buffer
        if((new_buf = (char *)ec_malloc(total_len + 1)) == NULL)
                return NULL;

        // Copy into new buffer
        memcpy(new_buf, base_ptr, start_len);
        memcpy(new_buf + start_len, new_str, new_str_len);
        memcpy(new_buf + start_len + new_str_len, base_ptr + start_len, end_len);

        // Free old data and return new buffer
        free(base_ptr);

        return new_buf;
}

U32 get_file_length(int fd, U32 cur_file_pos) {
	lseek(fd, 0, SEEK_SET);
	U32 len = lseek(fd, 0, SEEK_END);
	lseek(fd, cur_file_pos, SEEK_SET);	
	
	return len;
}

int open_file_buffer(CCCSTR file_name, BOOL writeable, char ** buf) {
	int fd = 0;
	int flag = O_RDONLY;
	U32 file_len = 0;

	// If you want file descriptor to be writeable
	if(writeable == TRUE)
		flag = O_RDWR;

	// open file
	if((fd = open(file_name, flag)) == -1)
		return -1;

	file_len = get_file_length(fd, 0);

	if((*buf = (char *)ec_malloc(file_len + 1)) == NULL)
		return -1;

	if(read(fd, *buf, file_len) == -1)
		return -1;

	return fd;
}

void *ec_malloc(U32 bytes) {
	void *ptr = NULL;
	if((ptr = malloc(bytes)) == NULL)
		return NULL;

	memset((char *)ptr, 0, bytes);

	return ptr;
}
