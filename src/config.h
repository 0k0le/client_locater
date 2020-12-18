#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIGFILE  "configfile="
#define THREADCOUNT ":threadcount="
#define OUTPUTFILE  ":outputfile="
#define LNKLISTFILE ":lnklistfile="

typedef struct config_data {
    unsigned int thread_count;
    char *output_file;
    char *lnk_list_file;

} CONFIGDATA, *pCONFIGDATA;

#endif
