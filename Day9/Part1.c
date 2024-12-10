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

static int inline swap(int32_t* expanded, uint32_t* first_empty, uint32_t* last_occupied, uint64_t* depth){
    (*depth)++;
    if (*depth >= MAXRECURSION){
        return MAXRECERROR;
    }
    assert(*first_empty < *last_occupied);
    assert(expanded[*first_empty] == EMPTY);
    assert(expanded[*last_occupied] != EMPTY);
    uint32_t i;
    expanded[*first_empty] = expanded[*last_occupied];
    expanded[*last_occupied] = EMPTY;
    // find the next empty
    for (i = *first_empty; i <= *last_occupied; i++){
        if (expanded[i] == EMPTY){
            break;
        }
    }
    *first_empty = i;
    // find the next occupied and keep going
    for (i = *last_occupied; i >= *first_empty; i--){
        if (expanded[i] != EMPTY){
            break;
        }
    }
    *last_occupied = i;
    if (*first_empty >= *last_occupied){
        return 0;
    }
    return swap(expanded, first_empty, last_occupied, depth);
}

static int inline solve_expanded(int32_t* expanded, uint32_t expanded_len){
    uint32_t first_empty, last_occupied;
    uint32_t i;
    uint64_t depth = 0;
    // find the first empty
    for (i = 0; i < expanded_len; i++){
        if (expanded[i] == EMPTY){
            first_empty = i;
            break;
        }
    }
    // find the last occupied
    for (i = expanded_len - 1; i >= 0; i--){
        if (expanded[i] != EMPTY){
            last_occupied = i;
            break;
        }
    }
    return swap(expanded, &first_empty, &last_occupied, &depth);
}

static void inline compute_ckecksum(int32_t* expanded, uint32_t expanded_len, uint64_t* result){
    uint32_t i;
    char found_stop = 0;
    *result = 0;
    for (i = 0; i < expanded_len; i++){
        if (expanded[i] == EMPTY){
            if (!found_stop){
                found_stop = 1;
            }
        } else {
            *result += i*expanded[i];
            assert(!found_stop);
        }
    }
}

int get_result(const char* filename, uint64_t* result){

    int info = 0;
    *result = 0;
    uint32_t len, expanded_len;

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

    solve_expanded(expanded, expanded_len);
    compute_ckecksum(expanded, expanded_len, result);

free_all:
    if (ptr){
        fclose(ptr);
    }

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
    assert(result == 1928);

    info = get_result("test1.txt", &result);
    assert(info == 0);
    printf("The solution to the test is: %llu\n", result);
    assert(result == 60);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %llu\n", result);
    assert(info == 0);
    return info;
}