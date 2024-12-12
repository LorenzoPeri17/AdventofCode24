#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXLENERROR    -4

#define MAXCHAR    1000
#define MAXLEN     (uint64_t)((2ULL<<42ULL)-1ULL)

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
        if (*Nstones >= MAXCHAR){
            return MAXCHARERROR;
        }
        operands[(*Nstones)++] = atoll(token);
        token = strtok(NULL, " ");
    }
    return 0;
}

// N can be at most 20
static uint64_t inline pow10_fast(const uint32_t N){
    switch (N){
    case 0:
        return 1ULL;
    case 1:
        return 10ULL;
    case 2:
        return 100ULL;
    case 3:
        return 1000ULL;
    case 4:
        return 10000ULL;
    case 5:
        return 100000ULL;
    case 6:
        return 1000000ULL;
    case 7:
        return 10000000ULL;
    case 8:
        return 100000000ULL;
    case 9:
        return 1000000000ULL;
    case 10:
        return 10000000000ULL;
    case 11:
        return 100000000000ULL;
    case 12:
        return 1000000000000ULL;
    case 13:
        return 10000000000000ULL;
    case 14:
        return 100000000000000ULL;
    case 15:
        return 1000000000000000ULL;
    case 16:
        return 10000000000000000ULL;
    case 17:
        return 100000000000000000ULL;
    case 18:
        return 1000000000000000000ULL;
    case 19:
        return 10000000000000000000ULL;
    default:
        return 0ULL; // for case 20 and above (would overflow uint64_t)
    }
}

static int32_t inline get_digits_uint64(const uint64_t n) {
    if (n < 100000000000ULL) { // 1 to 11
        if (n < 100000) { // 1 to 5
            if (n < 100) {
                if (n < 10) return 1;
                return 2;
            }
            if (n < 1000) return 3;
            if (n < 10000) return 4;
            return 5;
        }
        if (n < 1000000) return 6;
        if (n < 10000000) return 7;
        if (n < 100000000) return 8;
        if (n < 1000000000) return 9;
        if (n < 10000000000ULL) return 10;
        return 11;
    } else { // 12 to 20
        if (n < 1000000000000ULL) return 12;
        if (n < 10000000000000ULL) return 13;
        if (n < 100000000000000ULL) return 14;
        if (n < 1000000000000000ULL) return 15;
        if (n < 10000000000000000ULL) return 16;
        if (n < 100000000000000000ULL) return 17;
        if (n < 1000000000000000000ULL) return 18;
        if (n < 10000000000000000000ULL) return 19;
        return 20;
    }
}

static int inline split(const uint64_t idx, uint64_t* stones, uint64_t* Nstones, const uint64_t Ndigits){

    // check that there is enough room
    if ((*Nstones) + 1 >= MAXLEN){
        return MAXLENERROR;
    }
    const lldiv_t dv = lldiv(stones[idx], pow10_fast(Ndigits/2));
    stones[idx] = dv.quot;
    stones[(*Nstones)++] = dv.rem;
    return 0;
}
    

static int inline evolve(uint64_t* stones, uint64_t* Nstones){
    int info = 0;
    uint64_t Ndigits;
    const uint64_t curr_stones = *Nstones;
    for (uint64_t i=0; i<curr_stones; i++){
        if(stones[i] == 0){
            stones[i] = 1ULL;
        }
        else{
            Ndigits = get_digits_uint64(stones[i]);
            if ((Ndigits & 1) == 0){ // even number of digits
                info = split(i, stones, Nstones, Ndigits);
                if (info){
                    return info;
                }
            }
            else{
                stones[i] *= 2024ULL;
            }
        }
    }
    return info;
}

int get_result(const char* filename, const int Nevolutions, uint64_t* result){

    int info = 0;
    *result = 0;
    uint64_t Nstones, Ninitial;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    uint64_t* initial_stones = (uint64_t*)malloc(MAXCHAR*sizeof(uint64_t));
    if (!initial_stones){
        info = MALLOCERROR;
        goto free_initial_stones;
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
    info = parse_line(line, initial_stones, &Ninitial);
    if (info){
        goto free_all;
    }
    fclose(ptr); 
    ptr = NULL;

    for (uint64_t ist=0; ist<Ninitial; ist++){
        Nstones=1;
        stones[0] = initial_stones[ist];
        for (int i=0; i<Nevolutions; i++){
            printf("%llu: %d\n", ist+1, i+1);
            info = evolve(stones, &Nstones);
            if (info){
                goto free_all;
            }
        }
        *result += Nstones;
        // save the array as space separated values
        sprintf(line, "%llu_%d_%llu.txt", ist, Nevolutions, Nstones);
        ptr = fopen(line, "w");
        if (!ptr) {
            info = FILENOTFOUND;
            goto free_all;
        }
        fprintf(ptr, "%llu", stones[0]);
        for (uint64_t i=1; i<Nstones; i++){
            fprintf(ptr, " %llu", stones[i]);
        }
        fclose(ptr);
    }
    

free_all:
    if (ptr){
        fclose(ptr);    
    }

free_stones:
    free(stones);

free_initial_stones:
    free(initial_stones);

free_line:
    free(line);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint64_t result;

    info = get_result("input.txt", 50, &result);
    assert(info == 0);

    return info;
}