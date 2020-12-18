#ifndef __THREADS_H
#define __THREADS_H

#include "types.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>
#include "func.h"

#define LINKADDITION "/d/computer-gigs/search/cpg"

typedef struct thread_data {
    char    **link_list;
    U32     total_links;

} THREADDATA, *pTHREADDATA;

typedef struct curlstring {
    char *str;
} CURLSTRING, *pCURLSTRING;

ssize_t curlwrite(void *ptr, size_t size, size_t nmemb, pCURLSTRING str) {
    register size_t old_len = strlen(str->str);
    register size_t new_len = old_len + (size*nmemb);
    
    str->str = (char *)realloc(str->str, new_len + 1);
    if(!str->str)
        ERRQ("Realloc");

    memcpy(str->str + old_len, ptr, size * nmemb);

    str->str[new_len] = 0;

    return size*nmemb;
}

static void *thread_curl(void *data) {
    pTHREADDATA thread_data = (pTHREADDATA)data;
    CURLcode res;
    register U32 total_lnk = thread_data->total_links;

    DEBUG("Thread Launched --> Total Scans: %d", thread_data->total_links);

    CURL *curl = curl_easy_init();
    if(curl == NULL)
        ERRQ("curl_easy_init()");

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlwrite);

    for(register U32 i = 0; i < total_lnk; i++) {
        if(curl == NULL)
            ERRQ("Failed to init curl!");
 
        CURLSTRING cstr = {0};
        cstr.str = (char *)ec_malloc(1);

        curl_easy_setopt(curl, CURLOPT_URL, thread_data->link_list[i]);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cstr); 

        DEBUG("CURLING %s", thread_data->link_list[i]);

        if((res = curl_easy_perform(curl)) != CURLE_OK)
            ERRQ("Failed to perform curl");    
       
        // Data processing function here...

        puts(cstr.str);

        free(cstr.str);
    }

    curl_easy_cleanup(curl);
    
    pthread_exit(NULL); 
}

#endif
