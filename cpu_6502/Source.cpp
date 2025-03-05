#include <stdio.h>
#include <stdlib.h>
#include <iostream>


using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;


struct Mem
{
	static constexpr u32 MAX_MEM = 1024 * 64; // 64KB 
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

	void WriteWord(u32 Cycles, Word word, u32 Address)
	{
		Data[Address] = word & 0xFF;
		Data[Address + 1] = (word >> 8);
		Cycles -= 2;
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

	Byte Read(u32& Cycles, u32 Address, Mem& memory)
	{
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	// opcodes
	static constexpr Byte

		// Load accumulator instruction
		INS_LDA_IM = 0xA9, 
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_LDA_AB = 0xAD,
		INS_LDA_ABX = 0xBD,
		INS_LDA_ABY = 0xB9,
		INS_LDA_INDX = 0xA1,
		INS_LDA_INDY = 0xB1,

		// load x register
		INS_LDX_IM = 0xA2, 
		INS_LDX_ZP = 0xA6,
		INS_LDX_ZPY = 0xB6,
		INS_LDX_AB = 0xAE,
		INS_LDX_ABY = 0xBE,

		// load y register
		INS_LDY_IM = 0xA0, 
		INS_LDY_ZP = 0xA4,
		INS_LDY_ZPY = 0xB4,
		INS_LDY_AB = 0xAC,
		INS_LDY_ABY = 0xBC,

		INS_JSR_AB = 0x20, // JSR jump to subroutine

		INS_NOP = 0x80; // 2 byte illegal NOP

	void setLDAFlags()
	{
		Z = (A == 0); // set if a=0
		N = (N & 0b10000000) > 0;
	}

	void setLDXFlags()
	{
		Z = (X == 0);
		N = (X & 0b10000000) > 0;
	}

	void setLDYFlags()
	{
		Z = (Y == 0);
		N = (Y & 0b10000000) > 0;
	}

	void LoadImmediate(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte val = Fetch(Cycles, memo);
		Reg = val;
		Cycles--;
	}

	void LoadZeroPage(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte ZeroPage = Fetch(Cycles, memo);
		Byte ReadData = Read(Cycles, ZeroPage, memo);
		Cycles--;
		Reg = ReadData;
	}

	void LoadZeroPageX(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte ZeroPageAddress = Fetch(Cycles, memo);
		Byte NewAddress = ZeroPageAddress + X;
		Cycles--;
		Reg = Read(Cycles, NewAddress, memo);
	}

	void LoadZeroPageY(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte ZeroPageAddress = Fetch(Cycles, memo);
		Byte NewAddress = ZeroPageAddress + Y;
		Cycles--;
		Reg = Read(Cycles, NewAddress, memo);
	}

	void LoadAbsolute(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte LOW = Fetch(Cycles, memo);
		Byte HIGH = Fetch(Cycles, memo);
		Word Address = (HIGH << 8) | LOW;
		Cycles--;
		Reg = Read(Cycles, Address, memo); // 16 bit address, read from word
	}

	void LoadAbsoluteX(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte LOW = Fetch(Cycles, memo);
		Byte HIGH = Fetch(Cycles, memo);
		Word Address = ((HIGH << 8) | LOW) + X;
		Cycles--;
		Reg = Read(Cycles, Address, memo); // 16 bit address, read from word
	}

	void LoadAbsoluteY(Byte& Reg, u32& Cycles, Mem& memo)
	{
		Byte LOW = Fetch(Cycles, memo);
		Byte HIGH = Fetch(Cycles, memo);
		Word Address = ((HIGH << 8) | LOW) + Y;
		Cycles--;
		Reg = Read(Cycles, Address, memo); // 16 bit address, read from word
	}

	void Execute(u32 Cycles, Mem& memory) // each instruction has specific cycles
	{
		while (Cycles > 0)
		{
			// fetch instruction from PC
			Byte Ins = Fetch(Cycles, memory); // gets the opcode
			switch (Ins)
			{
			/*Load Accumulator*/
			case INS_LDA_IM:
			{
				LoadImmediate(A, Cycles, memory);
				setLDAFlags();
			} break;
			case INS_LDA_ZP:
			{
				LoadZeroPage(A, Cycles, memory);
				setLDAFlags();
			}break;
			case INS_LDA_ZPX:
			{
				LoadZeroPageX(A, Cycles, memory);
				setLDAFlags();
			} break;
			case INS_LDA_AB: // low byte then high byte, following instruction
			{
				LoadAbsolute(A, Cycles, memory);
				setLDAFlags();
			}break;
			case INS_LDA_ABX:
			{
				LoadAbsoluteX(A, Cycles, memory);
				setLDAFlags();
			} break;
			case INS_LDA_ABY:
			{
				LoadAbsoluteY(A, Cycles, memory);
				setLDAFlags();
			} break;
			case INS_LDA_INDX:
			{
				Word OffsetAddr = WFetch(Cycles, memory);
				Byte XOff = X + OffsetAddr;
				Cycles--;
				Byte L = Read(Cycles, XOff, memory);
				Cycles--;
				Byte H = Read(Cycles, XOff + 1, memory);
				Cycles--;
				Word Address = (H << 8) | L;
				Cycles--;
				A = Read(Cycles, Address, memory);
				setLDAFlags();
			} break;
			case INS_LDA_INDY:
			{
				Word OffsetAddr = WFetch(Cycles, memory);
				Byte XOff = Y + OffsetAddr;
				Cycles--;
				Byte L = Read(Cycles, XOff, memory);
				Cycles--;
				Byte H = Read(Cycles, XOff + 1, memory);
				Cycles--;
				Word Address = (H << 8) | L;
				Cycles--;
				A = Read(Cycles, Address, memory);
				setLDAFlags();
			} break; 

			/* Load X Register*/
			case INS_LDX_IM:
			{
				LoadImmediate(X, Cycles, memory);
				setLDXFlags();
			} break;
			case INS_LDX_ZP:
			{
				LoadZeroPage(X, Cycles, memory);
				setLDXFlags();
			} break;
			case INS_LDX_ZPY:
			{
				LoadZeroPageY(X, Cycles, memory);
				setLDXFlags();
			} break;
			case INS_LDX_AB:
			{
				LoadAbsolute(X, Cycles, memory);
				setLDXFlags();
			} break;
			case INS_LDX_ABY:
			{
				LoadAbsoluteY(X, Cycles, memory);
				setLDXFlags();
			} break;

			/*LOAD Y REG*/
			case INS_LDY_IM:
			{
				LoadImmediate(Y, Cycles, memory);
				setLDYFlags();
			} break;
			case INS_LDY_ZP:
			{
				LoadZeroPage(Y, Cycles, memory);
				setLDYFlags();
			} break;
			case INS_LDY_ZPY:
			{
				LoadZeroPageY(Y, Cycles, memory);
				setLDYFlags();
			} break;
			case INS_LDY_AB:
			{
				LoadAbsolute(Y, Cycles, memory);
				setLDYFlags();
			} break;
			case INS_LDY_ABY:
			{
				LoadAbsoluteY(Y, Cycles, memory);
				setLDYFlags();
			} break;
			/*NO OP*/
			case INS_NOP:
			{
				Cycles -= 2;
				PC += 2;
			} break;
			/* JUMP TO SUBROUTINE */
			case INS_JSR_AB:
			{
				Word JAddress = WFetch(Cycles, memory);
				memory.WriteWord(Cycles, PC - 1, SP++);
				PC = JAddress;
				Cycles--;
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