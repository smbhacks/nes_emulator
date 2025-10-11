#pragma once

enum AddressingMode
{
    none = 0, // implicit, 'a' regiszter, stb. (pl RTS)
    immediate, // (pl LDA #0)
    relative, // (pl BCS #1)
    zeropage, // (pl LDA $00)
    absolute, // (pl LDA $1000)
    indirect, // (pl JMP ($00))
    zeropage_x, // (pl LDA $00,x)
    zeropage_y, // (pl LDA $00,y)
    absolute_x, // (pl LDA $1000,x)
    absolute_y, // (pl LDA $1000,y)
    indexed_indirect, // (pl LDA ($00,x))
    indirect_indexed // (pl LDA ($00),y)
};