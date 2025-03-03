#include <stdio.h>
#include <stdlib.h>


using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;


struct Mem
{
	static constexpr u32 MAX_MEM = 1024 * 64; // ?
	Byte Data[MAX_MEM]; // ?

	void Initialize()
	{
		// zero all values in memory
		for (u32 i = 0; i < MAX_MEM; i++)
		{
			Data[i] = 0;
		}
	}

	Byte operator[](u32 Address) const 
	{
		// assert that its within memory space

		return Data[Address];
	}

	Byte& operator[](u32 Address)
	{
		return Data[Address];
	}

};

struct CPU
{

	Word PC; // program counter
	Word SP; // stack pointer

	Byte A, X, Y; //registers

	Byte C : 1; // status flag
	Byte Z : 1; // status flag
	Byte I : 1; // status flag
	Byte D : 1; // status flag
	Byte B : 1; // status flag
	Byte V : 1; // status flag
	Byte N : 1; // status flag


	void Reset(Mem& memory)
	{
		PC = 0xFFFC; // initializes to reset vector address
		D = C = Z = I = B = V = N = 0; // clear flags
		SP = 0x0100; // init stack pointer to first page
		A = X = Y = 0;
		memory.Initialize();
	}

	Byte Fetch(u32& Cycles, Mem& memory)
	{
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}

	Word WFetch(u32& Cycles, Mem& memory)
	{
		Byte H = memory[PC];
		PC++;
		Byte L = memory[PC];
		PC++;
		Cycles += 2;
		return (H << 8) | L;
	}

	Byte Read(u32& Cycles, Byte Address, Mem& memory)
	{
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}
	// opcodes
	static constexpr Byte

		INS_LDA_IM = 0xA9, // Load accumulator instruction
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_LDA_AB = 0xAD,
		INS_LDA_ABX = 0xBD,
		INS_LDA_ABY = 0xB9,
		INS_LDA_INDX = 0xA1,
		INS_LDA_INDY = 0xB1,

		INS_JSR_AB = 0x20; // JSR jump to subroutine

	void setLDAFlags()
	{
		Z = (A == 0); // set if a=0
		N = (N & 0b10000000) > 0;
	}

	void Execute(u32 Cycles, Mem& memory) // each instruction has specific cycles
	{
		while (Cycles > 0)
		{
			// fetch instruction from PC
			Byte Ins = Fetch(Cycles, memory); // gets the opcode
			switch (Ins)
			{
			case INS_LDA_IM:
			{
				Byte value = Fetch(Cycles, memory);
				A = value;
				setLDAFlags();
			} break;
			case INS_LDA_ZP:
			{
				Byte ZeroPageAddress = Fetch(Cycles, memory);
				Byte ReadData = Read(Cycles, ZeroPageAddress, memory);
				A = ReadData;
				setLDAFlags();
			}break;
			case INS_LDA_ZPX:
			{
				Byte ZeroPageAddress = Fetch(Cycles, memory);
				Byte NewAddress = ZeroPageAddress + X;
				Cycles--;
				A = Read(Cycles, NewAddress, memory);
				setLDAFlags();
			} break;
			case INS_LDA_AB: // low byte then high byte, following instruction
			{
				Byte LOW = Fetch(Cycles, memory);
				Byte HIGH = Fetch(Cycles, memory);
				Byte Address = (HIGH << 8) | LOW;
				Cycles--;
				A = Read(Cycles, Address, memory);
				setLDAFlags();
			}break;
			case INS_LDA_ABX:
			{
				Byte LOW = Fetch(Cycles, memory);
				Byte HIGH = Fetch(Cycles, memory);
				Byte Address = ((HIGH << 8) | LOW) + X;
				Cycles--;
				A = Read(Cycles, Address, memory);
				setLDAFlags();
			} break;
			case INS_LDA_ABY:
			{
				Byte LOW = Fetch(Cycles, memory);
				Byte HIGH = Fetch(Cycles, memory);
				Byte Address = ((HIGH << 8) | LOW) + Y;
				Cycles--;
				A = Read(Cycles, Address, memory);
				setLDAFlags();
			} break;
			case INS_JSR_AB:
			{

			} break;
			default:
			{
				printf("Instruction not handled %d", Ins);
			}
			}
		}
	}

};

int main() 
{
	Mem mem;
	CPU cpu;
	cpu.Reset(mem);
	// start
	mem[0xFFFC] = CPU::INS_LDA_IM; // why not cpu.INS_LDA_IM
	printf("Loaded instruction into 0xFFFC");
	mem[0xFFFD] = 0x42;
	printf("0x42 immediate");
	mem[0x0042] = 0x84;
	printf("Loaded 0x84 into immediate address");
	//end
	cpu.Execute(3,mem); // pass clock cycles
	return 0;
}