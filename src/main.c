// client_locater - Client gathering toolkit
// Updates added upon recommendation
// Currently only supports c/c++

// Matthew Todd Geiger
// Jun 24 05:30

// Standard header
#include "stdafx.h"

// Crypto
#include "crypt.h"

// Config
#include "config.h"

// Curl
#include "curl/curl.h"

// STD libs
#include <strings.h>

#include <pthread.h>

#include "thread_types.h"
#include "analyze.h"

// Peel information from craigslist web scrape
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

// Curl write callback
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

// Curl thread main
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

        if((res = curl_easy_perform(curl)) != CURLE_OK) {
            ERRQ("Failed to perform curl: %s", curl_easy_strerror(res));
        }
       
        rip_craigslist_data(str, thread_data);

        free(str);
    }

    curl_easy_cleanup(curl);

    // Call sort here
    sort_dates_thread(thread_data); 

    pthread_exit(NULL); 
}

// Unload link list
void unload_lnk_file(char **lnk_list, U32 line_count) {
    if(line_count == 0)
        ERRQ("Invalid line count number");

    for(U32 i = 0; i < line_count; i++)
        free(lnk_list[i]);

    free(lnk_list);
}

// Load link file into link list
U32 load_lnk_file(const char *const lnk_file, char ***lnk_list) {
    int fd = 0;
    char *lnk_file_buffer = NULL;
    U32 line_count = 0;

    if((fd = open_file_buffer(lnk_file, FALSE, &lnk_file_buffer)) == -1)
        ERRQ("Failed to load file: %s", lnk_file);

    close(fd);

    if((line_count = file_to_list(lnk_file_buffer, lnk_list)) == FALSE)
        ERRQ("Failed to convert file: %s into list", lnk_file);
    DEBUG("File converted to list");

    free(lnk_file_buffer);

    return line_count;
}

// Load config file
BOOL load_config_file(const char *const config_file_arg, pCONFIGDATA config_data) {
    char        *config_file_buffer = NULL;
    char        *temp_buffer        = NULL;
    const char  *config_file        = config_file_arg + strlen(CONFIGFILE);
    const char  *ptr                = NULL;
    int         fd                  = 0;

    if((fd = open_file_buffer(config_file, FALSE, &config_file_buffer)) == -1)
        ERRQ("Failed to open file: %s", config_file);

    close(fd);

    // Find thread count
    if((ptr = strstr(config_file_buffer, THREADCOUNT)) != NULL) {
        ptr = ptr + strlen(THREADCOUNT);

        U32 buffer_size = 0;
        for(; is_number(ptr[buffer_size]); buffer_size++);

        temp_buffer = (char *)ec_malloc(sizeof(char) * (buffer_size + 1));
        memcpy(temp_buffer, ptr, buffer_size);

        if((config_data->thread_count = strtoul(temp_buffer, NULL, 0)) < 1)
            ERRQ("Invalid thread count: %d", config_data->thread_count);

        free(temp_buffer);
    }

    if((ptr = strstr(config_file_buffer, OUTPUTFILE)) != NULL) {
        ptr = ptr + strlen(OUTPUTFILE);

        U32 buffer_size = 0;
        for(; is_valid_file_char(ptr[buffer_size]); buffer_size++);

        config_data->output_file = (char *)ec_malloc(sizeof(char) * (buffer_size + 1));
        memcpy(config_data->output_file, ptr, buffer_size);
    }


    if((ptr = strstr(config_file_buffer, LNKLISTFILE)) != NULL) {
        ptr = ptr + strlen(LNKLISTFILE);

        U32 buffer_size = 0;
        for(; is_valid_file_char(ptr[buffer_size]); buffer_size++);

        config_data->lnk_list_file = (char *)ec_malloc(sizeof(char) * (buffer_size + 1));
        memcpy(config_data->lnk_list_file, ptr, buffer_size);
    }

    free(config_file_buffer);

    return TRUE;
}

// Unload config file
void unload_config_data(pCONFIGDATA config_data) {
    free(config_data->lnk_list_file);
    free(config_data->output_file);
}

