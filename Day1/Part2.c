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

static uint64_t inline get_max(uint32_t* group1, uint32_t* group2, uint32_t len){
    uint64_t max = 0;
    uint32_t i;
    for (i = 0; i<len; i++){
        if (group1[i]>max){
            max = group1[i];
        }
    }
    for (i = 0; i<len; i++){
        if (group2[i]>max){
            max = group2[i];
        }
    }
    return max;
}

static void inline make_counts(uint32_t* group, uint32_t len, uint32_t* counts){
    for (uint32_t i = 0; i<len; i++){
        counts[group[i]]++;
    }
    return;
}

static uint32_t inline calculate_similarity(uint32_t* group1, uint32_t* group2, uint32_t len, uint32_t* counts_g2){

    uint32_t g1, simil = 0;
    make_counts(group2, len, counts_g2);
    for (uint32_t i = 0; i<len; i++){
        g1 = group1[i];
        simil += g1 * counts_g2[g1];
    }

    return simil;
}

int get_similarity(const char* filename, uint32_t* simil){

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

    uint64_t max = get_max(group1, group2, len);
    uint32_t* counts_g2 = (uint32_t*)calloc(max+1, sizeof(uint32_t));

    if (!counts_g2){
        info = MALLOCERROR;
        goto free_counts_g2;
    }
    *simil = calculate_similarity(group1, group2, len, counts_g2);

free_all:
free_counts_g2:
    free(counts_g2);

free_group2:
    free(group2);

free_group1:
    free(group1);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t simil;

    info = get_similarity("test.txt", &simil);

    assert(info == 0);
    assert(simil == 31);

    info = get_similarity("input.txt", &simil);
    printf("The solution to part 2 is: %d\n", simil);
    assert(info == 0);
    return info;
}