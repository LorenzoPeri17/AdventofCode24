#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXRECERROR    -4

#define MAXRECURSION  INT64_MAX

#define MAXCHAR    INT32_MAX
#define EMPTY      -1

struct EmptySpace{
    uint32_t start;
    uint32_t length;
};

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        case MAXCHARERROR:
            printf("MAXCHAR not enough\n");
            break;
        case MAXRECERROR:
            printf("Max recursion reached\n");
            break;
        default:
            break;
    }
}

// keep parsing one line from the file
// we must parse one digit at a time
// and store it in the disk array
static int inline parse_input(FILE* ptr, int32_t* disk, uint32_t* len){
    *len = 0;
    while(1){
        char c = fgetc(ptr);
        if (c == EOF || c == '\n'){
            break;
        }
        disk[*len] = c - '0';
        (*len)++;
        if (*len >= MAXCHAR){
            return MAXCHARERROR;
        }
    }
    return 0;
}

static int inline expand_disk(int32_t* disk, uint32_t len, int32_t* expanded, uint32_t* expanded_len){
    uint32_t i, j;
    uint32_t seek = 0;
    int32_t id = 0;
    int32_t next;
    char empty = 0;
    for (i = 0; i < len; i++){
        next = disk[i];
        for (j = 0; j < next; j++){
            expanded[seek++] = empty ? EMPTY : id;
            if (seek >= MAXCHAR){
                return MAXCHARERROR;
            }
        }
        if (empty){
            id++;
        }
        empty = !empty;
    }
    *expanded_len = seek;
    return 0;
}

static void inline assign_sizes(int32_t* disk, uint32_t len, int32_t* sizes, uint32_t* max_id){
    uint32_t i;
    for (i = 0; i < len; i+=2){
        sizes[i/2] = disk[i];
    }
    *max_id = i/2;
    return;
}

static void inline assign_empty_sizes(int32_t* disk, uint32_t len, struct EmptySpace* empty_sizes, uint32_t* len_empty){
    uint32_t i;
    uint32_t start = 0;
    uint32_t length;
    char empty = 0;
    (*len_empty) = 0;
    for (i = 0; i < len; i++){
        if (empty){
            length = disk[i];
            if (length > 0){
                empty_sizes[i/2].start = start;
                empty_sizes[i/2].length = length;
                (*len_empty)++;
            }
        }
        start += disk[i];
        empty = !empty;
    }
    return;
}

static int inline swap(int32_t* expanded, uint32_t expanded_len, int32_t* sizes, int32_t* id_to_swap, struct EmptySpace* empty_sizes, uint32_t len_empty, uint64_t* depth){
    (*depth)++;
    if (*depth >= MAXRECURSION){
        return MAXRECERROR;
    }
    
    if(*id_to_swap==0){
        return 0;
    }
    // locate where id_to_swap begins in expanded
    uint32_t i;
    uint32_t start, empty_used;
    for (i = 0; i < expanded_len; i++){
        if (expanded[i] == *id_to_swap){
            break;
        }
    }
    start = i;
    // find the leftmost empty space that can accommodate id_to_swap
    for (i = 0; i < len_empty; i++){
        if (empty_sizes[i].length >= sizes[*id_to_swap]){
            break;
        }
    }
    empty_used = i;
    if (empty_used < len_empty){
        if (empty_sizes[empty_used].start < start){
            // swap the id_to_swap with the empty space
            for (i = 0; i < sizes[*id_to_swap]; i++){
                expanded[empty_sizes[empty_used].start+i] = *id_to_swap;
                expanded[start+i] = EMPTY;
            }
            empty_sizes[empty_used].length -= sizes[*id_to_swap];
            empty_sizes[empty_used].start += sizes[*id_to_swap];
        }
    }

    (*id_to_swap)--;
    // print the expanded disk
    return swap(expanded, expanded_len, sizes, id_to_swap, empty_sizes, len_empty, depth);

}

static int inline solve_expanded(int32_t* expanded, uint32_t expanded_len, int32_t* sizes, uint32_t max_id, struct EmptySpace* empty_sizes, uint32_t len_empty){
    uint64_t depth = 0;
    int32_t id_to_swap = max_id-1;
    return swap(expanded, expanded_len, sizes, &id_to_swap, empty_sizes, len_empty, &depth);
}

static void inline compute_ckecksum(int32_t* expanded, uint32_t expanded_len, uint64_t* result){
    uint32_t i;
    *result = 0;
    for (i = 0; i < expanded_len; i++){
        if (expanded[i] == EMPTY){
            continue;
        } else {
            *result += i*expanded[i];
        }
    }
}

int get_result(const char* filename, uint64_t* result){

    int info = 0;
    *result = 0;
    uint32_t len, expanded_len, len_empty, max_id;

    int32_t* disk = (int32_t*)malloc(MAXCHAR * sizeof(int32_t));
    if (!disk){
        info = MALLOCERROR;
        goto free_disk;
    }
    int32_t* expanded = (int32_t*)malloc(MAXCHAR * sizeof(int32_t));
    if (!expanded){
        info = MALLOCERROR;
        goto free_expanded;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    info = parse_input(ptr, disk, &len);
    if (info != 0){
        goto free_all;
    }
    fclose(ptr);
    ptr = NULL;

    info = expand_disk(disk, len, expanded, &expanded_len);
    if (info != 0){
        goto free_all;
    }
    int32_t* sizes = (int32_t*)malloc((2+len/2) * sizeof(int32_t));
    if (!sizes){
        info = MALLOCERROR;
        goto free_sizes;
    }
    assign_sizes(disk, len, sizes, &max_id);

    struct EmptySpace* empty_sizes = (struct EmptySpace*)malloc((2+len/2) * sizeof(struct EmptySpace));
    if (!empty_sizes){
        info = MALLOCERROR;
        goto free_empty_sizes;
    }
    assign_empty_sizes(disk, len, empty_sizes, &len_empty);

    solve_expanded(expanded, expanded_len, sizes, max_id, empty_sizes, len_empty);

    compute_ckecksum(expanded, expanded_len, result);

free_all:
    if (ptr){
        fclose(ptr);
    }

free_empty_sizes:
    free(empty_sizes);

free_sizes:
    free(sizes);

free_expanded:
    free(expanded);

free_disk:
    free(disk);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint64_t result;
    info = get_result("test.txt", &result);
    assert(info == 0);
    printf("The solution to the test is: %llu\n", result);
    assert(result == 2858);

    info = get_result("input.txt", &result);
    printf("The solution to part 2 is: %llu\n", result);
    assert(info == 0);
    return info;
}