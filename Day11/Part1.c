#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXLENERROR    -4
#define MAXDIGITSERROR -5


#define MAXCHAR    1000000000
#define MAXLEN     INT32_MAX
#define MAXIDGITS  100

#define NEVOLUTIONS 25


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
        case MAXLENERROR:
            printf("MAXLEN not enough\n");
            break;
        case MAXDIGITSERROR:
            printf("MAXDIGITS not enough\n");
            break;
        default:
            break;
    }
}

// parse a line of result and operands
// of the form
// \d+ \d+ \d+ ...
static int inline parse_line(char* line, uint64_t* operands, uint64_t* Nstones){
    char* token = strtok(line, " ");
    *Nstones = 0;
    while(token){
        if (*Nstones >= MAXLEN){
            return MAXLENERROR;
        }
        operands[(*Nstones)++] = atoll(token);
        token = strtok(NULL, " ");
    }
    return 0;
}

static int inline get_digits(uint64_t number, char* digits, uint64_t* Ndigits){
    *Ndigits = 0;
    while(number){
        digits[*Ndigits] = number%10;
        number /= 10;
        (*Ndigits)++;
        if ((*Ndigits) >= MAXIDGITS){
            return MAXDIGITSERROR;
        }
    }
    return 0;
}

static int inline split_and_shuffle_over(uint64_t idx, uint64_t* stones, uint64_t* Nstones, char* digits, uint64_t Ndigits){
    // the first half of the digits go to stones[idx]
    // the second half of the digits go to stones[idx+1]
    // everything else must shuffle over to make room

    // check that there is enough room
    if ((*Nstones) + 1 >= MAXLEN){
        return MAXLENERROR;
    }
    // make room
    uint64_t i;
    for (i=(*Nstones); i>idx; i--){
        stones[i+1] = stones[i];
    }
    (*Nstones)++;
    uint64_t midpoint = Ndigits/2;
    stones[idx+1] = digits[midpoint-1];
    stones[idx]   = digits[Ndigits-1];

    for (i=midpoint-1; i>0; i--){
        if (i > Ndigits){
            return FILENOTFOUND;
        }
        stones[idx+1] *= 10;
        stones[idx+1] += digits[i-1];
        stones[idx] *= 10;
        stones[idx] += digits[midpoint+i-1];
    }

    return 0;
}
    

static int inline evolve(uint64_t* stones, uint64_t* Nstones){
    int info = 0;
    char digits[MAXIDGITS];
    uint64_t Ndigits;
    for (uint64_t i=0; i<(*Nstones); i++){
        if(stones[i] == 0){
            stones[i] = 1;
        }
        else{
            info = get_digits(stones[i], digits, &Ndigits);
            if (info){
                return info;
            }
            if ((Ndigits & 1) == 0){ // even number of digits
                info = split_and_shuffle_over(i, stones, Nstones, digits, Ndigits);
                i++;
                if (info){
                    return info;
                }
            }
            else{
                stones[i] *= 2024;
            }
        }
    }
    return info;
}

int get_result(const char* filename, uint64_t* result){

    int info = 0;
    *result = 0;
    uint64_t Nstones;
    uint64_t total;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    uint64_t* stones = (uint64_t*)malloc(MAXLEN*sizeof(uint64_t));
    if (!stones){
        info = MALLOCERROR;
        goto free_stones;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    fgets(line, MAXCHAR, ptr);
    if (!((line[strlen(line)-1] == '\n') || feof(ptr))){
        info = MAXCHARERROR;
        goto free_all;
    }
    // if there is a newline, remove it
    if (line[strlen(line)-1] == '\n'){
        line[strlen(line)-1] = '\0';
    }
    info = parse_line(line, stones, &Nstones);
    if (info){
        goto free_all;
    }
    fclose(ptr); 
    ptr = NULL;

    for (int i=0; i<NEVOLUTIONS; i++){
        info = evolve(stones, &Nstones);
        if (info){
            goto free_all;
        }
    }
    *result = Nstones;

free_all:
    if (ptr){
        fclose(ptr);    
    }

free_stones:
    free(stones);

free_line:
    free(line);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint64_t result;
    info = get_result("test.txt", &result);
    printf("The solution to the test is: %llu\n", result);
    assert(info == 0);
    assert(result == 55312);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %llu\n", result);
    assert(info == 0);
    return info;
}