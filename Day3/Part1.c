#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define FILENOTFOUND -2
#define MALLOCERROR -1
#define EOFFOUND 1

#define STARTCHAR 20000

void print_info(int info){
    switch (info){
        case FILENOTFOUND:
            printf("File not found\n");
            break;
        case MALLOCERROR:
            printf("Error allocating array\n");
            break;
        default:
            break;
    }
}

// read a line until newline character or EOF
static int inline get_line(char* line, uint32_t* len, FILE* ptr){
    char c;
    *len = 0;
    uint32_t maxlen = STARTCHAR;
    while((c = fgetc(ptr)) != '\n' && c != EOF){
        line[*len] = c;
        (*len)++;
        if (*len >= STARTCHAR){
            // I guess we can realloc here
            maxlen = 2 * STARTCHAR;
            line = (char*)realloc(line, 2*maxlen*sizeof(char));
            if (!line){
                return MALLOCERROR;
            }
        }
    }
    line[*len] = '\0';
    (*len)++;
    if (c == EOF){
        return EOFFOUND;
    }
    return 0;
}

// Parse a line to look for the pattern
//  mul(\d{1,3},\d{1,3})
static void inline parse_line(char* line, uint32_t len, uint32_t* result){
    int i, j, startnum;
    int num1, num2;
    for(i = 0; i < len-3; i++){
        if (line[i] == 'm' && line[i+1] == 'u' && line[i+2] == 'l'){
            i+=3;
            if (line[i] == '('){
                // We found the pattern
                // Now we need to find the numbers
                startnum = ++i;
                num1 = 0;
                for (j=0; j<3; j++){
                    if(!isdigit(line[startnum+j])){
                        break;
                    }
                    num1 = num1*10 + (line[startnum+j] - '0');
                }
                if (line[startnum+j] != ',' && num1 !=0){
                    continue;
                }
                i+=j+1;
                startnum = startnum+j+1;
                num2 = 0;
                for (j=0; j<3; j++){
                    if(!isdigit(line[startnum+j])){
                        break;
                    }
                    num2 = num2*10 + (line[startnum+j] - '0');
                }
                if (line[startnum+j] == ')' && num2 !=0){
                    *result += num1*num2;
                }
            }
        }
    }
}

int get_result(const char* filename, uint32_t* result){
    int info = 0;
    uint32_t len;
    *result = 0;

    char * line = (char*)malloc(STARTCHAR*sizeof(char));
    if (!line){
        info = MALLOCERROR;
        goto free_line;
    }
    FILE* ptr = fopen(filename, "r");
    if (!ptr) {
        info = FILENOTFOUND;
        goto free_all;
    }

    while((info = get_line(line, &len, ptr))>=0){
        parse_line(line, len, result);
        if (info == EOFFOUND){
            break;
        }
    }

free_all:
free_line:
    free(line);

    fclose(ptr);
    print_info(info);
    return info;
}

int main(){

    int info;
    uint32_t result;

    info = get_result("test.txt", &result);
    printf("Result: %d\n", result);

    assert(info == EOFFOUND);
    assert(result == 161);

    info = get_result("input.txt", &result);
    printf("The solution to part 1 is: %d\n", result);
    assert(info == EOFFOUND);
    return (info == EOFFOUND);
}