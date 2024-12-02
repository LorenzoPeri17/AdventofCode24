#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND 1
#define MALLOCERROR -1
#define MAXLINERROR -2

#define MAXLINE 25
#define MAXCHAR 100

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        case MAXLINERROR:
            printf("Reached MAXLEN element\n");
            break;
        default:
            break;
    }
}

// Parse a line of the file
// of the form 
// \d+ \d+ \d+ ...
static int inline parse_line(char* line, uint32_t* levels, uint32_t* Nlevels){
    uint32_t i = 0;
    char* token = strtok(line, " ");
    while(token){
        levels[i] = atoi(token);
        i++;
        token = strtok(NULL, " ");
        if (i>=MAXLINE){
            return MAXLINERROR;
        }
    }
    *Nlevels = i;
    return 0;
}

static uint32_t inline is_safe(uint32_t* levels, uint32_t Nlevels){
    int slope = levels[1] > levels[0] ? 1 : -1;
    int diff, new_slope;
    for (uint32_t i=1; i<Nlevels; i++){
        // * check if the slope is always the same
        new_slope = levels[i] > levels[i-1] ? 1 : -1;
        if (new_slope != slope){
            return 0;
        }
        // * check the difference is between 1 and 3
        diff = abs((int)levels[i] - (int)levels[i-1]);
        if ((diff > 3)||(diff < 1)){
            return 0;
        }     
    }
    return 1;
}

int get_safe_count(const char* filename, uint32_t* count){

    int info = 0;
    *count = 0;
    char line[MAXCHAR];
    uint32_t Nlevels;

    uint32_t* level = (uint32_t*)malloc(MAXLINE*sizeof(uint32_t));
    if (!level){
        info = MALLOCERROR;
        goto free_level;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    while(fgets(line, MAXLINE, ptr)){
        info = parse_line(line, level, &Nlevels);
        if (info){
            goto free_all;
        }
        *count += is_safe(level, Nlevels);
    }
    fclose(ptr);
    
    
    

free_all:
free_level:
    free(level);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t count;

    info = get_safe_count("test.txt", &count);
    // printf("Count: %d\n", count);

    assert(info == 0);
    assert(count == 2);

    info = get_safe_count("input.txt", &count);
    printf("The solution to part 1 is: %d\n", count);
    assert(info == 0);
    return info;
}