#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define FILENOTFOUND -1
#define MALLOCERROR  -2
#define MAXCHARERROR -3
#define MAXHEIGHTERROR -4

#define MAXCHAR    1000
#define MAXWIDTH   MAXCHAR
#define MAXHEIGHT  5000

enum Token{
    EMPTY,
    ANTINODE
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
        default:
            break;
    }
}

static int inline get_pos(int x, int y, int width){
    return x + y*width;
}

static int inline min(int x, int y){
    return x < y ? x : y;
}

static int inline max(int x, int y){
    return x > y ? x : y;
}


static void inline count_antinodes_per_pair(int x, int y, int x1, int y1, int width, int height, enum Token* antinodes_map){
    int dx, dy;
    dx = abs(x1 - x);
    dy = abs(y1 - y);
    int linex, liney;

    if (dx != 0){
        double slope = (double)(y1 - y)/(double)(x1 - x);
        if(min(x, x1) - dx >= 0){
            linex = min(x, x1) - dx;
            liney = x < x1 ? y - slope * dx : y1 - slope * dx;
            antinodes_map[get_pos(linex , liney, width)] = ANTINODE;
        }
        if(max(x, x1) + dx < width){
            linex = max(x, x1) + dx;
            liney = x > x1 ? y + slope * dx : y1 + slope * dx;
            antinodes_map[get_pos(linex, liney, width)] = ANTINODE;
        }
    } else  { // vertical line
        if(min(y, y1) - dy >= 0){
            antinodes_map[get_pos(x, min(y, y1) - dy, width)] = ANTINODE;
        }
        if(max(y, y1) + dy < height){
            antinodes_map[get_pos(x, max(y, y1) + dy, width)] = ANTINODE;
        }
    }

    return;
}

static void inline find_antinodes_per_type(char antenna_type, char* map, int width, int height, enum Token* antinodes_map){
    int x, y, x1, y1;
    for (y = 0; y<height;y++){
        for (x = 0; x<width;x++){
            if (map[get_pos(x, y, width)] == antenna_type){
                for (y1=y; y1<height;y1++){
                    for (x1=0; x1<width;x1++){
                        if (y==y1 && x1<= x){
                            continue;
                        }
                        if (map[get_pos(x1, y1, width)] == antenna_type){
                            count_antinodes_per_pair(x, y, x1, y1, width, height, antinodes_map);
                        }
                    }
                }
            }
        }
    }
    return;
}


static uint32_t inline count_antinodes(char* map, int width, int height, char* antenna_types, uint32_t Nantenna_types, enum Token* antinodes_map){
    uint32_t count = 0;
    for (int i = 0; i < Nantenna_types; i++){
        find_antinodes_per_type(antenna_types[i], map, width, height, antinodes_map);
    }
    for (int i = 0; i < width*height; i++){
        if (antinodes_map[i] == ANTINODE){
            count++;
        }
    }
    return count;
}

int get_result(const char* filename, uint32_t* result){

    int info = 0;
    *result = 0;
    int width = 0, height = 0;
    uint32_t Nantenna_types = 0;
    uint32_t i, j;
    char curr, found;

    char* line = (char*)malloc(MAXCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    char* map = (char*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(char));
    if (!map){
        info = MALLOCERROR;
        goto free_map;
    }
    char* antenna_types = (char*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(char));
    if (!antenna_types){
        info = MALLOCERROR;
        goto free_antenna_types;
    }
    enum Token* antinodes_map = (enum Token*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(enum Token));
    if (!antinodes_map){
        info = MALLOCERROR;
        goto free_antinodes_map;
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
        for (i=0; i<width-1;i++){
            curr = line[i];
            found = 0;
            if((curr != '.') && (curr != '\0')){
                for (j=0; j<Nantenna_types;j++){
                    if(antenna_types[j] == curr){
                        found = 1;
                        break;
                    }
                }
                if (!found){
                    antenna_types[Nantenna_types] = curr;
                    Nantenna_types++;
                }
            }
        }
        // copy the line to the map
        memccpy(map+height*width, line, '\0', width);
        height++;
        if (height > MAXHEIGHT){
            info = MAXHEIGHTERROR;
            goto free_all;
        }
    }
    fclose(ptr);
    ptr = NULL;

    // initialize antinodes_map
    for (i=0; i<width*height;i++){
        antinodes_map[i] = EMPTY;
    }

    *result = count_antinodes(map, width, height, antenna_types, Nantenna_types, antinodes_map);

    // // print the map
    // for (i=0; i<height;i++){
    //     for (j=0; j<width;j++){
    //         printf("%c", map[get_pos(j, i, width)]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
    // // print the antinodes_map  
    // for (i=0; i<height;i++){
    //     for (j=0; j<width;j++){
    //         if (antinodes_map[get_pos(j, i, width)] == ANTINODE){
    //             printf("#");
    //         } else {
    //             printf(".");
    //         }
    //     }
    //     printf("\n");
    // }

free_all:
    if(ptr){
        fclose(ptr);
    }

free_antinodes_map:
    free(antinodes_map);

free_antenna_types:
    free(antenna_types);
    
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
    assert(result == 14);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %u\n", result);
    assert(info == 0);
    return info;
}