#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND 1
#define MALLOCERROR -1
#define MAXLINERROR -2
#define MAXCHARERROR -3
#define RECURSIONERROR -4
#define MAXRULEERROR -5

#define MAXLINE 1000
#define MAXCHAR 5000
#define MAXRULE 10000

#define MAXRECURSION MAXLINE*MAXLINE

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
        case RECURSIONERROR:
            printf("Recursion limit reached\n");
            break;
        case MAXRULEERROR:
            printf("MAXRULE not enough\n");
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
    return -1;
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

// recursively swap wrongly ordered pages and see if we can fix the order
static int inline attempt_swap(uint32_t* pages, uint32_t Npages, struct Rule* relevant_rules, uint32_t Nrelevant_rules, uint32_t* depth){
    // let's make sure we don't go into infinite recursion
    if ((*depth)++ > MAXRECURSION){
        return RECURSIONERROR;
    }
    uint32_t i, ir, temp;
    uint32_t page_before_index = UINT32_MAX;
    uint32_t page_after_index = UINT32_MAX;
    for (i=0;i<Nrelevant_rules;i++){
        for (ir=0;ir<Npages;ir++){
            if (pages[ir] == relevant_rules[i].page_before){
                page_before_index = ir;
            }
            if (pages[ir] == relevant_rules[i].page_after){
                page_after_index = ir;
            }
        }
        assert(page_before_index != UINT32_MAX);
        assert(page_after_index != UINT32_MAX);
        if (page_before_index > page_after_index){
            // we need to swap
            temp = pages[page_before_index];
            pages[page_before_index] = pages[page_after_index];
            pages[page_after_index] = temp;
            attempt_swap(pages, Npages, relevant_rules, Nrelevant_rules, depth);
        }
    }
    assert(is_correct_order(pages, Npages, relevant_rules, Nrelevant_rules));
    return 0;
}

static int inline fix_order(uint32_t* pages, uint32_t Npages, struct Rule* rules, uint32_t Nrules, struct Rule* relevant_rules, uint32_t* Nrelevant_rules){
    uint32_t i, j=0;
    // let us copy over only the relevant rules
    for (i=0;i<Nrules;i++){
        if (check_rule(pages, Npages, rules + i)>=0){
            relevant_rules[j] = rules[i];
            j++;
        }
    }
    *Nrelevant_rules = j;
    uint32_t depth = 0;
    return attempt_swap(pages, Npages, relevant_rules, *Nrelevant_rules, &depth);
}

int get_result(const char* filename, uint32_t* result){

    int info = 0;
    *result = 0;
    uint32_t Npages;
    int is_parsing_rules = 1;
    uint32_t Nrules=0;
    uint32_t Nrelevant_rules=0;

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
    struct Rule* rules = (struct Rule*)malloc(MAXRULE*sizeof(struct Rule));
    if(!rules){
        info = MALLOCERROR;
        goto free_rules;
    }
    struct Rule* relevant_rules = (struct Rule*)malloc(MAXRULE*sizeof(struct Rule));
    if(!relevant_rules){
        info = MALLOCERROR;
        goto free_relevant_rules;
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
            if (Nrules >= MAXRULE){
                info = MAXRULEERROR;
                goto free_all;
            }
        }
        else{
            info = parse_pages(line, pages, &Npages);
            if (info){
                goto free_all;
            }
            if (!is_correct_order(pages, Npages, rules, Nrules)){
                info = fix_order(pages, Npages, rules, Nrules, relevant_rules, &Nrelevant_rules);
                if (info){
                    goto free_all;
                }
                assert(is_correct_order(pages, Npages, rules, Nrules));
                *result += get_middle(pages, Npages);
            }
            // else{
            //     assert(is_correct_order(pages, Npages, rules, Nrules));
            //     *result += get_middle(pages, Npages);
            // }
        }
        // a newline separates rules from page lists
        if(strlen(line) == 1 && line[0] == '\n'){
            is_parsing_rules = 0;
        }
        
    }
    
free_all:
    fclose(ptr);    
free_relevant_rules:
    free(relevant_rules);

free_rules:
    free(rules);

free_pages:
    free(pages);

free_line:
    free(line);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t result;
    info = get_result("test.txt", &result);

    assert(info == 0);
    assert(result == 123);

    info = get_result("input.txt", &result);
    printf("The solution to part 2 is: %d\n", result);
    assert(info == 0);
    return info;
}