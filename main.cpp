#include <bits/stdc++.h>
using namespace std;

class Register
{
private:
    vector<string> registers;

public:
    Register(vector<string> reg)
    {
        registers = reg;
    }

    string readRegister(int Index)
    {
        if (Index >= 16 && Index < 0)
            return "";
        return registers[Index];
    }

    void writeRegister(int Index, string value)
    {

        registers[Index] = value;
    }
    vector<string> getRegister();
};

vector<string> Register:: getRegister()
{
    return registers;
}

enum class InstructionType
{
    ARITHMETIC,
    LOGICAL,
    SHIFT,
    MEMORY,
    CONTROL,
    HALT,
    NOP
};

enum class op_code
{
    ADD,
    SUB,
    MUL,
    INC,
    AND,
    OR,
    XOR,
    NOT,
    SLLI,
    SRLI,
    LI,
    LD,
    ST,
    JMP,
    BEQZ,
    HLT
};

class Instruction
{
public:
    InstructionType type;
    op_code opcode;
    int rd;
    int rs1;
    int rs2;
    int imm4;
    int imm8;
    bool raw_hazard;
    bool isStalled;

    Instruction() : isStalled(false), type(InstructionType ::NOP), raw_hazard(false), rd(-1), rs1(-1), rs2(-1), imm4(-1), imm8(-1) {}
};

string hexToBinary(char hexDigit)
{
    switch (hexDigit)
    {
    case '0':
        return "0000";
    case '1':
        return "0001";
    case '2':
        return "0010";
    case '3':
        return "0011";
    case '4':
        return "0100";
    case '5':
        return "0101";
    case '6':
        return "0110";
    case '7':
        return "0111";
    case '8':
        return "1000";
    case '9':
        return "1001";
    case 'a':
        return "1010";
    case 'b':
        return "1011";
    case 'c':
        return "1100";
    case 'd':
        return "1101";
    case 'e':
        return "1110";
    case 'f':
        return "1111";
    default:
        return "";
    }
}

int signedBinaryToInt(string bin)
{
    int decimal = 0;
    int n = bin.length();
    int sign = (bin[0] == '1') ? -1 : 1;

    for (int i = n - 1; i >= 0; i--)
    {
        if (bin[i] == '1')
        {
            decimal += sign * pow(2, n - 1 - i);
        }
    }
    if (decimal < 0)
    {
        decimal += 256;
        decimal = -decimal;
    }
    return decimal;
}

string decToHexa(int n)
{
    string ans = "";
    if (n < 0)
        n += 256;
    while (n != 0)
    {
        int rem = 0;
        char ch;
        rem = n % 16;

        if (rem < 10)
        {
            ch = rem + 48;
        }
        else
        {
            ch = rem + 55;
        }
        ans = ch + ans;
        n = n / 16;
    }

    if (ans == "")
        ans = "00";
    else if (ans.size() == 1)
        ans = "0" + ans;
    ans[0] = tolower(ans[0]);
    ans[1] = tolower(ans[1]);
    return ans;
}

int binaryToInt(string binary)
{
    int decimal = 0;
    int power = 0;
    for (int i = binary.length() - 1; i >= 0; --i)
    {
        if (binary[i] == '1')
        {
            decimal += pow(2, power);
        }
        power++;
    }
    return decimal;
}

class Program : public Register
{
    vector<string> instructions;
    vector<string> data;
    int PC;
    string A, B;
    string LMD;
    int cycles = 0;
    int Stalls = 0;
    int ALUOutput;
    Instruction IR;

    // output variables
    int arithmatic_inst;
    int logical_inst;
    int shift_inst;
    int memory_inst;
    int control_inst;
    int halt_inst;
    int data_stalls;
    int control_stalls;
    int loadIM_inst;

public:
    Program(vector<string> ins, vector<string> dta, vector<string> reg) : Register(reg)
    {
        instructions = ins;
        data = dta;
        PC = 0;
        cycles = 0;
        LMD = "";
        arithmatic_inst = 0;
        logical_inst = 0;
        shift_inst = 0;
        memory_inst = 0;
        control_inst = 0;
        halt_inst = 0;
        data_stalls = 0;
        control_stalls = 0;
        loadIM_inst = 0;
    }

    Instruction fetchInstruction(int pc);
    Instruction decodeInstruction(const Instruction &inst);
    Instruction executeInstruction(const Instruction &inst);
    Instruction memOperation(const Instruction &inst);
    Instruction writeBack(const Instruction &inst);
    void Pipelining();
    vector<string> getData();
    void PrintResult(const string& file);
};

vector<string> Program::getData()
{
    return data;
}

