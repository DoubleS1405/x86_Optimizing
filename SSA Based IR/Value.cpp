#include "Value.h"

extern z3::context* z3Context;
extern z3::expr* z3Equation;
extern map<string, z3::expr*> symbolExprMap;

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
	IR* op1 = new ConstInt(IR::OPR_GET32INT, intVal);
	op1->Size = size;
	op1->ast = std::make_shared<BTreeNode>(MakeBTreeNode(op1->printOpr(IR::OPR_GET32INT)));
	op1->ast->m_NodeType = NT_OPERATOR;
	op1->ast->valPtr = op1;

	IR* op2 = new ConstInt(IR::OPR_GET8INT, size);
	op2->Size = 8;
	op2->ast = std::make_shared<BTreeNode>(MakeBTreeNode(op2->printOpr(IR::OPR_GET8INT)));
	op2->ast->m_NodeType = NT_OPERATOR;
	op2->ast->valPtr = op2;

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
		//printf("ADD Operand can constant folding %x + %x = %x\n", dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar,
			//dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar,
			//dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar + dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar);
	}
	// Constant Folding 가능 조건이 아닌 경우 IR 생성
	else
	{
		rst = CraeteBinaryIR(op1, op2, IR::OPR::OPR_ADD);
	}

	return rst;
}

