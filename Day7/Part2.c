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

enum Operator{
    ADD     = 0,
    MUL     = 1,
    CONCAT  = 2,
};

static void inline initialize_operators(enum Operator* operators, uint32_t Noperands){
    for (int i=0; i<Noperands-1; i++){
        operators[i] = ADD;
    }
}

// we want to loop through all the possible permutations of the operators
static void inline increment_operators(enum Operator* operators, uint32_t Noperands){
    int carry = 1;
    int i = 0;
    while (carry) {
        operators[i] = (operators[i] + 1) % 3;
        carry = (operators[i] == 0);
        i++;
        if (i == Noperands-1){
            break;
        }
    }
}

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

static int inline digit_count(uint64_t n){
    int count = 0;
    while(n){
        n /= 10;
        count++;
    }
    return count;
}

static uint64_t inline pow10(int n){
    uint64_t result = 1;
    for (int i=0; i<n; i++){
        result *= 10;
    }
    return result;
}

static uint64_t inline concat(uint64_t a, uint64_t b){
    return a*pow10(digit_count(b)) + b;
}

static int inline total_combinations(int Noperands){
    int res = 1;
    for (int i=0; i<Noperands-1; i++){
        res *= 3;
    }
    return res;
}

// check if a line is admissible
// total = operand[0] OP operand[1] OP ... OP operand[Noperands-1]
// where OP is either + or * or CONCAT
static uint64_t inline check_line_with_concat(uint64_t* operands, enum Operator* operators, uint32_t Noperands, uint64_t total){
    uint64_t result;
    uint64_t next;
    int iop;
    for (iop = 0, initialize_operators(operators, Noperands); iop < total_combinations(Noperands); iop++, increment_operators(operators, Noperands)){
        result = operands[0];
        for (int i=0; i<Noperands-1; i++){
            next = operands[i+1];
            switch(operators[i]){
                case ADD:
                    result += next;
                    break;
                case MUL:
                    result *= next;
                    break;
                case CONCAT:
                    result = concat(result, next);
                    break;
                default:
                    break;
            }
        }
        if (result == total){
            return result;
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
    enum Operator* operators = (enum Operator*)malloc(MAXLINE*sizeof(enum Operator));
    if (!operators){
        info = MALLOCERROR;
        goto free_operators;
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
        (*result) += check_line_with_concat(operands, operators, Noperands, total);
    }

free_all:
        fclose(ptr);    

free_operators:
    free(operators);

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
    assert(result == 11387);

    info = get_result("input.txt", &result);
    printf("The solution to part 2 is: %llu\n", result);
    assert(info == 0);
    return info;
}