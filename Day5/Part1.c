#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND 1
#define MALLOCERROR -1
#define MAXLINERROR -2
#define MAXCHARERROR -3

#define MAXLINE 100
#define MAXCHAR 500

struct Rule{
    uint32_t page_before;
    uint32_t page_after;
};

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        case MAXLINERROR:
            printf("MAXLINE not enough\n");
            break;
        case MAXCHARERROR:
            printf("MAXCHAR not enough\n");
            break;
        default:
            break;
    }
}

// Parse a line containing a rule
// of the form
// \d+|\d+
static int inline parse_rule(char* line, struct Rule* rule){
    sscanf(line, "%d|%d", &rule->page_before, &rule->page_after);
    return 0;
}

// parse a line containing a list of pages
// of the form
// \d+,\d+,\d+...
static int inline parse_pages(char* line, uint32_t* pages, uint32_t* Npages){
    uint32_t i = 0;
    char* token = strtok(line, ",");
    while(token){
        pages[i] = atoi(token);
        i++;
        token = strtok(NULL, ",");
        if (i>=MAXLINE){
            return MAXLINERROR;
        }
    }
    *Npages = i;
    return 0;
}

static int inline check_rule(uint32_t* pages, uint32_t Npages, struct Rule* rule){
    uint32_t i;
    uint32_t page_before_index = UINT32_MAX;
    uint32_t page_after_index = UINT32_MAX;
    for (i=0;i<Npages;i++){
        if (pages[i] == rule->page_before){
            page_before_index = i;
        }
        if (pages[i] == rule->page_after){
            page_after_index = i;
        }
    }
    if ((page_before_index != UINT32_MAX)&&(page_after_index != UINT32_MAX)){
        // we found both numbers
        return page_before_index < page_after_index ? 1 : 0;
    }
    return 1;
}

static int inline is_correct_order(uint32_t* pages, uint32_t Npages, struct Rule* rules, uint32_t Nrules){
    uint32_t i, j;
    for (i=0;i<Nrules;i++){
        if (!check_rule(pages, Npages, rules + i)){
            return 0;
        }
    }
    return 1;
}

static uint32_t inline get_middle(uint32_t* pages, uint32_t Npages){
    return pages[Npages/2];
}

int get_result(const char* filename, uint32_t* result){

    int info = 0;
    *result = 0;
    uint32_t Npages;
    int is_parsing_rules = 1;
    uint32_t Nrules=0;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    uint32_t* pages = (uint32_t*)malloc(MAXLINE*sizeof(uint32_t));
    if (!pages){
        info = MALLOCERROR;
        goto free_pages;
    }
    struct Rule* rules = (struct Rule*)malloc(MAXLINE*sizeof(struct Rule));
    if(!rules){
        info = MALLOCERROR;
        goto free_rules;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    while(fgets(line, MAXLINE, ptr)){
        // check we read until a newline or EOF
        if (!((line[strlen(line)-1] == '\n') || feof(ptr))){
            info = MAXCHARERROR;
            goto free_all;
        }
        if(is_parsing_rules){
            info = parse_rule(line, rules + Nrules);
            Nrules++;
        }
        else{
            info = parse_pages(line, pages, &Npages);
            if (info){
                goto free_all;
            }
            if (is_correct_order(pages, Npages, rules, Nrules)){
                *result += get_middle(pages, Npages);
            }
        }
        // a newline separates rules from page lists
        if(strlen(line) == 1 && line[0] == '\n'){
            is_parsing_rules = 0;
        }
        
    }
    
free_all:
free_pages:
    free(pages);

free_rules:
    free(rules);

free_line:
    free(line);

    fclose(ptr);
    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t result;
    info = get_result("test.txt", &result);

    assert(info == 0);
    assert(result == 143);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %d\n", result);
    assert(info == 0);
    return info;
}