Instruction Program::fetchInstruction(int pc)
{
    pair<string, string> ins;
    ins.first = instructions[pc];
    ins.second = instructions[pc + 1];
    PC += 2;
    string a, b;
    a = ins.first;
    b = ins.second;
    IR.opcode = op_code(binaryToInt(hexToBinary(a[0])));

    if (IR.opcode == op_code(0) || IR.opcode == op_code(1) || IR.opcode == op_code(2) || IR.opcode == op_code(3))
    {
        IR.type = InstructionType::ARITHMETIC;
        IR.rd = binaryToInt(hexToBinary(a[1]));
        IR.rs1 = binaryToInt(hexToBinary(b[0]));
        IR.rs2 = binaryToInt(hexToBinary(b[1]));
    }
    else if (IR.opcode == op_code(4) || IR.opcode == op_code(5) || IR.opcode == op_code(6) || IR.opcode == op_code(7))
    {
        IR.type = InstructionType::LOGICAL;
        IR.rd = binaryToInt(hexToBinary(a[1]));
        IR.rs1 = binaryToInt(hexToBinary(b[0]));
        IR.rs2 = binaryToInt(hexToBinary(b[1]));
    }
    else if (IR.opcode == op_code(8) || IR.opcode == op_code(9))
    {
        IR.type = InstructionType::SHIFT;
        IR.rd = binaryToInt(hexToBinary(a[1]));
        IR.rs1 = binaryToInt(hexToBinary(b[0]));
        IR.imm4 = signedBinaryToInt(hexToBinary(b[1]));
    }
    else if (IR.opcode == op_code(10))
    {
        loadIM_inst++;
        IR.type = InstructionType::MEMORY;
        IR.rd = binaryToInt(hexToBinary(a[1]));
        IR.imm8 = signedBinaryToInt(hexToBinary(b[0]) + hexToBinary(b[1]));
    }
    else if (IR.opcode == op_code(11) || IR.opcode == op_code(12))
    {
        IR.type = InstructionType::MEMORY;
        IR.rd = binaryToInt(hexToBinary(a[1]));
        IR.rs1 = binaryToInt(hexToBinary(b[0]));
        IR.imm4 = signedBinaryToInt(hexToBinary(b[1]));
    }

    else if (IR.opcode == op_code(13))
    {
        IR.type = InstructionType::CONTROL;
        IR.imm8 = signedBinaryToInt(hexToBinary(a[1]) + hexToBinary(b[0]));
    }
    else if (IR.opcode == op_code(14))
    {
        IR.type = InstructionType::CONTROL;
        IR.rs1 = binaryToInt(hexToBinary(a[1]));
        IR.imm8 = signedBinaryToInt(hexToBinary(b[0]) + hexToBinary(b[1]));
    }
    else if (IR.opcode == op_code(15))
    {
        IR.type = InstructionType::HALT;
    }
    return IR;
}

Instruction Program::decodeInstruction(const Instruction &inst)
{
    if (inst.rs1 != -1)
        A = readRegister(IR.rs1);
    if (inst.rs2 != -1)
        B = readRegister(IR.rs2);
    return inst;
}

