#include "Processor.h"
#include <iostream>
#include <string>
#include <iomanip>
#include "InstructionDecode.h"
#include "Register.h"


Processor::Processor()
{
	memory = new uint8_t[Processor::MEMORY_SIZE];
}

void Processor::Run(const uint32_t* rawInstructions, const uint64_t instructionCount)
{
	Instruction* instructions = DecodeInstructions(rawInstructions, instructionCount);

	//set stack pointer
	registers[2].dword = Processor::MEMORY_SIZE;

	while (true)
	{
		const uint64_t instructionIndex = pc / 4;
		if (instructionIndex >= instructionCount)
		{
			throw new std::runtime_error("Index out of bounds.\nTried to access instruction: " + std::to_string(instructionIndex));
		}

		const Instruction instruction = instructions[pc / 4];

		bool stopProgram = false;
		RunInstruction(instruction, &stopProgram);

		if (debugEnabled)
		{
			PrintRegisters();
			std::cin.get();
		}


		if (stopProgram)
		{
			break;
		}
	}
}

void Processor::PrintInstructions(const uint32_t* rawInstructions, const uint64_t instructionCount)
{
	Instruction* instructions = DecodeInstructions(rawInstructions, instructionCount);
	for (int i = 0; i < instructionCount; i++)
	{
		const Instruction instruction = instructions[i];
		std::cout << std::setw(32 + 6) << InstructionToBits(rawInstructions[i]) << "  " << InstructionAsString(instruction) << std::endl;
	}
}

void Processor::EnvironmentCall(bool* stopProgram)
{
	if (registers[10].dword == 10)
	{
		*stopProgram = true;
	}
}

