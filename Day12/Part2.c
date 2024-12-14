#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define FILENOTFOUND   -1
#define MALLOCERROR    -2
#define MAXCHARERROR   -3
#define MAXHEIGHTERROR -4
#define MAXGROUPSERROR -5


#define MAXCHAR    10000
#define MAXWIDTH   MAXCHAR
#define MAXHEIGHT  50000
#define NGROUPS    1000

#define LEFT  1<<0
#define RIGHT 1<<1
#define UP    1<<2
#define DOWN  1<<3

#define VISITED_NONE 0
#define VISITED_ONCE 1<<5
#define VISITED_TWICE_UP    UP
#define VISITED_TWICE_DOWN  DOWN
#define VISITED_TWICE_LEFT  LEFT
#define VISITED_TWICE_RIGHT RIGHT

struct Group{
    char Letter;
    uint32_t sides;
    uint32_t area;
    char processed;
    int* positions;
};

struct Plot{
    char Letter;
    char visited;
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
        case MAXHEIGHTERROR:
            printf("MAXHEIGHT not enough\n");
            break;
        case MAXGROUPSERROR:
            printf("NGROUPS not enough\n");
            break;
        default:
            break;
    }
}

static int inline get_pos(int x, int y, int width){
    return x + y*width;
}

static void inline free_groups(struct Group* groups, uint32_t Ngroups){
    for (uint32_t i = 0; i<Ngroups; i++){
        if (groups[i].positions){
            free(groups[i].positions);
        }
    }
}


static void inline recurse_through_group(int width, int height, struct Plot* map, struct Group* group, int x, int y){

    assert(map[get_pos(x, y, width)].visited == VISITED_NONE);
    assert(map[get_pos(x, y, width)].Letter == group->Letter);

    map[get_pos(x, y, width)].visited = VISITED_ONCE;
    group->positions[group->area] = get_pos(x, y, width);
    group->area++;

    if (x > 0  && map[get_pos(x-1, y, width)].Letter == group->Letter && map[get_pos(x-1, y, width)].visited == VISITED_NONE){
        recurse_through_group(width, height, map, group, x-1, y);
    }
    if (x < width-1 && map[get_pos(x+1, y, width)].Letter == group->Letter && map[get_pos(x+1, y, width)].visited == VISITED_NONE){
        recurse_through_group(width, height, map, group, x+1, y);
    }
    if (y > 0 && map[get_pos(x, y-1, width)].Letter == group->Letter && map[get_pos(x, y-1, width)].visited == VISITED_NONE){
        recurse_through_group(width, height, map, group, x, y-1);
    }
    if (y < height-1 && map[get_pos(x, y+1, width)].Letter == group->Letter && map[get_pos(x, y+1, width)].visited == VISITED_NONE){
        recurse_through_group(width, height, map, group, x, y+1);
    }
    return;
}

static int inline start_new_group(struct Group* groups, uint32_t* Ngroups, struct Plot* map, int x, int y, const int width, const int height){
    struct Group* group = &groups[*Ngroups];
    group->Letter = map[get_pos(x, y, width)].Letter;
    group->sides = 0;
    group->area = 0;
    group->processed = 0;
    group->positions = (int*)malloc(width*height*sizeof(int));
    (*Ngroups)++;
    if (*Ngroups >= NGROUPS){
        return MAXGROUPSERROR;
    }
    if (!group->positions){
        return MALLOCERROR;
    }
    recurse_through_group(width, height, map, group, x, y);
    return 0;
}

static int inline process_groups(const int width, const int height, struct Plot* map, struct Group* groups, uint32_t* Ngroups){
    int info = 0;
    char curr;
    for (int y = 0; y<height; y++){
        for (int x = 0; x<width; x++){
            if (map[get_pos(x, y, width)].visited == VISITED_ONCE){
                continue;
            }
            info = start_new_group(groups, Ngroups, map, x, y, width, height);
            if (info){
                return info;
            }
        }
    }
    return info;
}

static char inline get_adjacent(const int width, const int height, const struct Plot* map, const int x, const int y, const char curr){
    char perimeter = 0;
    if (x == 0 || map[get_pos(x-1, y, width)].Letter != curr){
        perimeter |= LEFT;
    }
    if (x == width-1 || map[get_pos(x+1, y, width)].Letter != curr){
        perimeter |= RIGHT;
    }
    if (y == 0 || map[get_pos(x, y-1, width)].Letter != curr){
        perimeter |= UP;
    }
    if (y == height-1 || map[get_pos(x, y+1, width)].Letter != curr){
        perimeter |= DOWN;
    }
    return perimeter;
}

