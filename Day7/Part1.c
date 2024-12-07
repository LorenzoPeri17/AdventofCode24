#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define uint128_t __uint128_t

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXLINEERROR   -4
#define BADLINE        -5
#define BITMASKERROR   -6

#define MAXCHAR    1000
#define MAXLINE    100

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
        case MAXLINEERROR:
            printf("MAXLINE not enough\n");
            break;
        case BADLINE:
            printf("Bad line\n");
            break;
        case BITMASKERROR:
            printf("Bitmask is not enough\n");
            break;
        default:
            break;
    }
}

// parse a line of result and operands
// of the form
// (\d+): \d+ \d+ ...
static int inline parse_line(char* line, uint64_t* operands, uint32_t* Noperands, uint64_t* total){
    char* token = strtok(line, ": ");
    if (!token){
        return BADLINE;
    }
    *total = atoll(token);
    token = strtok(NULL, " ");
    if (!token){
        return BADLINE;
    }
    *Noperands = 0;
    while(token){
        if (*Noperands >= MAXLINE){
            return MAXLINEERROR;
        }
        operands[(*Noperands)++] = atoll(token);
        token = strtok(NULL, " ");
    }
    return 0;
}

// check if a line is admissible
// total = operand[0] OP operand[1] OP ... OP operand[Noperands-1]
// where OP is either + or *
static uint64_t inline check_line(uint64_t* operands, uint32_t Noperands, uint64_t total){
    uint64_t result;
    uint64_t next;
    uint128_t operator_bitmask;
    for (operator_bitmask = 0; operator_bitmask < (1 << (Noperands)); operator_bitmask++){
        result = operands[0];
        for (int i=0; i<Noperands-1; i++){
            next = operands[i+1];
            if (operator_bitmask & (1 << i)){
                result *= next;
            }
            else{
                result += next;
            }
        }
        if (result == total){
            return total;
        }
    }
    return 0;
}

int get_result(const char* filename, uint64_t* result){

    int info = 0;
    *result = 0;
    uint32_t Noperands;
    uint64_t total;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    uint64_t* operands = (uint64_t*)malloc(MAXLINE*sizeof(uint64_t));
    if (!operands){
        info = MALLOCERROR;
        goto free_operands;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    while(fgets(line, MAXCHAR, ptr)){
        if (!((line[strlen(line)-1] == '\n') || feof(ptr))){
            info = MAXCHARERROR;
            goto free_all;
        }
        // if there is a newline, remove it
        if (line[strlen(line)-1] == '\n'){
            line[strlen(line)-1] = '\0';
        }
        info = parse_line(line, operands, &Noperands, &total);
        if (info){
            goto free_all;
        }
        if(Noperands > 128){
            info = BITMASKERROR;
            goto free_all;
        }
        (*result) += check_line(operands, Noperands, total);
    }

free_all:
        fclose(ptr);    

free_operands:
    free(operands);

free_line:
    free(line);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint64_t result;
    info = get_result("test.txt", &result);

    assert(info == 0);
    assert(result == 3749);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %llu\n", result);
    assert(info == 0);
    return info;
}