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
#define MAXLINEERROR   -5

#define MAXCHAR    (uint64_t)((2ULL<<30ULL)-1ULL)
#define MAXLINE    1000000000
#define MAXLEN     (uint64_t)((2ULL<<40ULL)-1ULL)

struct cached{
    uint64_t stone;
    uint64_t Nstones;
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
        case MAXLENERROR:
            printf("MAXLEN not enough\n");
            break;
        case MAXLINEERROR:
            printf("MAXLINE not enough\n");
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

static int inline read_while(FILE* ptr, char* line, const char term){
    char c;
    uint64_t i = 0;
    while (fread(&c, sizeof(char), 1, ptr) == 1){
        if (c == term){
            break;
        }
        line[i++] = c;
        if (i >= MAXLINE){
            return MAXLINEERROR;
        }
    }
    line[i] = '\0';
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

static uint64_t inline find_in_cache(const uint64_t stone, const struct cached* cache, const uint64_t cache_len){
    for (uint64_t i=0; i<cache_len; i++){
        if (cache[i].stone == stone){
            return cache[i].Nstones;
        }
    }
    return 0;
}

static void inline add_to_cache(const uint64_t stone, const uint64_t Nstones, struct cached* cache, uint64_t* cache_len){
    cache[*cache_len].stone = stone;
    cache[*cache_len].Nstones = Nstones;
    (*cache_len)++;
}

int get_result_cached(const char* filename, const int Nevolutions, uint64_t* result, struct cached* cache, uint64_t* cache_len, const uint64_t expected_initial_len){

    int info = 0;
    uint64_t Nstones, Ninitial;

    char* line = (char*)malloc(MAXLINE*sizeof(char));
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

    // read while the file is not at the EOF
    Ninitial = 0;
    while(!feof(ptr)){
        read_while(ptr, line, ' ');
        initial_stones[Ninitial++] = atoll(line);
        if (Ninitial >= MAXCHAR){
            info = MAXCHARERROR;
            goto free_all;
        }
    }
    fclose(ptr); 
    ptr = NULL;

    assert(expected_initial_len == Ninitial);

    uint64_t cache_res;
    for (uint64_t ist=0; ist<Ninitial; ist++){
        Nstones=1;
        stones[0] = initial_stones[ist];
        if ((cache_res = find_in_cache(stones[0], cache, *cache_len))){
            Nstones = cache_res;
        } else {
            for (int i=0; i<Nevolutions; i++){
                info = evolve(stones, &Nstones);
                if (info){
                    goto free_all;
                }
            }
            add_to_cache(initial_stones[ist], Nstones, cache, cache_len);
        }
        *result += Nstones;
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
    uint64_t result = 0;

    struct cached* cache = (struct cached*)calloc(MAXLEN, sizeof(struct cached));
    if (!cache){
        info = MALLOCERROR;
        print_info(info);
        return info;
    }
    uint64_t cache_len = 0;

    result = 0;
    info = get_result_cached("test.txt", 25, &result, cache, &cache_len, 2ULL);
    assert(info == 0);
    assert(result == 55312);

    info = get_result_cached("input.txt", 25, &result, cache, &cache_len, 8ULL);
    assert(info == 0);
    assert(result == 200446+55312);

    char* files[] =
    {
    "0_50_743059691.txt",
    "1_50_757249564.txt",
    "2_50_663251546.txt",
    "3_50_1010392024.txt",
    "4_50_684173008.txt",
    "5_50_1519869935.txt",
    "6_50_318432949.txt",
    "7_50_1204152645.txt",
    };

    uint64_t initial_lengths[] = {
    743059691ULL,
    757249564ULL,
    663251546ULL,
    1010392024ULL,
    684173008ULL,
    1519869935ULL,
    318432949ULL,
    1204152645ULL,
    };

    result = 0;
    for (int i=0; i<8; i++){
        printf("Processing file %d\n", i+1);
        info = get_result_cached(files[i], 25, &result, cache, &cache_len, initial_lengths[i]);
        if (info){
            printf("Error in file %d\n", i+1);
            break;
        }
        printf("Result until now: %llu\n", result);
    }

    printf("Result: %llu\n", result);
    free(cache);
    return info;
}