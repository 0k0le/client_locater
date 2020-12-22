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

void rip_craigslist_data(char *html_page, pTHREADDATA thread_data) {
    register const char *ptr = html_page;

    while((ptr = strstr(ptr, CLASSINDEX)) != NULL) {
        if((ptr = strstr(ptr, DATEINDEX)) == NULL)
            break;
        
        U32 buf_len_count = 0;

        if(thread_data->date_list == NULL)
            thread_data->date_list = (char **)ec_malloc(sizeof(char *));
        else
            thread_data->date_list = (char **)realloc(thread_data->date_list, (thread_data->total_processed_links + 1) * sizeof(char *));

        ptr += strlen(DATEINDEX);

        // Calculate date buffer length
        for(; ptr[buf_len_count] != '\"'; buf_len_count++);

        (thread_data->date_list)[thread_data->total_processed_links] = (char *)ec_malloc(sizeof(char) * (buf_len_count + 1));
        memcpy((thread_data->date_list)[thread_data->total_processed_links], ptr, buf_len_count);

        buf_len_count = 0;

        if(thread_data->final_link_list == NULL)
            thread_data->final_link_list = (char **)ec_malloc(sizeof(char *));
        else
            thread_data->final_link_list = (char **)realloc(thread_data->final_link_list, (thread_data->total_processed_links + 1) * sizeof(char *));

        if((ptr = strstr(ptr, HREFINDEX)) == NULL) {
            thread_data->total_processed_links++;
            break;
        }

        ptr += strlen(HREFINDEX);

        for(; ptr[buf_len_count] != '\"' && ptr[buf_len_count]; buf_len_count++);

        (thread_data->final_link_list)[thread_data->total_processed_links] = (char *)ec_malloc(sizeof(char) * (buf_len_count + 1));
        memcpy((thread_data->final_link_list)[thread_data->total_processed_links], ptr, buf_len_count);

        thread_data->total_processed_links++;
    }
}

ssize_t curlwrite(void *ptr, size_t size, size_t nmemb, char **str) {
    register size_t old_len = strlen(*str);
    register size_t new_len = old_len + (size*nmemb);
    
    *str = (char *)realloc(*str, new_len + 1);
    if(!(*str))
        ERRQ("Realloc");

    memcpy((*str) + old_len, ptr, size * nmemb);

    (*str)[new_len] = 0;

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
 
        char *str = (char *)ec_malloc(1);

        curl_easy_setopt(curl, CURLOPT_URL, thread_data->link_list[i]);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str); 

        DEBUG("CURLING %s", thread_data->link_list[i]);

        if((res = curl_easy_perform(curl)) != CURLE_OK)
            ERRQ("Failed to perform curl");    
       
        rip_craigslist_data(str, thread_data);

        free(str);
    }

    curl_easy_cleanup(curl);
    
    pthread_exit(NULL); 
}

#endif