void Processor::RunInstruction(const Instruction instruction, bool* stopProgram)
{
	int64_t x;
	switch (instruction.type)
	{
		case InstructionType::lb:
			registers[instruction.rd].dword = static_cast<int64_t>(static_cast<int8_t>(GetByteFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate))));
			pc += 4;
			break;
		case InstructionType::lh:
			registers[instruction.rd].dword = static_cast<int64_t>(static_cast<int16_t>(GetHalfWordFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate))));
			pc += 4;
			break;
		case InstructionType::lw:
			registers[instruction.rd].dword = static_cast<int64_t>(static_cast<int32_t>(GetWordFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate))));
			pc += 4;
			break;
		case InstructionType::ld:
			registers[instruction.rd].dword = static_cast<int64_t>(GetDoubleWordFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate)));
			pc += 4;
			break;
		case InstructionType::lbu:
			registers[instruction.rd].udword = static_cast<uint64_t>(GetByteFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate)));
			pc += 4;
			break;
		case InstructionType::lhu:
			registers[instruction.rd].udword = static_cast<uint64_t>(GetHalfWordFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate)));
			pc += 4;
			break;
		case InstructionType::lwu:
			registers[instruction.rd].udword = static_cast<uint64_t>(GetWordFromMemory(registers[instruction.rs1].dword + static_cast<int64_t>(instruction.immediate)));
			pc += 4;
			break;
		case InstructionType::fence:
		case InstructionType::fence_i:
			throw new std::runtime_error("Instruction not implemented yet.");
		case InstructionType::addi:
			x = static_cast<int64_t>(instruction.immediate);
			registers[instruction.rd].dword = registers[instruction.rs1].dword + x;
			//registers[instruction.rd].dword = registers[instruction.rs1].dword + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate));
			pc += 4;
			break;
		case InstructionType::slli:
			registers[instruction.rd].dword = registers[instruction.rs1].dword << static_cast<int64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::slti:
			registers[instruction.rd].dword = (registers[instruction.rs1].dword < static_cast<int64_t>(instruction.immediate)) ? 1 : 0;
			pc += 4;
			break;
		case InstructionType::sltiu:
			registers[instruction.rd].dword = (registers[instruction.rs1].udword < static_cast<uint64_t>(instruction.immediate)) ? 1 : 0;
			pc += 4;
			break;
		case InstructionType::xori:
			registers[instruction.rd].dword = registers[instruction.rs1].dword ^ static_cast<int64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::srli:
			registers[instruction.rd].udword = registers[instruction.rs1].udword >> static_cast<uint64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::srai:
			registers[instruction.rd].dword = registers[instruction.rs1].dword >> static_cast<int64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::ori:
			registers[instruction.rd].dword = registers[instruction.rs1].dword | static_cast<int64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::andi:
			registers[instruction.rd].dword = registers[instruction.rs1].dword & static_cast<int64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::auipc:
			registers[instruction.rd].udword = pc + static_cast<uint64_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::addiw:
			registers[instruction.rd].word = registers[instruction.rs1].word + static_cast<int32_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::slliw:
			registers[instruction.rd].word = registers[instruction.rs1].word << static_cast<int32_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::srliw:
			registers[instruction.rd].uword = registers[instruction.rs1].uword >> instruction.immediate;
			pc += 4;
			break;
		case InstructionType::sraiw:
			registers[instruction.rd].word = registers[instruction.rs1].word >> static_cast<int32_t>(instruction.immediate);
			pc += 4;
			break;
		case InstructionType::sb:
			StoreByteInMemory(registers[instruction.rs1].dword + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)), registers[instruction.rs2].byte);
			pc += 4;
			break;
		case InstructionType::sh:
			StoreHalfWordInMemory(registers[instruction.rs1].dword + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)), registers[instruction.rs2].half);
			pc += 4;
			break;
		case InstructionType::sw:
			StoreWordInMemory(registers[instruction.rs1].dword + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)), registers[instruction.rs2].word);
			pc += 4;
			break;
		case InstructionType::sd:
			StoreDoubleWordInMemory(registers[instruction.rs1].dword + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)), registers[instruction.rs2].dword);
			pc += 4;
			break;
		case InstructionType::add:
			registers[instruction.rd].dword = registers[instruction.rs1].dword + registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::sub:
			registers[instruction.rd].dword = registers[instruction.rs1].dword - registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::sll:
			registers[instruction.rd].dword = registers[instruction.rs1].dword << registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::slt:
			registers[instruction.rd].dword = (registers[instruction.rs1].dword < registers[instruction.rs2].dword) ? 1 : 0;
			pc += 4;
			break;
		case InstructionType::sltu:
			registers[instruction.rd].dword = (registers[instruction.rs1].udword < registers[instruction.rs2].udword) ? 1 : 0;
			pc += 4;
			break;
		case InstructionType::xor_:
			registers[instruction.rd].dword = registers[instruction.rs1].dword ^ registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::srl:
			registers[instruction.rd].udword = registers[instruction.rs1].udword >> registers[instruction.rs2].udword;
			pc += 4;
			break;
		case InstructionType::sra:
			registers[instruction.rd].dword = registers[instruction.rs1].dword >> registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::or_:
			registers[instruction.rd].dword = registers[instruction.rs1].dword | registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::and_:
			registers[instruction.rd].dword = registers[instruction.rs1].dword & registers[instruction.rs2].dword;
			pc += 4;
			break;
		case InstructionType::lui:
			x = static_cast<int64_t>(static_cast<int32_t>(instruction.immediate));
			registers[instruction.rd].dword = x;
			//registers[instruction.rd].dword = static_cast<int64_t>(static_cast<int32_t>(instruction.immediate));
			pc += 4;
			break;
		case InstructionType::addw:
			registers[instruction.rd].word = registers[instruction.rs1].word + registers[instruction.rs2].word;
			pc += 4;
			break;
		case InstructionType::subw:
			registers[instruction.rd].word = registers[instruction.rs1].word - registers[instruction.rs2].word;
			pc += 4;
			break;
		case InstructionType::sllw:
			registers[instruction.rd].word = registers[instruction.rs1].word << registers[instruction.rs2].word;
			pc += 4;
			break;
		case InstructionType::srlw:
			registers[instruction.rd].uword = registers[instruction.rs1].uword >> registers[instruction.rs2].uword;
			pc += 4;
			break;
		case InstructionType::sraw:
			registers[instruction.rd].word = registers[instruction.rs1].word >> registers[instruction.rs2].word;
			pc += 4;
			break;
		case InstructionType::beq:
			pc = (registers[instruction.rs1].dword ==  registers[instruction.rs2].dword)  ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::bne:
			pc = (registers[instruction.rs1].dword !=  registers[instruction.rs2].dword)  ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::blt:
			pc = (registers[instruction.rs1].dword <   registers[instruction.rs2].dword)  ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::bge:
			pc = (registers[instruction.rs1].dword >=  registers[instruction.rs2].dword)  ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::bltu:
			pc = (registers[instruction.rs1].udword <  registers[instruction.rs2].udword) ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::bgeu:
			pc = (registers[instruction.rs1].udword >= registers[instruction.rs2].udword) ? pc + static_cast<int64_t>(static_cast<int32_t>(instruction.immediate)) : pc + 4;
			break;
		case InstructionType::jalr: //(*) ask about 3
			registers[instruction.rd].dword = pc + 4;
			pc = registers[instruction.rs1].udword + static_cast<uint64_t>(instruction.immediate);
			break;
		case InstructionType::jal:
			registers[instruction.rd].dword = pc + 4;
			pc = pc + static_cast<uint64_t>(instruction.immediate << 1);
			break;
		case InstructionType::ecall:
			EnvironmentCall(stopProgram);
			pc += 4;
			break;
		case InstructionType::ebreak:
			PrintRegisters();
			std::cin.get();
			pc += 4;
			break;
		case InstructionType::csrrw:
		case InstructionType::csrrs:
		case InstructionType::csrrc:
		case InstructionType::csrrwi:
		case InstructionType::csrrsi:
		case InstructionType::csrrci:
			throw new std::runtime_error("Instruction not implemented yet.");
		default:
			throw std::runtime_error("instruction identifier not recognized. iid: " + NumberToBits(static_cast<uint32_t>(instruction.type)));
			break;
	}
}

