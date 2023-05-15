#include "Value.h"

DWORD tmpNum = 0;

Value::Value()
{
	Name = "t" + std::to_string(tmpNum);
	//printf("[Default Constructor] Value Name :%s\n", Name.c_str());
	tmpNum++;
}

Value::Value(DWORD val, WORD size)
{
	OperandType = OPERANDTYPE_CONSTANT;
	Size = size;
	Name = "t" + std::to_string(tmpNum);
	//printf("Value Name :%s\n", Name.c_str());
	tmpNum++;
}

Value::Value(string ValName, DWORD idx)
{
	Name = ValName + "(" + std::to_string(idx) + ")";
	//printf("Value Name :%s\n", Name.c_str());
}

Value::Value(const Value& valRef)
	:OperandType(valRef.OperandType), Size(valRef.Size), UseList(valRef.UseList), isHiddenRHS(valRef.isHiddenRHS)
{
	Name = valRef.Name;
	//printf("Value Name :%s\n", Name.c_str());
}

IR* CraeteBVVIR(DWORD intVal, BYTE size)
{
	Value* op1 = new ConstInt(IR::OPR_GET32INT, intVal);
	Value* op2 = new ConstInt(IR::OPR_GET8INT, size);
	IR * testAddIR = new IR(IR::OPR_BVV, op1, op2, size);
	testAddIR->isHiddenRHS = true;
	return testAddIR;
}

IR* CraeteLoadIR(Value* op1, IR::OPR _opr)
{
	IR * testAddIR = new IR(IR::OPR_LOAD, op1);
	testAddIR->isHiddenRHS = true;
	return testAddIR;
}

IR* CraeteBinaryIR(Value* op1, Value* op2, IR::OPR _opr)
{
	IR * testAddIR = new IR(_opr, op1, op2);
	testAddIR->Size = op1->Size;
	return testAddIR;
}

IR* CraeteStoreIR(Value* op1, Value* op2, IR::OPR _opr)
{
	IR * testAddIR = new IR(IR::OPR_STORE, op1, op2);
	testAddIR->isHiddenRHS = true;
	return testAddIR;
}

IR* CraeteBitVecValIR(DWORD val, DWORD size, IR::OPR _opr)
{
	Value* valValue = new ConstInt(IR::OPR_GET32INT, val);
	Value* sizeValue = new ConstInt(IR::OPR_GET32INT, size);
	IR * testAddIR = new IR(IR::OPR_BVV, valValue, sizeValue);
	return testAddIR;
}

void ConcatOptimize(Value* concatValue)
{
	IR* concatIR = dynamic_cast<IR*>(concatValue);
	bool isOperandSame = true;
	if (concatIR != nullptr)
	{
		if (concatIR->opr == IR::OPR_CONCAT)
		{
			for (int i = 0; i < concatIR->Operands.size(); i++)
			{
				if (concatIR->Operands[0]->valuePtr != concatIR->Operands[i]->valuePtr)
				{
					isOperandSame = false;
					break;
				}
			}

			if (isOperandSame == true)
			{
				for (int i = 0; i < concatIR->Operands.size(); i++)
				{
					if (dynamic_cast<IR*>(concatIR->Operands[i]->valuePtr))
					{
						IR::OPR opr = dynamic_cast<IR*>(concatIR->Operands[i]->valuePtr)->opr;
						switch (opr)
						{
							case IR::OPR_EXTRACT16H:
								break;
							case IR::OPR_EXTRACT8HH:
								break;
							case IR::OPR_EXTRACT8HL:
								break;
							case IR::OPR_EXTRACT8H:
								break;
							case IR::OPR_EXTRACT8L:
								break;
							default:
								break;
						}						
					}
				}
			}
		}
	}
}