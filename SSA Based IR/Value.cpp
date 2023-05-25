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
	//testAddIR->isHiddenRHS = true;
	return testAddIR;
}



IR* CraeteSubIR(Value* op1, IR::OPR _opr)
{

}

IR* CraeteAndIR(Value* op1, IR::OPR _opr)
{

}

IR* CraeteOrIR(Value* op1, IR::OPR _opr)
{

}

IR* CraeteXorIR(Value* op1, IR::OPR _opr)
{

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

IR* CreateAddIR(Value* op1, Value* op2)
{
	IR* rst = nullptr;
	if (op1->Size != op2->Size)
	{
		printf("CraeteAddIR Error (Operand is not matched)\n");
		return 0;
	}
	// Constant Folding 가능한 경우 CreateBinaryIR를 호출하지 않고 Imm Value 생성
	if (isConstantFolding(op1, op2))
	{
		rst = CraeteBVVIR(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar + dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar, 32);
		printf("ADD Operand can constant folding %x + %x = %x\n", dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar,
			dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar,
			dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar + dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar);
	}
	// Constant Folding 가능 조건이 아닌 경우 IR 생성
	else
	{
		rst = CraeteBinaryIR(op1, op2, IR::OPR::OPR_ADD);
	}

	return rst;
}

IR* CraeteStoreIR(Value* op1, Value* op2, IR::OPR _opr)
{
	IR * testAddIR = new IR(IR::OPR_STORE, op1, op2);
	testAddIR->Size = op1->Size;
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

bool isConstantFolding(Value* _op1, Value* _op2)
{
	IR* op1IR = nullptr;
	IR* op2IR = nullptr;

	op1IR = dynamic_cast<IR*>(_op1);
	op2IR = dynamic_cast<IR*>(_op2);

	if (op1IR && op2IR)
	{
		if (op1IR->opr == IR::OPR::OPR_BVV && op1IR->opr == IR::OPR::OPR_BVV)
			return true;
	}
	return false;
}