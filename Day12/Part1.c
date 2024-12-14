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

struct Group{
    char Letter;
    uint32_t perimeter;
    uint32_t area;
    char processed;
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

static void inline recurse_through_group(int width, int height, struct Plot* map, struct Group* group, int x, int y){

    assert(map[get_pos(x, y, width)].visited == 0);
    assert(map[get_pos(x, y, width)].Letter == group->Letter);

    map[get_pos(x, y, width)].visited = 1;
    group->area++;

    if (x > 0  && map[get_pos(x-1, y, width)].Letter == group->Letter){
        if (map[get_pos(x-1, y, width)].visited == 0){
            recurse_through_group(width, height, map, group, x-1, y);
        }
    } else {
        group->perimeter++;
    }
    if (x < width-1 && map[get_pos(x+1, y, width)].Letter == group->Letter){
        if (map[get_pos(x+1, y, width)].visited == 0){
            recurse_through_group(width, height, map, group, x+1, y);
        }
    } else {
        group->perimeter++;
    }
    if (y > 0 && map[get_pos(x, y-1, width)].Letter == group->Letter){
        if (map[get_pos(x, y-1, width)].visited == 0){
            recurse_through_group(width, height, map, group, x, y-1);
        }
    } else {
        group->perimeter++;
    }
    if (y < height-1 && map[get_pos(x, y+1, width)].Letter == group->Letter){
        if (map[get_pos(x, y+1, width)].visited == 0){
            recurse_through_group(width, height, map, group, x, y+1);
        }
    } else {
        group->perimeter++;
    }
    return;
}

static int inline start_new_group(struct Group* groups, uint32_t* Ngroups, struct Plot* map, int x, int y, int width, int height){
    struct Group* group = &groups[*Ngroups];
    group->Letter = map[get_pos(x, y, width)].Letter;
    group->perimeter = 0;
    group->area = 0;
    group->processed = 0;
    (*Ngroups)++;
    if (*Ngroups >= NGROUPS){
        return MAXGROUPSERROR;
    }
    recurse_through_group(width, height, map, group, x, y);
    return 0;
}

static int inline process_groups(const int width, const int height, struct Plot* map, struct Group* groups, uint32_t* Ngroups){
    int info = 0;
    char curr;
    for (int y = 0; y<height; y++){
        for (int x = 0; x<width; x++){
            if (map[get_pos(x, y, width)].visited){
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
        *result += groups[i].area * groups[i].perimeter;
        groups[i].processed = 1;
    }

free_all:
    if(ptr){
        fclose(ptr);
    }
    
free_groups:
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
    assert(info == 0);
    assert(result == 140);

    info = get_result("test1.txt", &result);
    assert(info == 0);
    assert(result == 772);

    info = get_result("test.txt", &result);
    assert(info == 0);
    assert(result == 1930);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %u\n", result);
    assert(info == 0);
    return info;
}