// Function  that divides up link list for curl threads
char ***prepare_threads(pCONFIGDATA config_data, U32 total_links, char **lnk_list) {
    U32 divisor = (total_links / config_data->thread_count);
    char ***lnk_lists = NULL;

    // Organize links
    lnk_lists = (char ***)ec_malloc(sizeof(char **) * config_data->thread_count);

    for(U32 i = 0; i < config_data->thread_count - 1; i++) {
        lnk_lists[i] = (char **)ec_malloc(sizeof(char *) * divisor);
        for(U32 j = 0; j < divisor; j++) {
           lnk_lists[i][j] = (char *)ec_malloc(sizeof(char) * ((strlen(lnk_list[(i * divisor) + j])
                + strlen(LINKADDITION)) + 1));
           strcpy(lnk_lists[i][j], lnk_list[(i * divisor) + j]);
           strcat(lnk_lists[i][j], LINKADDITION);
        }
    }
    
    {
        register U32 i = config_data->thread_count - 1;
        
        lnk_lists[i] = (char **)ec_malloc(sizeof(char *) * (divisor + (total_links %
            config_data->thread_count)));

        for(U32 j = 0; j < divisor + (total_links % config_data->thread_count); j++) {
            lnk_lists[i][j] = (char *)ec_malloc(sizeof(char) * ((strlen(lnk_list[(i * divisor) + j])
                + strlen(LINKADDITION)) + 1));
            strcpy(lnk_lists[i][j], lnk_list[(i * divisor) + j]);
            strcat(lnk_lists[i][j], LINKADDITION);
        }
                
    }

    return lnk_lists;
}

// Cleanup curl thread link list
void cleanup_prethread_data(pCONFIGDATA config_data, U32 total_links, char ***lnk_lists) {
    register U32 divisor = total_links / config_data->thread_count;
    
    for(U32 i = 0; i < config_data->thread_count - 1; i++) {
        for(U32 j = 0; j < divisor; j++)
            free(lnk_lists[i][j]);

        free(lnk_lists[i]);
    }

    for(U32 i = 0; i < divisor + (total_links % config_data->thread_count); i++) {
        free(lnk_lists[config_data->thread_count - 1][i]);
    }

    free(lnk_lists[config_data->thread_count - 1]);
    free(lnk_lists);
}

