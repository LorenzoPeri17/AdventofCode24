#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND -1
#define MALLOCERROR  -2
#define MAXCHARERROR -3
#define BADCHAR      -4
#define BADGUARD     -5
#define RECURSIONERROR -6

#define ESCAPED       0

#define MAXCHAR    1000
#define MAXWIDTH   MAXCHAR
#define MAXHEIGHT  5000
#define MAXRECURSION  MAXWIDTH*MAXHEIGHT*100

enum Tokens{
    EMPTY,
    OBSTACLE,
    GUARD_UP,
    GUARD_DOWN,
    GUARD_LEFT,
    GUARD_RIGHT,
    VISITED,
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
        case BADCHAR:
            printf("Bad character in map\n");
            break;
        case BADGUARD:
            printf("Bad guard position\n");
            break;
        case RECURSIONERROR:
            printf("Recursion limit reached\n");
            break;
        default:
            break;
    }
}

static int inline parse_line(char* line, enum Tokens* map, int width){
    for (int i=0;i<width;i++){
        switch (line[i]){
            case '.':
                map[i] = EMPTY;
                break;
            case '#':
                map[i] = OBSTACLE;
                break;
            case '^':
                map[i] = GUARD_UP;
                break;
            default:
                return BADCHAR;
                break;
        }
    }
    return 0;
}

static int inline get_pos(int x, int y, int width){
    return x + y*width;
}

static int inline step_guard(enum Tokens* map, int width, int height, int* guard_pos_x, int* guard_pos_y, uint32_t* depth){
    if ((*depth)++ > MAXRECURSION){
        return RECURSIONERROR;
    }
    enum Tokens next;
    switch(map[get_pos(*guard_pos_x, *guard_pos_y, width)]){
        case GUARD_UP:
            if (*guard_pos_y == 0){
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                return ESCAPED;
            }
            next = map[get_pos(*guard_pos_x, (*guard_pos_y)-1, width)];
            if (next != OBSTACLE){
                // we move up
                map[get_pos(*guard_pos_x, (*guard_pos_y)-1, width)]=GUARD_UP;
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                (*guard_pos_y)--;
            } else {
                // it is an obstacle, rotate clockwise
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = GUARD_RIGHT;
            }
            return step_guard(map, height, width, guard_pos_x, guard_pos_y, depth);
            break;
        case GUARD_DOWN:
            if (*guard_pos_y == height-1){
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                return ESCAPED;
            }
            next = map[get_pos(*guard_pos_x, (*guard_pos_y)+1, width)];
            if (next != OBSTACLE){
                // we move down
                map[get_pos(*guard_pos_x, (*guard_pos_y)+1, width)]=GUARD_DOWN;
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                (*guard_pos_y)++;
            } else {
                // it is an obstacle, rotate clockwise
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = GUARD_LEFT;
            }
            return step_guard(map, height, width, guard_pos_x, guard_pos_y, depth);
            break;
        case GUARD_LEFT:
            if (*guard_pos_x == 0){
                    map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                    return ESCAPED;
            }
            next = map[get_pos((*guard_pos_x)-1, *guard_pos_y, width)];
            if (next != OBSTACLE){
                // we move left
                map[get_pos((*guard_pos_x)-1, *guard_pos_y, width)]=GUARD_LEFT;
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                (*guard_pos_x)--;
            } else {
                // it is an obstacle, rotate clockwise
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = GUARD_UP;
            }
            return step_guard(map, height, width, guard_pos_x, guard_pos_y, depth);
            break;
        case GUARD_RIGHT:
            if (*guard_pos_x == width-1){
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                return ESCAPED;
            }
            next = map[get_pos((*guard_pos_x)+1, *guard_pos_y, width)];
            if (next != OBSTACLE){
                // we move right
                map[get_pos((*guard_pos_x)+1, *guard_pos_y, width)]=GUARD_RIGHT;
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = VISITED;
                (*guard_pos_x)++;
            } else {
                // it is an obstacle, rotate clockwise
                map[get_pos(*guard_pos_x, *guard_pos_y, width)] = GUARD_DOWN;
            }
            return step_guard(map, height, width, guard_pos_x, guard_pos_y, depth);
            break;
        default:
            return BADGUARD;
            break;
    }
}

static int inline find_path(enum Tokens* map, int width, int height){
    uint32_t depth = 0;
    int guard_pos_x, guard_pos_y;
    for (int x=0; x<width; x++){
        for(int y=0; y<height; y++){
            if (map[get_pos(x, y, width)] == GUARD_UP){
                guard_pos_x = x;
                guard_pos_y = y;
                break;
            }
        }
    }
    return step_guard(map, width, height, &guard_pos_x, &guard_pos_y, &depth);
}

static uint32_t inline count_visited(enum Tokens* map, int width, int height){
    uint32_t count = 0;
    for (int x=0; x<width; x++){
        for(int y=0; y<height; y++){
            if (map[get_pos(x, y, width)] == VISITED){
                count++;
            }
        }
    }
    return count;
}

int get_result(const char* filename, uint32_t* result){

    int info = 0;
    *result = 0;
    int width, height=0;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    enum Tokens* map = (enum Tokens*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(enum Tokens));
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
        info = parse_line(line, map+width*height, width);
        if (info){
            goto free_all;
        }
        height++;
    }
    fclose(ptr);
    ptr = NULL;

    info = find_path(map, width, height);
    if (info){
        goto free_all;
    }
    *result = count_visited(map, width, height);

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
    uint32_t result;
    info = get_result("test.txt", &result);

    assert(info == 0);
    assert(result == 41);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %d\n", result);
    assert(info == 0);
    return info;
}