Instruction Program ::executeInstruction(const Instruction &inst)
{
    ALUOutput = 0;
    string hex_rd = readRegister(inst.rd);
    int rd_value = binaryToInt(hexToBinary(hex_rd[0]) + hexToBinary(hex_rd[1]));
    switch (inst.type)
    {
    case InstructionType::ARITHMETIC:
    {
        arithmatic_inst++;
        int rs1_value = signedBinaryToInt(hexToBinary(A[0]) + hexToBinary(A[1]));
        int rs2_value = signedBinaryToInt(hexToBinary(B[0]) + hexToBinary(B[1]));
        if (inst.opcode == op_code::ADD)
        {
            ALUOutput = rs1_value + rs2_value;
        }
        else if (inst.opcode == op_code::SUB)
        {
            ALUOutput = rs1_value - rs2_value;
        }
        else if (inst.opcode == op_code::MUL)
        {
            ALUOutput = rs1_value * rs2_value;
        }
        else if (inst.opcode == op_code::INC)
        {
            rd_value++;
            ALUOutput = rd_value;
        }
        break;
    }
    case InstructionType::LOGICAL:
    {
        logical_inst++;
        int rs1_value = signedBinaryToInt(hexToBinary(A[0]) + hexToBinary(A[1]));
        int rs2_value = signedBinaryToInt(hexToBinary(B[0]) + hexToBinary(B[0]));

        if (inst.opcode == op_code::AND)
        {
            ALUOutput = rs1_value & rs2_value;
        }
        else if (inst.opcode == op_code::OR)
        {
            ALUOutput = rs1_value | rs2_value;
        }
        else if (inst.opcode == op_code::XOR)
        {
            ALUOutput = rs1_value ^ rs2_value;
        }
        else if (inst.opcode == op_code::NOT)
        {
            ALUOutput = ~rs1_value;
        }
        break;
    }
    case InstructionType::SHIFT:
    {
        shift_inst++;
        int rs1_value = signedBinaryToInt(hexToBinary(A[0]) + hexToBinary(A[1]));
        int rs2_value = signedBinaryToInt(hexToBinary(B[0]) + hexToBinary(B[0]));
        if (inst.opcode == op_code::SLLI)
        {
            ALUOutput = (rs1_value << inst.imm4);
        }
        else if (inst.opcode == op_code::SRLI)
        {
            ALUOutput = (rs1_value >> inst.imm4);
        }
        break;
    }
    case InstructionType::MEMORY:
    {
        int rs1_value = signedBinaryToInt(hexToBinary(A[0]) + hexToBinary(A[1]));
        memory_inst++;
        if (inst.opcode == op_code::LI)
        {
            ALUOutput = inst.imm8;
        }
        else if (inst.opcode == op_code::LD)
        {
            ALUOutput = rs1_value + inst.imm4;
        }
        else if (inst.opcode == op_code::ST)
        {
            ALUOutput = rs1_value + inst.imm4;
        }
        break;
    }
    case InstructionType::CONTROL:
    {
        control_inst++;
        int rs1_value = signedBinaryToInt(hexToBinary(A[0]) + hexToBinary(A[1]));
        int rs2_value = signedBinaryToInt(hexToBinary(B[0]) + hexToBinary(B[0]));
        if (inst.opcode == op_code::JMP)
        {
            ALUOutput = PC + inst.imm8 * 2;
        }
        else if (inst.opcode == op_code::BEQZ)
        {
            if (rs1_value == 0)
            {
                ALUOutput = PC + inst.imm8 * 2;
            }
            else
                ALUOutput = PC;
        }
        break;
    }
    case InstructionType::HALT:
    {
        halt_inst++;
        break;
    }
    }
    return inst;
}

Instruction Program ::memOperation(const Instruction &inst)
{
    cout << ALUOutput << endl;
    cout << PC << endl;
    if (inst.type == InstructionType::CONTROL)
    {
        PC = ALUOutput;
    }
    if (inst.type == InstructionType::ARITHMETIC || inst.type == InstructionType::LOGICAL ||
        inst.type == InstructionType::SHIFT)
    {
        LMD = decToHexa(ALUOutput);
    }

    else if (inst.opcode == op_code::LD)
    {
        LMD = data[ALUOutput];
    }
    else if (inst.opcode == op_code::LI)
    {
        LMD = decToHexa(ALUOutput);
    }
    else if (inst.opcode == op_code::ST)
    {
        data[ALUOutput] = readRegister(inst.rd);
        
    }
    return inst;
}

Instruction Program ::writeBack(const Instruction &inst)
{
    if (inst.type == InstructionType::ARITHMETIC || inst.type == InstructionType::LOGICAL ||
        inst.type == InstructionType::SHIFT)
    {
        writeRegister(inst.rd, LMD);
    }
    else if (inst.type == InstructionType ::MEMORY)
    {
        if (inst.opcode == op_code::LD || inst.opcode == op_code::LI)
        {
            writeRegister(inst.rd, LMD);
        }
    }
    return inst;
}