IR* CreateSubIR(Value* op1, Value* op2)
{
	IR* rst = nullptr;
	if (op1->Size != op2->Size)
	{
		printf("CraeteSubIR Error (Operand is not matched)\n");
		return 0;
	}
	// Constant Folding 가능한 경우 CreateBinaryIR를 호출하지 않고 Imm Value 생성
	if (isConstantFolding(op1, op2))
	{
		rst = CraeteBVVIR(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar - dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar, 32);
		//printf("ADD Operand can constant folding %x + %x = %x\n", dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar,
		//dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar,
		//dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar + dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar);
	}
	// Constant Folding 가능 조건이 아닌 경우 IR 생성
	else
	{
		rst = CraeteBinaryIR(op1, op2, IR::OPR::OPR_SUB);
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

string IR::printOpr(OPR _opr)
{
	switch (_opr)
	{
	case IR::OPR::OPR_CONCAT:
		return string("Concat");
		break;
	case IR::OPR::OPR_BVV:	
		return string("OPR_BVV");
		break;
	case IR::OPR::OPR_GET32INT:
		return string("OPR_GET32INT");
		break;
	case IR::OPR::OPR_GET8INT:
		return string("OPR_GET8INT");
		break;
	case IR::OPR::OPR_EXTRACT16H:
		return string("EXTRACT16H");
		break;
	case IR::OPR::OPR_EXTRACT8HH:
		return string("EXTRACT8HH");
		break;
	case IR::OPR::OPR_EXTRACT8HL:
		return string("EXTRACT8HL");
		break;
	case IR::OPR::OPR_EXTRACT8H:
		return string("EXTRACT8H");
		break;
	case IR::OPR::OPR_EXTRACT8L:
		return string("EXTRACT8L");
		break;
	case IR::OPR::OPR_ADD:
		return string("OPR_ADD");
		break;
	case IR::OPR::OPR_SUB:
		return string("OPR_SUB");
		break;
	case IR::OPR::OPR_OR:
		return string("OPR_OR");
		break;
	case IR::OPR::OPR_AND:
		return string("OPR_AND");
		break;
	case IR::OPR::OPR_XOR:
		return string("OPR_XOR");
		break;
	case IR::OPR::OPR_ROL:
		return string("OPR_ROL");
		break;
	case IR::OPR::OPR_SAR:
		return string("OPR_SAR");
		break;
	case IR::OPR::OPR_LOAD:
		return string("OPR_LOAD");
		break;
	case IR::OPR::OPR_STORE:
		return string("OPR_X");
		break;
	default:
		return string("OPR_STORE");
		break;
	}
}

//string printOpr(IR::OPR _opr)
//{
//	switch (_opr)
//	{
//	case IR::OPR::OPR_CONCAT:
//		return string("Concat");
//		break;
//	case IR::OPR::OPR_BVV:
//		return string("OPR_BVV");
//		break;
//	case IR::OPR::OPR_EXTRACT16H:
//		return string("EXTRACT16H");
//		break;
//	case IR::OPR::OPR_EXTRACT8HH:
//		return string("EXTRACT8HH");
//		break;
//	case IR::OPR::OPR_EXTRACT8HL:
//		return string("EXTRACT8HL");
//		break;
//	case IR::OPR::OPR_EXTRACT8H:
//		return string("EXTRACT8H");
//		break;
//	case IR::OPR::OPR_EXTRACT8L:
//		return string("EXTRACT8L");
//		break;
//	case IR::OPR::OPR_ADD:
//		return string("OPR_ADD");
//		break;
//	case IR::OPR::OPR_SUB:
//		return string("OPR_SUB");
//		break;
//	case IR::OPR::OPR_OR:
//		return string("OPR_OR");
//		break;
//	case IR::OPR::OPR_AND:
//		return string("OPR_AND");
//		break;
//	case IR::OPR::OPR_XOR:
//		return string("OPR_XOR");
//		break;
//	case IR::OPR::OPR_ROL:
//		return string("OPR_ROL");
//		break;
//	case IR::OPR::OPR_SAR:
//		return string("OPR_SAR");
//		break;
//	case IR::OPR::OPR_LOAD:
//		return string("OPR_LOAD");
//		break;
//	case IR::OPR::OPR_STORE:
//		return string("OPR_X");
//		break;
//	default:
//		return string("OPR_STORE");
//		break;
//	}
//}



string EvaluateExpTree(std::shared_ptr<BTreeNode> bt)
{
	string op1, op2, op3, op4;
	string exprString;

	if (bt->left == NULL)
	{
		if (bt->m_NodeType == NT_SYMVAR)
		{
			if (symbolExprMap.find(bt->nodeName) != symbolExprMap.end())
			{
				z3Equation = symbolExprMap[bt->nodeName];
			}
		}
		return bt->nodeName;
	}

	op1 = EvaluateExpTree(bt->left);
	if (bt->right != NULL)
	{
		op2 = EvaluateExpTree(bt->right);
	}

	if (bt->nodeName.compare("OPR_ADD") == 0)
	{
		//*z3Equation = 
		return "bvadd " + op1 + " " + op2;
	}

	else if (bt->nodeName.compare("Concat") == 0)
	{
		if (bt->third != NULL)
			op3 = EvaluateExpTree(bt->third);
		if (bt->fourth != NULL)
			op4 = EvaluateExpTree(bt->fourth);
		if (bt->fourth != nullptr)
		{
			//z3::concat_expr
			return "(Concat " + op1 + " " + op2 + " " + op3 + " " + op4 + ")";
		}

		else if (bt->third != nullptr && bt->fourth == nullptr)
		{
			return "(Concat " + op1 + " " + op2 + " " + op3 + ")";
		}

		else
		{
			return "(Concat " + op1 + " " + op2 + ")";
		}
	}

	else if (bt->nodeName.compare("EXTRACT16H") == 0)
	{
		return "(Extract 31 16  " + op1 + ")";
	}

	else if (bt->nodeName.compare("EXTRACT8H") == 0)
	{
		return "(Extract 15 8  " + op1 + ")";
	}

	else if (bt->nodeName.compare("EXTRACT8L") == 0)
	{
		return "(Extract 7 0  " + op1 + ")";
	}

	else
	{
		if (bt->nodeName.empty() == false)
		{
			//if (bt->m_NodeType == NT_SYMVAR)
			//{
			//	if (symbolExprMap.find(bt->nodeName) != symbolExprMap.end())
			//	{
			//		symbolExprMap[bt->m_NodeType]
			//	}
			//}

			//else
			//{
			//	
			//}
			//printf("[test] %s\n", bt->nodeName.c_str());
			exprString = bt->nodeName;
		}

		else
		{
			exprString = "di rlagkstn2";
		}
	}

	return "";
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