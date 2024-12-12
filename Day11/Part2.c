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

#define MAXCHAR    10000
#define MAXLEN     100

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
static int inline parse_line(char* line, uint64_t* stones, uint64_t* Nstones){
    char* token = strtok(line, " ");
    *Nstones = 0;
    while(token){
        if ((uint64_t)(*Nstones) >= MAXLEN){
            return MAXLENERROR;
        }
        stones[(*Nstones)++] = atoll(token);
        token = strtok(NULL, " ");
    }
    return 0;
}

// N can be at most 20
static uint64_t inline pow10_fast(uint32_t N){
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

static int32_t inline get_digits_uint64(uint64_t n) {
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


// static uint64_t inline evolve_digit(const uint64_t stone, const int32_t remaining){
//     if(remaining <= 0){
//         return 1;
//     }
//     int32_t next_iter = remaining-1;
//     uint32_t Ndigits;
//     lldiv_t dv;
//     switch (stone){
//     case 0:
//         return evolve_digit(2024, next_iter-1);
//         break;
//     case 1:
//         return evolve_digit(2024, next_iter);
//         break;
//     default:
//         // Ndigits = (uint32_t)floorl(log10l((long double) stone)) + 1;
//         Ndigits = get_digits_uint64(stone);
//         if (!(Ndigits & 1)){ // even number of digits
//             dv = lldiv(stone, pow10_fast(Ndigits/2));
//             return evolve_digit(dv.quot, next_iter) + evolve_digit(dv.rem, next_iter);
//         }
//         else{
//             return evolve_digit(stone * 2024, next_iter);
//         }
//     break;
//     }
// }

static uint64_t inline evolve_digit(const uint64_t stone, const int32_t remaining){
    if(remaining == 0){
        return 1ULL;
    }
    lldiv_t dv;
    int32_t next_iter = remaining;
    uint64_t next_stone = stone;
    if (next_stone == 0){
        next_stone = 1;
        next_iter--;
    }
    uint32_t Ndigits = get_digits_uint64(next_stone);

    while((Ndigits & 1) && (next_iter>0)){ // odd number of digits
        next_stone *= 2024ULL;
        Ndigits = get_digits_uint64(next_stone);
        next_iter--;
    }
    if(next_iter == 0){
        return 1ULL;
    }
    else{
        // printf("%d \r", next_iter);
        dv = lldiv(next_stone, pow10_fast(Ndigits/2));
        return evolve_digit(dv.quot, next_iter-1) + evolve_digit(dv.rem, next_iter-1);
    }
}

int get_result(const char* filename, int Nevolutions, uint64_t* result){

    int info = 0;
    *result = 0;
    uint64_t Nstones;

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

    for (uint64_t i = 0; i<Nstones; i++){
        printf("Evolving %llu\n", i+1);
        *result += evolve_digit(stones[i], Nevolutions);
    }

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
    info = get_result("test.txt", 25, &result);
    // printf("The solution to test is: %llu\n", result);
    assert(info == 0);
    assert(result == 55312);

    info = get_result("input.txt", 25, &result);
    assert(info == 0);
    assert(result == 200446);

    // info = get_result("input.txt", 40, &result);

    info = get_result("input.txt", 75, &result);
    printf("The solution to part 2 is: %llu\n", result);
    assert(info == 0);
    return info;
}