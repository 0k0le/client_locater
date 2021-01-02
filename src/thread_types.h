#ifndef __THREAD_TYPES_H
#define __THREAD_TYPES_H

#include "types.h"

#define LINKADDITION "/d/computer-gigs/search/cpg"

// Html search references
#define CLASSINDEX  "result-info"
#define HREFINDEX   "href=\""
#define DATEINDEX   "datetime=\""

#define BUFLEN 4096

typedef struct thread_data {
    char    **link_list;
    char    **date_list;
    char    **final_link_list;
    U32     total_links;
    U32     total_processed_links;

} THREADDATA, *pTHREADDATA;

#endif