void Program::Pipelining()
{
    // IF, ID, EX, MEM, WB
    Instruction fetched_inst;
    Instruction decoded_inst;
    Instruction executed_inst;
    Instruction mem_out_inst;
    Instruction inst_done;
    Instruction temp;

    bool isBranchResolved = false;
    bool rawHazard = false;
    bool controlHazard = false;

    // Instruction
    while (inst_done.type != InstructionType::HALT)
    {
        cycles++;

        // WB stage
        if (mem_out_inst.type != InstructionType::NOP)
            inst_done = writeBack(mem_out_inst);
        else
            inst_done = mem_out_inst;

        if (inst_done.opcode == op_code::HLT)
            break;

        // MEM stage
        if (executed_inst.type != InstructionType::NOP)
            mem_out_inst = memOperation(executed_inst);
        else
            mem_out_inst = executed_inst;

        // EX stage
        if (decoded_inst.type != InstructionType::NOP)
            executed_inst = executeInstruction(decoded_inst);
        else
            executed_inst = decoded_inst;

        // ID stage
        if (fetched_inst.type != InstructionType::NOP)
            decoded_inst = decodeInstruction(fetched_inst);
        else
            decoded_inst = fetched_inst;

        // IF stage
        if (!controlHazard && !rawHazard)
            fetched_inst = fetchInstruction(PC);
        else
            fetched_inst = Instruction();

        // Control Hazard Detection
        if (fetched_inst.type == InstructionType::CONTROL)
        {
            cout << "Control Hazard Detected." << endl;
            control_stalls += 2;
            controlHazard = true;
        }
        else if (executed_inst.type == InstructionType::CONTROL && controlHazard)
        {
            cout << "Control Hazard over." << endl;
            // isBranchResolved = true;
            controlHazard = false;
        }

        // RAW Hazard Detection
        if ((((decoded_inst.type != InstructionType::CONTROL && decoded_inst.opcode != op_code::ST) && fetched_inst.type != InstructionType::NOP) && (fetched_inst.rs1 != (-1) && fetched_inst.opcode != op_code::JMP)) &&
            (decoded_inst.rd == fetched_inst.rs1 || decoded_inst.rd == fetched_inst.rs2))
        {
            cout << "Raw Hazard Detected." << endl;
            data_stalls += 2;
            rawHazard = true;
            temp = fetched_inst;
            fetched_inst = Instruction();
            decoded_inst.raw_hazard = true;
        }

        if (!rawHazard)
            if ((((executed_inst.type != InstructionType::CONTROL && executed_inst.opcode != op_code::ST) &&
                  fetched_inst.type != InstructionType::NOP) &&
                 (fetched_inst.rs1 != (-1) && fetched_inst.opcode != op_code::JMP)) &&
                (executed_inst.rd == fetched_inst.rs1 || executed_inst.rd == fetched_inst.rs2))
            {
                data_stalls += 1;
                rawHazard = true;
                temp = fetched_inst;
                fetched_inst = Instruction();
                executed_inst.raw_hazard = true;
            }

        if (rawHazard && mem_out_inst.raw_hazard)
        {
            mem_out_inst.raw_hazard = false;
            fetched_inst = temp;
            rawHazard = false;
        }
        cout << endl;
    }
}

void Program::PrintResult(const string& file)
{
    ofstream outputFile(file);
    if (!outputFile.is_open()) {
        cout << "Error opening the file " << file << endl;
        return;
    }

    arithmatic_inst--;
    int total_inst = arithmatic_inst + logical_inst + shift_inst + memory_inst + control_inst + halt_inst;
    double cpi = (double)cycles / total_inst;
    data_stalls -= 2;

    outputFile << "Total number of instructions executed        : " << total_inst << endl;
    outputFile << "Number of instructions in each class" << endl;
    outputFile << "Arithmetic instructions                      : " << arithmatic_inst << endl;
    outputFile << "Logical instructions                         : " << logical_inst << endl;
    outputFile << "Shift instructions                           : " << shift_inst << endl;
    outputFile << "Memory instructions                          : " << memory_inst - loadIM_inst << endl;
    outputFile << "Load immediate instructions                  : " << loadIM_inst << endl;
    outputFile << "Control instructions                         : " << control_inst << endl;
    outputFile << "Halt instructions                            : " << halt_inst << endl;
    outputFile << "Cycles Per Instruction                       : " << cpi << endl;
    outputFile << "Total number of stalls                       : " << data_stalls + control_stalls << endl;
    outputFile << "Data stalls (RAW)                            : " << data_stalls << endl;
    outputFile << "Control stalls                               : " << control_stalls << endl;

    outputFile.close();
}


vector<string> ReadCache(const string &filename)
{
    vector<string> cache;
    ifstream file(filename);
    string line;

    if (file.is_open())
    {
        while (getline(file, line))
        {
            cache.push_back(line);
        }
        file.close();
    }
    else
    {
        cerr << "Unable to open " << filename << endl;
    }
    return cache;
}

void WriteToFile(const vector<string> &data, const string &filename)
{
    ofstream file(filename);
    if (file.is_open())
    {
        for (const string &line : data)
        {
            file << line << endl;
        }
        file.close();
        cout << "Data written to " << filename << " successfully." << endl;
    }
    else
    {
        cerr << "Unable to open file " << filename << " for writing." << endl;
    }
}

int main()
{
    vector<string> iCache = ReadCache("./input/ICache.txt");
    vector<string> dCache = ReadCache("./input/DCache.txt");
    vector<string> registerFile = ReadCache("./input/RF.txt");

    Program P(iCache, dCache, registerFile);
    P.Pipelining();
    WriteToFile(P.getData(), "./output/DCache.txt");
    P.PrintResult("./output/Output.txt");
}