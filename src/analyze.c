// analyze.c
//
/*
 *
 * Post scrape functions
 *      - Multithreaded Sort
 *      - Remove unwanted links
 *
 */
#include <stdio.h>

#include "stdafx.h"
#include "thread_types.h"

void sort_arrays(int *days, int *months, int *years, const U32 max, pTHREADDATA thread_data) {
    BOOL switching = TRUE;
    BOOL switching_lines = FALSE;

    while(switching == TRUE) {
        switching = FALSE;

        U32 i = 0;
        for(; i < (max - 1); i++) {
            switching_lines = FALSE;

            if(days[i] > days[i + 1]) {
                switching_lines = TRUE;
                break;
            }
        }

        if(switching_lines) {
            int temp = days[i];
            days[i] = days[i + 1];
            days[i + 1] = temp;

            temp = months[i];
            months[i] = months[i + 1];
            months[i + 1] = temp;

            temp = years[i];
            years[i] = years[i + 1];
            years[i + 1] = temp;

            char *temp2 = thread_data->date_list[i];
            thread_data->date_list[i] = thread_data->date_list[i + 1];
            thread_data->date_list[i + 1] = temp2;

            temp2 = thread_data->final_link_list[i];
            thread_data->final_link_list[i] = thread_data->final_link_list[i + 1];
            thread_data->final_link_list[i + 1] = temp2;

            switching = TRUE;
        }
    }

    switching = TRUE;
    switching_lines = FALSE;

    while(switching == TRUE) {
        switching = FALSE;

        U32 i = 0;
        for(; i < (max - 1); i++) {
            switching_lines = FALSE;

            if(months[i] > months[i + 1]) {
                switching_lines = TRUE;
                break;
            }
        }

        if(switching_lines) {
            int temp = days[i];
            days[i] = days[i + 1];
            days[i + 1] = temp;

            temp = months[i];
            months[i] = months[i + 1];
            months[i + 1] = temp;

            temp = years[i];
            years[i] = years[i + 1];
            years[i + 1] = temp;

            char *temp2 = thread_data->date_list[i];
            thread_data->date_list[i] = thread_data->date_list[i + 1];
            thread_data->date_list[i + 1] = temp2;

            temp2 = thread_data->final_link_list[i];
            thread_data->final_link_list[i] = thread_data->final_link_list[i + 1];
            thread_data->final_link_list[i + 1] = temp2;

            switching = TRUE;
        }
    }

    switching = TRUE;
    switching_lines = FALSE;

    while(switching == TRUE) {
        switching = FALSE;

        U32 i = 0;
        for(; i < (max - 1); i++) {
            switching_lines = FALSE;

            if(years[i] > years[i + 1]) {
                switching_lines = TRUE;
                break;
            }
        }

        if(switching_lines) {
            int temp = days[i];
            days[i] = days[i + 1];
            days[i + 1] = temp;

            temp = months[i];
            months[i] = months[i + 1];
            months[i + 1] = temp;

            temp = years[i];
            years[i] = years[i + 1];
            years[i + 1] = temp;

            char *temp2 = thread_data->date_list[i];
            thread_data->date_list[i] = thread_data->date_list[i + 1];
            thread_data->date_list[i + 1] = temp2;

            temp2 = thread_data->final_link_list[i];
            thread_data->final_link_list[i] = thread_data->final_link_list[i + 1];
            thread_data->final_link_list[i + 1] = temp2;

            switching = TRUE;
        }
    }
}

void sort_dates_thread(pTHREADDATA thread_data) {
    const U32 MAX_DATES = thread_data->total_processed_links;

    if(MAX_DATES <= 1) return;

    char syear[5] = {0}, smonth[3] = {0}, sday[3] = {0};
    int years[MAX_DATES], months[MAX_DATES], days[MAX_DATES];

    // Seperate days/months/years into seperate arrays
    for(unsigned int i = 0; i < MAX_DATES; i++) {
        memcpy(syear, thread_data->date_list[i], 4);
        memcpy(smonth, thread_data->date_list[i] + 5, 2);
        memcpy(sday, thread_data->date_list[i] + 8, 2);

        years[i] = atoi(syear);
        months[i] = atoi(smonth);
        days[i] = atoi(sday);
    }

    sort_arrays(days, months, years, MAX_DATES, thread_data);  
}
