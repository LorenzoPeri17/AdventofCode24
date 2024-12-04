#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define FILENOTFOUND 1
#define MALLOCERROR -1
#define MAXLINERROR -2

#define MAXWIDTH  1000
#define MAXHEIGHT 1000

#define TOK_ELSE 0
#define TOK_X 1
#define TOK_M 2
#define TOK_A 3
#define TOK_S 4

#define XMASLEN 3

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        case MAXLINERROR:
            printf("Reached MAXLEN element\n");
            break;
        default:
            break;
    }
}

// Parse a line of the file
static void inline parse_line(char* line, int* table, uint32_t width){
    for (uint32_t i=0; i<width; i++){
        switch(line[i]){
            case 'X':
                table[i] = TOK_X;
                break;
            case 'M':
                table[i] = TOK_M;
                break;
            case 'A':
                table[i] = TOK_A;
                break;
            case 'S':
                table[i] = TOK_S;
                break;
            default:
                table[i] = TOK_ELSE;
                break;
        }
    }
}

// Count the number of occurrences of an X-MAS
// That is, this shape:
//                      M.S
//                      .A.
//                      M.S
// But the M and S can be in any order
// let us read the patter left to right
// so the above is MAS-MAS
static uint32_t inline find_X_mas_from_A(int* table, uint32_t width, uint32_t height, uint32_t x, uint32_t y){
    uint32_t count = 0;
    assert(table[y*width+x] == TOK_A);

    // first check that the A is not on the border
    if (x == 0 || x == width-1 || y == 0 || y == height-1){
        return 0;
    }

    // find MAS-MAS
    if (table[(y-1)*width+x-1] == TOK_M && table[(y+1)*width+x+1] == TOK_S && 
        table[(y+1)*width+x-1] == TOK_M && table[(y-1)*width+x+1] == TOK_S){
        count++;
    }

    // find SAM-MAS
    else
    if (table[(y-1)*width+x-1] == TOK_S && table[(y+1)*width+x+1] == TOK_M && 
        table[(y+1)*width+x-1] == TOK_M && table[(y-1)*width+x+1] == TOK_S){
        count++;
    }

    // find MAS-SAM
    else
    if (table[(y-1)*width+x-1] == TOK_M && table[(y+1)*width+x+1] == TOK_S && 
        table[(y+1)*width+x-1] == TOK_S && table[(y-1)*width+x+1] == TOK_M){
        count++;
    }

    // find SAM-SAM
    else
    if (table[(y-1)*width+x-1] == TOK_S && table[(y+1)*width+x+1] == TOK_M && 
        table[(y+1)*width+x-1] == TOK_S && table[(y-1)*width+x+1] == TOK_M){
        count++;
    }

    return count;
}

static void inline get_X_count(int* table, uint32_t width, uint32_t height, uint32_t* count){
    for (uint32_t i=0; i<height; i++){
        for (uint32_t j=0; j<width; j++){
            if (table[i*width+j] == TOK_A){
                *count += find_X_mas_from_A(table, width, height, j, i);
            }
        }
    }
}

int get_count(const char* filename, uint32_t* count){

    int info = 0;
    *count = 0;
    uint32_t width, height = 0;

    char* line = (char*)malloc(MAXWIDTH*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    int* table = (int*)malloc(MAXWIDTH*MAXHEIGHT*sizeof(int));
    if (!table){
        info = MALLOCERROR;
        goto free_table;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    while(fgets(line, MAXWIDTH, ptr)){
        width = strlen(line);
        // take the newline out
        if (line[width-1] == '\n'){
            line[width-1] = '\0';
            width--;
        }
        parse_line(line, table+height*width, width);
        height++;
        if (height == MAXHEIGHT){
            info = MAXLINERROR;
            goto free_all;
        }
    }

    get_X_count(table, width, height, count);
    
free_all:
free_table:
    free(table);

free_line:
    free(line);

    print_info(info);
    fclose(ptr);
    return info;
}

int main(){

    int info;
    uint32_t count;

    info = get_count("test.txt", &count);
    printf("Count: %d\n", count);

    assert(info == 0);
    assert(count == 9);

    info = get_count("input.txt", &count);
    printf("The solution to part 2 is: %d\n", count);
    assert(info == 0);
    return info;
}