int main(const int argc, char** argv, const char** const envp) {   
    UNUSED(envp);
   
    puts("Initalizing...");

    // locals
    BOOL            config_file     = FALSE;
    CONFIGDATA      config_data     = { 0 };
    char            **lnk_list      = NULL;
    U32             total_links     = 0;
    char            ***lnk_lists    = NULL;
    pthread_t       *tids           = NULL;
    pTHREADDATA     thread_data     = NULL;

    // Scan for config
    for(int i = 1; i < argc; i++) {
         if(strstr(argv[i], CONFIGFILE) != NULL) {
            if(load_config_file(argv[i], &config_data) == FALSE)
                ERRQ("Failed to load config file: %s", argv[i]);

            config_file = TRUE;
            DEBUG("Config file loaded");
         } 
    }

    DEBUG("Thread Count: %d", config_data.thread_count);
    DEBUG("Output File: %s", config_data.output_file);
    DEBUG("Link List File: %s", config_data.lnk_list_file);

    // Load link file
    if((total_links = load_lnk_file(config_data.lnk_list_file, &lnk_list)) < 1)
        ERRQ("Failed to load link file!");

    DEBUG("Link file loaded");

    // Init crypto
    init_locks();
    DEBUG("Crypto locks initialized");

    // Init curl
    curl_global_init(CURL_GLOBAL_ALL);
    DEBUG("Curl initialized");

    // Initialize thread memory
    lnk_lists = prepare_threads(&config_data, total_links, lnk_list);
    DEBUG("Threads prepared"); 

    // Cleanup link file
    unload_lnk_file(lnk_list, total_links);
    DEBUG("Link file unloaded");

#ifdef _DEBUG
    register U32 divisor = total_links / config_data.thread_count;

    for(U32 i = 0; i < config_data.thread_count - 1; i++) {
        DEBUG("Thread %d", i);

        for(U32 j = 0; j < divisor; j++)
            DEBUG("%s", lnk_lists[i][j]);

    }

    {
        register U32 i = config_data.thread_count - 1;

        for(U32 j = 0; j < divisor + (total_links % config_data.thread_count); j++)
            DEBUG("%s", lnk_lists[i][j]);
    }
#endif

    puts("Spawning Threads...");

    // Spawn threads
    tids = (pthread_t *)ec_malloc(sizeof(pthread_t) * config_data.thread_count);
    thread_data = (pTHREADDATA)ec_malloc(sizeof(THREADDATA) * config_data.thread_count);

    for(U32 i = 0; i < config_data.thread_count - 1; i++) {
        thread_data[i].link_list = lnk_lists[i];
        thread_data[i].total_links = total_links / config_data.thread_count;
    }

    thread_data[config_data.thread_count - 1].link_list = lnk_lists[config_data.thread_count - 1];
    thread_data[config_data.thread_count - 1].total_links = (total_links / config_data.thread_count) +
        (total_links % config_data.thread_count);

    for(U32 i = 0; i < config_data.thread_count; i++) {
        if(pthread_create(tids + i, NULL, thread_curl, (void *)(thread_data + i)) != 0)
            ERRQ("Failed to spawn thread!");
    }

    puts("Threads Spawned... This may take a couple minutes...");

    for(U32 i = 0; i < config_data.thread_count; i++) {
        pthread_join(tids[i], NULL);
    }

    puts("Threads completed");

    free(tids);

    cleanup_prethread_data(&config_data, total_links, lnk_lists);
    DEBUG("Threads cleaned up...");

    THREADDATA final_sort_data = {0};
    U32 total_processed_links = 0;
    for(U32 i = 0; i < config_data.thread_count; i++) {
        total_processed_links += thread_data[i].total_processed_links;
    }     

    final_sort_data.date_list = (char **)ec_malloc(sizeof(char *) * total_processed_links);
    final_sort_data.final_link_list = (char **)ec_malloc(sizeof(char *) * total_processed_links); 
    final_sort_data.total_processed_links = total_processed_links;

    U32 position = 0;

    for(U32 i = 0; i < config_data.thread_count; i++) {
        for(U32 j = 0; j < thread_data[i].total_processed_links; j++) {
            final_sort_data.date_list[j + position] = thread_data[i].date_list[j];
            final_sort_data.final_link_list[j + position] = thread_data[i].final_link_list[j];
        }

        position += thread_data[i].total_processed_links;
    }

    sort_dates_thread(&final_sort_data);

    for(U32 i = 0; i < total_processed_links; i++)
        printf("%s %s\n", final_sort_data.date_list[i], final_sort_data.final_link_list[i]);

    free(final_sort_data.date_list);
    free(final_sort_data.final_link_list);

    for(U32 i = 0; i < config_data.thread_count; i++) {
        for(U32 j = 0; j < thread_data[i].total_processed_links; j++) {
            if(thread_data[i].date_list[j]) { 
#ifdef _DEBUG
                //printf("%s ", thread_data[i].date_list[j]);
#endif
                free(thread_data[i].date_list[j]);
            } else ERRQ("NULL DATA SECTOR ERROR!");

            if(thread_data[i].final_link_list[j]) {
#ifdef _DEBUG
                //printf("%s", thread_data[i].final_link_list[j]);
#endif
                free(thread_data[i].final_link_list[j]);
            } else ERRQ("NULLL DATA SECTOR ERROR!");

#ifdef _DEBUG
            //printf("\n");
#endif
        }

        free(thread_data[i].final_link_list);
        free(thread_data[i].date_list);
    }

    free(thread_data);

    fflush(stdout);

    // Kill Crypto
    kill_locks();
    DEBUG("Crypto locks killed");

    // Cleanup config
    unload_config_data(&config_data);
    DEBUG("Config data unloaded");

    return EXIT_SUCCESS;
}
