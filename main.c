#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct State8080 {
    uint8_t *reg;
    uint8_t *memory;
} State8080;

/* Register Map
    0 A
    1 F(flags)
    2 B
    3 C
    4 D
    5 E
    6 H
    7 L
    8-9 Stack Pointer
    10-11 Program Counter
*/

enum Register {
    AR = 0,
    FR = 1,
    BR = 2,
    CR = 3,
    DR = 4,
    ER = 5,
    HR = 6,
    LR = 7,
    SP = 8,
    PC = 10
} Register;

uint16_t get16Bit(uint8_t* byte) {
    uint16_t result;
    result = *(uint16_t*)byte;
    return result;
}

void UnimplementedInstruction(State8080* state, uint8_t* opcode) {
    printf("Error: Unimplemented instruct: %x\n", *opcode);
    exit(1);
    return;
}

int Emulate8080Op(State8080* state) {
    uint8_t *opcode = &state->memory[state->reg[PC]];

    uint8_t cycles = 4;
    uint16_t tempWord;

    switch(*opcode) {
        case 0x00:                              //NOP
            break;
        case 0x01:                              //LXI B,d16
            state->reg[CR] = opcode[1];
            state->reg[BR] = opcode[2];
            state->reg[PC] += 2;
            cycles = 10;
            break;
        case 0x02:                              //STAX B
            tempWord = get16Bit(&state->reg[BR]);
            state->memory[tempWord] = state->reg[AR];
            cycles = 7;
            break;
        case 0x03:                              //INX B
            state->reg[BR]++;
            state->reg[CR]++;
            cycles = 5;
            break;
        case 0x04:                              //INR B
            state->reg[BR]++;
            cycles = 5;
            break;
        case 0x05:                              //DCR B
            state->reg[BR]--;
            cycles = 5;
            break;
        case 0x06:                              //MVI B,d8
            state->reg[BR] = opcode[1];
            state->reg[PC]++;
            cycles = 7;
            break;
        case 0x08:                              //NOP
            break;
        case 0x09:                              //DAD B
            tempWord = get16Bit(&state->reg[HR]);
            tempWord += get16Bit(&state->reg[BR]);
            state->reg[HR] = tempWord;
            state->reg[LR] = tempWord >> 8;
            cycles = 10;
            break;
        case 0x0A:                              //LDAX B
            tempWord = get16Bit(&state->reg[BR]);
            state->reg[AR] = state->memory[tempWord];
            cycles = 7;
            break;
        case 0x0B:                              //DCX B
            state->reg[BR]--;
            state->reg[CR]--;
            cycles = 5;
            break;
        case 0x0C:                              //INR C
            state->reg[CR]++;
            cycles = 5;
            break;
        case 0x0D:                              //DCR C
            state->reg[BR]--;
            cycles = 5;
            break;
        case 0x0E:                              //MVI C,d8
            state->reg[CR] = opcode[1];
            state->reg[PC]++;
            cycles = 7;
            break;
        case 0x10:                              //NOP
            break;                  
        case 0x11:                              //LXI D,d16
            state->reg[ER] = opcode[1];
            state->reg[DR] = opcode[2];
            state->reg[PC] += 2;
            cycles = 10;
            break;
        case 0x12:                              //STAX D
            tempWord = get16Bit(&state->reg[BR]);
            state->memory[tempWord] = state->reg[AR];
            cycles = 7;
            break;
        case 0x76:                              //HLT
            printf("[HLT]");
            exit(1);
            cycles = 7;
            break;
        default:
            UnimplementedInstruction(state, opcode);
            break;
    }

    state->reg[PC]++;
    return 0;
}

void ReadFileIntoMemory(State8080* state, char* filename, uint32_t offset) { //TODO: Fix overflow bug
    FILE *f = fopen(filename, "rb");
    if (f==NULL) {
        printf("error: Failed to open &s\n", filename);
        exit(1);
    }

    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    uint8_t *buffer = &state->memory[offset];
    fread(buffer, fsize, 1 ,f);
    fclose(f);

    return;
}

void printProgram(State8080* state) {
    uint8_t opcode;

    printf("[Program Start]\n");
    do {
        opcode = state->memory[state->reg[PC]];
        printf("%x\n", opcode);

        state->reg[PC]++;
    } while (opcode != 0x76);
    printf("[Program End]\n\n");

    return;
}

void setReg(State8080* state) { //For debugging only
    state->reg[AR] = 0;
    state->reg[FR] = 0;
    state->reg[BR] = 1;
    state->reg[CR] = 1;
    state->reg[DR] = 0;
    state->reg[ER] = 0;
    state->reg[HR] = 1;
    state->reg[LR] = 1;
    state->reg[SP] = 0;
    state->reg[PC] = 0;
}

State8080* Init8080(void) {
    State8080* state = calloc(1, sizeof(State8080));
    state->reg = calloc(11, sizeof(char));
    state->memory = calloc(0x10000, sizeof(uint8_t)); //16k

    setReg(state);
    return state;
}

int main(int argc, char**argv) {
    int done = 0;

    State8080* state = Init8080();

    ReadFileIntoMemory(state, "test.dat", 0);
    printProgram(state);
    
    printf("[Running]\n");
    while (done == 0) {
        done = Emulate8080Op(state);
    }

    return 0;
}