static void inline walk_horizontal(const int width, const int height, struct Plot* map, const int x, const int y, const char curr, const char direction){
    int i = x;
    while(i < width && map[get_pos(i, y, width)].Letter == curr && get_adjacent(width, height, map, i, y, curr) & direction){
        map[get_pos(i, y, width)].visited |= direction;
        i++;
    }
    i = x-1;
    while(i >= 0 && map[get_pos(i, y, width)].Letter == curr && get_adjacent(width, height, map, i, y, curr) & direction){
        map[get_pos(i, y, width)].visited |= direction;
        i--;
    }
    return;
}

static void inline walk_vertical(const int width, const int height, struct Plot* map, const int x, const int y, const char curr, const char direction){
    int i = y;
    while(i < height && map[get_pos(x, i, width)].Letter == curr && get_adjacent(width, height, map, x, i, curr) & direction){
        map[get_pos(x, i, width)].visited |= direction;
        i++;
    }
    i = y-1;
    while(i >= 0 && map[get_pos(x, i, width)].Letter == curr && get_adjacent(width, height, map, x, i, curr) & direction){
        map[get_pos(x, i, width)].visited |= direction;
        i--;
    }
    return;
}

static void inline walk(const int width, const int height, struct Plot* map, const int x, const int y, const char curr, const char direction){
    if (direction == LEFT || direction == RIGHT){
        walk_vertical(width, height, map, x, y, curr, direction);
    } else {
        walk_horizontal(width, height, map, x, y, curr, direction);
    }
    return;
}

static void inline find_sides(const int width, const int height, struct Plot* map, struct Group* group){
    int x, y;
    for (uint32_t i = 0; i<group->area; i++){
        x = group->positions[i] % width;
        y = group->positions[i] / width;
        char adjacents = get_adjacent(width, height, map, x, y, group->Letter);

        for (char j=0; j < 4; j++){
            char direction = 1<<j;
            char current = map[get_pos(x, y, width)].visited ;
            if (map[get_pos(x, y, width)].visited & direction){
                continue;
            } else if (adjacents & direction){
                walk(width, height, map, x, y, group->Letter, direction);
                group->sides++;
            }
        }
    }
}

int get_result(const char* filename, uint32_t* result){

    int info = 0;
    *result = 0;
    int width = 0, height = 0;
    uint32_t i;
    uint32_t Ngroups = 0;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    struct Plot* map = (struct Plot*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(struct Plot));
    if (!map){
        info = MALLOCERROR;
        goto free_map;
    }
    struct Group* groups = (struct Group*)malloc(NGROUPS*sizeof(struct Group));
    if (!groups){
        info = MALLOCERROR;
        goto free_groups;
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
        // copy the line to the map
        for (i = 0; i<width; i++){
            map[get_pos(i, height, width)].Letter = line[i];
            map[get_pos(i, height, width)].visited = 0;
        }
        height++;
        if (height > MAXHEIGHT){
            info = MAXHEIGHTERROR;
            goto free_all;
        }
    }
    fclose(ptr);
    ptr = NULL;

    process_groups(width, height, map, groups, &Ngroups);

    for (i = 0; i<Ngroups; i++){
        if (groups[i].processed){
            continue;
        }
        find_sides(width, height, map, &groups[i]);
        *result += groups[i].area * groups[i].sides;
        groups[i].processed = 1;
    }

free_all:
    if(ptr){
        fclose(ptr);
    }
    
free_groups:
    free_groups(groups, Ngroups);
    free(groups);

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

    info = get_result("test0.txt", &result);
    printf("The solution to test is: %u\n", result);
    assert(info == 0);
    assert(result == 80);

    info = get_result("test1.txt", &result);
    assert(info == 0);
    assert(result == 436);

    info = get_result("test2.txt", &result);
    assert(info == 0);
    assert(result == 236);  

    info = get_result("test3.txt", &result);
    assert(info == 0);
    assert(result == 368); 

    info = get_result("test.txt", &result);
    assert(info == 0);
    assert(result == 1206);

    info = get_result("input.txt", &result);
    printf("The solution to part 2 is: %u\n", result);
    assert(info == 0);
    return info;
}