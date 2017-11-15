#include "InstructionDecode.h"
#include <cstdint>
#include <stdexcept>
#include "Instruction.h"
#include "InstructionFormat.h"

static uint32_t GetImmediateMask(uint16_t instructionIdentifier)
{
	const uint32_t removefunct7    = 0b00000000'00000000'00000000'00011111;
	const uint32_t wholeidentifier = 0b11111111'11111111'11111111'11111111;
	switch (instructionIdentifier)
	{
		case 0b000000'001'0010011:
		case 0b000000'101'0010011:
			return removefunct7;
		default:
			return wholeidentifier;
	}
}

static uint16_t GetInstructionIdentifierMask(uint16_t instructionIdentifier)
{
	const uint16_t onlyOpCodeAndFunct3 = 0b000000'111'1111111;
	const uint16_t wholeidentifier     = 0b111111'111'1111111;
	switch (instructionIdentifier & onlyOpCodeAndFunct3)
	{
		case 0b000000'001'0010011:
		case 0b000000'101'0010011:
		case 0b000000'000'0110011:
		case 0b000000'001'0110011:
		case 0b000000'010'0110011:
		case 0b000000'011'0110011:
		case 0b000000'100'0110011:
		case 0b000000'101'0110011:
		case 0b000000'110'0110011:
		case 0b000000'111'0110011:
		case 0b000000'000'1110011:
			return wholeidentifier;
		default:
			return onlyOpCodeAndFunct3;
	}
}

static InstructionType GetInstructionType(const uint32_t opcode, const uint32_t funct3, const uint32_t funct7OrImmediate)
{
	const uint16_t instructionIdentifier = static_cast<uint16_t>((opcode            <<  0) |
																 (funct3            <<  7) |
																 (funct7OrImmediate << 10));
	const uint16_t mask = GetInstructionIdentifierMask(instructionIdentifier);

	const uint16_t instructionType = instructionIdentifier & mask;
	return static_cast<InstructionType>(instructionType);
}

int32_t SignExtend_uint12_t(int32_t toExtend)
{
	return (toExtend << 20) >> 20;
}
static int32_t SignExtend_uint20_t(int32_t toExtend)
{
	return (toExtend << 12) >> 12;
}

static Instruction DecodeRType(const uint32_t rawInstruction)
{
	RType rType(rawInstruction);
	Instruction decoded;
	decoded.rd = rType.rd;
	decoded.rs1 = rType.rs1;
	decoded.rs2 = rType.rs2;
	decoded.type = GetInstructionType(rType.opcode, rType.funct3, rType.funct7);

	return decoded;
}

static Instruction DecodeIType(const uint32_t rawInstruction)
{
	const IType iType(rawInstruction);

	Instruction decoded = { 0 };
	decoded.rd = iType.rd;
	decoded.rs1 = iType.rs1;
	decoded.immediate = SignExtend_uint12_t(iType.immediate);
	decoded.type = GetInstructionType(iType.opcode, iType.funct3, iType.immediate >> 5);
	decoded.immediate &= GetImmediateMask((iType.funct3 << 7) | iType.opcode);

	return decoded;
}

static Instruction DecodeSType(const uint32_t rawInstruction)
{
	const SType sType(rawInstruction);

	Instruction decoded = { 0 };
	decoded.rs1 = sType.rs1;
	decoded.rs2 = sType.rs2;
	decoded.immediate = (sType.immediate2 << 5) |
		(sType.immediate1 << 0);
	decoded.immediate = SignExtend_uint12_t(decoded.immediate);
	decoded.type = GetInstructionType(sType.opcode, sType.funct3, 0);

	return decoded;
}

static Instruction DecodeSBType(const uint32_t rawInstruction)
{
	const SBType sbType(rawInstruction);

	Instruction decoded = { 0 };
	decoded.rs1 = sbType.rs1;
	decoded.rs2 = sbType.rs2;
	decoded.immediate = (sbType.immediate1 << 11) |
		(sbType.immediate2 <<  1) |
		(sbType.immediate3 <<  5) |
		(sbType.immediate4 << 12);
	decoded.immediate >>= 1;
	decoded.immediate = SignExtend_uint12_t(decoded.immediate);
	decoded.immediate <<= 1;
	decoded.type = GetInstructionType(sbType.opcode, sbType.funct3, 0);

	return decoded;
}

static Instruction DecodeUJType(const uint32_t rawInstruction)
{
	const UUJType ujType = { rawInstruction };

	Instruction decoded = { 0 };
	decoded.rd = ujType.type.rd;
	decoded.immediate = (ujType.type.immediate1 << 12) |
		(ujType.type.immediate2 << 11) |
		(ujType.type.immediate3 <<  1) |
		(ujType.type.immediate4 << 20);
	decoded.immediate = SignExtend_uint20_t(decoded.immediate);
	decoded.type = GetInstructionType(ujType.type.opcode, 0, 0);

	return decoded;
}

static Instruction DecodeUType(const uint32_t rawInstruction)
{
	const UUType uType = { rawInstruction };

	Instruction decoded = { 0 };
	decoded.rd = uType.type.rd;
	decoded.immediate = uType.type.immediate << 12;
	decoded.type = GetInstructionType(uType.type.opcode, 0, 0);

	return decoded;
}

Instruction DecodeInstruction(const uint32_t rawInstruction)
{
	//opcode is the first 7 bits
	const uint32_t opcode = rawInstruction & 127;

	switch (opcode)
	{
		case 0b00000011:
		case 0b00001111:
		case 0b00010011:
		case 0b01100111:
		case 0b01110011:
			return DecodeIType(rawInstruction);
		case 0b00010111:
		case 0b00110111:
			return DecodeUType(rawInstruction);
		case 0b00100011:
			return DecodeSType(rawInstruction);
		case 0b00110011:
			return DecodeRType(rawInstruction);
		case 0b01100011:
			return DecodeSBType(rawInstruction);
		case 0b01101111:
			return DecodeUJType(rawInstruction);
		default:
			throw std::runtime_error("Invalid opcode. opcode: " + std::to_string(opcode));
	}
}

const Instruction* DecodeInstructions(const uint32_t* rawInstructions, const uint32_t instructionsCount)
{
	Instruction* instructions = new Instruction[instructionsCount];

	for (uint32_t i = 0; i < instructionsCount; i++)
	{
		const uint32_t rawInstruction = rawInstructions[i];
		instructions[i] = DecodeInstruction(rawInstruction);
		//PrintInstruction(instructions[i]);
	}

	return instructions;
}

std::string GetProgramAsString(const uint32_t* rawInstructions, const uint32_t instructionCount)
{
	std::string program;
	for(uint32_t i = 0; i < instructionCount; i++)
	{
		const Instruction& instruction = DecodeInstruction(rawInstructions[i]);
		program += InstructionAsString(instruction) + "\n";
	}

	return program;
}