bool Processor::CompareRegisters(const uint32_t* compareWith)
{
	for (uint32_t i = 0; i < 32; i++)
	{
		//(*)don't compare stack pointer because the correct is wrong
		if (registers[i].uword != compareWith[i] && i != 2)
		{
			return false;
		}
	}

	return true;
}

void Processor::SetDebugMode(const bool useDebugMode)
{
	debugEnabled = useDebugMode;
}

void Processor::PrintRegisters()
{
	std::cout << "Registers:" << std::endl;
	uint32_t index = 0;
	for(Register x : registers)
	{
		std::cout << RegisterName(index) << "  " << std::setw(19) << std::to_string(x.dword) << "  " << NumberToBits64(x.udword) << std::endl;
		index++;
	}
	std::cout << std::endl;
}

void Processor::VerifyMemorySpace(int64_t index, int64_t size)
{
	if (index < 0 || index + size > Processor::MEMORY_SIZE)
	{
		throw new std::runtime_error("Memory access out of range.\nTried to access memory address " + std::to_string(index));
	}
}

uint8_t Processor::GetByteFromMemory(int64_t index)
{
	VerifyMemorySpace(index, 1);

	return memory[index];
}
uint16_t Processor::GetHalfWordFromMemory(int64_t index)
{
	VerifyMemorySpace(index, 2);

	const uint16_t t1 = static_cast<uint16_t>(memory[index + 0]);
	const uint16_t t2 = static_cast<uint16_t>(memory[index + 1]);

	return (t1 << 8) |
		   (t2 << 0);
}
uint32_t Processor::GetWordFromMemory(int64_t index)
{
	VerifyMemorySpace(index, 4);

	const uint32_t t1 = static_cast<uint32_t>(memory[index + 0]);
	const uint32_t t2 = static_cast<uint32_t>(memory[index + 1]);
	const uint32_t t3 = static_cast<uint32_t>(memory[index + 2]);
	const uint32_t t4 = static_cast<uint32_t>(memory[index + 3]);

	return (t1 << 24) |
		   (t2 << 16) |
		   (t3 <<  8) |
		   (t4 <<  0);
}
uint64_t Processor::GetDoubleWordFromMemory(int64_t index)
{
	VerifyMemorySpace(index, 8);

	const uint64_t t1 = static_cast<uint64_t>(memory[index + 0]);
	const uint64_t t2 = static_cast<uint64_t>(memory[index + 1]);
	const uint64_t t3 = static_cast<uint64_t>(memory[index + 2]);
	const uint64_t t4 = static_cast<uint64_t>(memory[index + 3]);
	const uint64_t t5 = static_cast<uint64_t>(memory[index + 4]);
	const uint64_t t6 = static_cast<uint64_t>(memory[index + 5]);
	const uint64_t t7 = static_cast<uint64_t>(memory[index + 6]);
	const uint64_t t8 = static_cast<uint64_t>(memory[index + 7]);

	return (t1 << 56) |
		   (t2 << 48) |
		   (t3 << 40) |
		   (t4 << 32) |
		   (t5 << 24) |
		   (t6 << 16) |
		   (t7 <<  8) |
		   (t8 <<  0);
}

void Processor::StoreByteInMemory(int64_t index, int8_t byte)
{
	VerifyMemorySpace(index, 1);

	memory[index] = static_cast<uint8_t>(byte);
}
void Processor::StoreHalfWordInMemory(int64_t index, int16_t halfWord)
{
	VerifyMemorySpace(index, 2);

	memory[index + 0] = static_cast<uint8_t>(static_cast<uint16_t>(halfWord) >> 8);
	memory[index + 1] = static_cast<uint8_t>(static_cast<uint16_t>(halfWord) >> 0);
}
void Processor::StoreWordInMemory(int64_t index, int32_t word)
{
	VerifyMemorySpace(index, 4);

	memory[index + 0] = static_cast<uint8_t>(static_cast<uint32_t>(word) >> 24);
	memory[index + 1] = static_cast<uint8_t>(static_cast<uint32_t>(word) >> 16);
	memory[index + 2] = static_cast<uint8_t>(static_cast<uint32_t>(word) >>  8);
	memory[index + 3] = static_cast<uint8_t>(static_cast<uint32_t>(word) >>  0);
}
void Processor::StoreDoubleWordInMemory(int64_t index, int64_t doubleWord)
{
	VerifyMemorySpace(index, 8);

	memory[index + 0] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 56);
	memory[index + 1] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 48);
	memory[index + 2] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 40);
	memory[index + 3] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 32);
	memory[index + 4] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 24);
	memory[index + 5] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >> 16);
	memory[index + 6] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >>  8);
	memory[index + 7] = static_cast<uint8_t>(static_cast<uint64_t>(doubleWord) >>  0);
}


Processor::~Processor()
{
	delete[] memory;
}
