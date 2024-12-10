#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXWIDTHERROR  -4
#define MAXHEIGHTERROR -5
#define RECURSIONERROR -6


#define MAXCHAR    1000
#define MAXWIDTH   MAXCHAR
#define MAXHEIGHT  5000
#define MAXRECURSION  MAXWIDTH*MAXHEIGHT*100


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
        case MAXWIDTHERROR:
            printf("MAXWIDTH not enough\n");
            break;
        case MAXHEIGHTERROR:
            printf("MAXHEIGHT not enough\n");
            break;
        case RECURSIONERROR:
            printf("Recursion limit reached\n");
            break;
        default:
            break;
    }
}

static int inline get_pos(int x, int y, int width){
    return x + y*width;
}


static void inline parse_line(char* line, uint32_t* map, int width){
    for (int i=0;i<width;i++){
        map[i] = line[i] - '0';
    }
    return;
}

static int inline continue_trail(uint32_t looking_for, uint32_t* map, uint32_t* visited_map, int x, int y, const int width, const int height, uint64_t* depth){
    (*depth)++;
    if (*depth > MAXRECURSION){
        return RECURSIONERROR;
    }
    if (looking_for == 10){
        assert(map[get_pos(x, y, width)] == 9);
        visited_map[get_pos(x, y, width)] += 1;
        return 0;
    }
    // First we check if we are out of bounds
    // then we check if it is the number we are looking for)
    int info = 0;
    // check if we can go left
    if (x>0){
        if (map[get_pos(x-1, y, width)] == looking_for){
            info = continue_trail(looking_for+1, map, visited_map, x-1, y, width, height, depth);
            if (info){
                return info;
            }
        }
    }
    // check if we can go right
    if (x<width-1){
        if (map[get_pos(x+1, y, width)] == looking_for){
            info = continue_trail(looking_for+1, map, visited_map, x+1, y, width, height, depth);
            if (info){
                return info;
            }
        }
    }
    // check if we can go up
    if (y>0){
        if (map[get_pos(x, y-1, width)] == looking_for){
            info = continue_trail(looking_for+1, map, visited_map, x, y-1, width, height, depth);
            if (info){
                return info;
            }
        }
    }
    // check if we can go down
    if (y<height-1){
        if (map[get_pos(x, y+1, width)] == looking_for){
            info = continue_trail(looking_for+1, map, visited_map, x, y+1, width, height, depth);
            if (info){
                return info;
            }
        }
    }
    return info;
}

static uint64_t inline count_visited(uint32_t* visited_map, int width, int height){
    uint64_t count = 0;
    for (int i=0;i<width*height;i++){
        count+=visited_map[i];
    }
    return count;
}

static int inline start_trailhead(uint32_t* map, int x, int y, int width, int height, uint64_t* result){
    uint64_t depth = 0;
    uint32_t* visited_map = (uint32_t*)calloc(width*height, sizeof(uint32_t));
    if (!visited_map){
        return MALLOCERROR;
    }
    assert(visited_map[get_pos(x, y, width)] == 0);
    uint32_t looking_for = 1;
    int info = continue_trail(looking_for, map, visited_map, x, y, width, height, &depth);
    if (!info){
        *result += count_visited(visited_map, width, height);
    }
    free(visited_map);
    return info;
}

int get_result(const char* filename, uint64_t* result){

    int info = 0;
    *result = 0;
    int width, height=0;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    uint32_t* map = (uint32_t*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(uint32_t));
    if (!map){
        info = MALLOCERROR;
        goto free_map;
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
        width = strlen(line);
        if (width > MAXWIDTH){
            info = MAXWIDTHERROR;
            goto free_all;
        }
        parse_line(line, map+width*height, width);
        height++;
    }
    fclose(ptr);
    ptr = NULL;

    for (int x=0;x<width;x++){
        for (int y=0;y<height;y++){
            if (map[get_pos(x, y, width)] == 0){
                info = start_trailhead(map, x, y, width, height, result);
                if (info){
                    goto free_all;
                }
            }
        }
    }

free_all:
    if(ptr){
        fclose(ptr);    
    }

free_map:
    free(map);

free_line:
    free(line);

    print_info(info);
    return info;
}

int main(){

    int info;
    uint64_t result;
    info = get_result("test.txt", &result);
    // printf("The solution to the test is: %llu\n", result);
    assert(info == 0);
    assert(result == 81);

    info = get_result("input.txt", &result);
    printf("The solution to part 2 is: %llu\n", result);
    assert(info == 0);
    return info;
}