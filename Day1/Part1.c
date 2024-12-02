#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define FILENOTFOUND 1
#define MALLOCERROR -1
#define MAXLENERROR -2

#define MAXLEN 100000

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        case MAXLENERROR:
            printf("Reached MAXLEN element\n");
            break;
        default:
            break;
    }
}


static void inline sort_inplace(uint32_t* group, uint32_t len){
    for (uint32_t i = 0; i<len; i++){
        for (uint32_t j = i+1; j<len; j++){
            if (group[i]>group[j]){
                uint32_t temp = group[i];
                group[i] = group[j];
                group[j] = temp;
            }
        }
    }
    return;
}

static uint32_t inline calculate_difference(uint32_t* group1, uint32_t* group2, uint32_t len){

    uint32_t diff = 0;

    sort_inplace(group1, len);
    sort_inplace(group2, len);

    for (uint32_t i = 0; i<len; i++){
        diff += abs((int)(group1[i]-group2[i]));
    }

    return diff;
}

int get_difference(const char* filename, uint32_t* diff){

    int info = 0;
    uint32_t g1, g2;

    uint32_t * group1 = (uint32_t*)malloc(MAXLEN*sizeof(uint32_t));
    if (!group1){
        info = MALLOCERROR;
        goto free_group1;
    }
    uint32_t * group2 = (uint32_t*)malloc(MAXLEN*sizeof(uint32_t));
    if (!group2){
        info = MALLOCERROR;
        goto free_group2;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    uint32_t i = 0;
    while(fscanf(ptr, "%d %d", &g1, &g2)!=EOF){
        group1[i] = g1;
        group2[i] = g2;
        i++;
        if (i>=MAXLEN){
            info = MAXLENERROR;
            goto free_all;
        }
    }
    fclose(ptr);
    const uint32_t len = i;

    *diff = calculate_difference(group1, group2, len);

free_all:
free_group2:
    free(group2);

free_group1:
    free(group1);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t diff;

    info = get_difference("test.txt", &diff);
    // printf("Difference: %d\n", diff);

    assert(info == 0);
    assert(diff == 11);

    info = get_difference("input.txt", &diff);
    printf("The solution to part 1 is: %d\n", diff);
    assert(info == 0);
    return info;
}