#pragma once
#include <Windows.h>
#include <stdio.h>
#include <Zycore/LibC.h>
#include <Zydis/Zydis.h>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "AST.h"
#include "z3++.h"


using namespace std;

class OPERAND;
class IR;

class Value
{
public:
	enum VALUE_TYPE
	{
		OPERANDTYPE_REGISTER,
		OPERANDTYPE_MEMORY,
		OPERANDTYPE_CONSTANT
	};

	VALUE_TYPE OperandType;
	BYTE Size; // 8, 16, 32
	set<OPERAND*> UseList; // 현재 Value를 Use 하는 오퍼랜드
	std::string Name;
	bool isHiddenRHS=false;
	bool isTainted = false;

	shared_ptr<BTreeNode> ast;

	Value();

	Value::Value(const Value& valRef);

	Value(DWORD val, WORD size);

	Value(string ValName, DWORD idx);

	virtual void test() {};

};

class OPERAND
{
public:
	IR* parent;
	Value* valuePtr;
};

class IR : public Value
{
public:
	enum OPR
	{
		OPR_NONE,
		OPR_CONCAT,
		OPR_EXTRACT,
		OPR_EXTRACT16H,
		OPR_EXTRACT8HH,
		OPR_EXTRACT8HL,
		OPR_EXTRACT8H,
		OPR_EXTRACT8L,
		OPR_BVV, // BitVecVal 
		OPR_GET32INT,
		OPR_GET16INT,
		OPR_GET8INT,
		OPR_STORE,
		OPR_LOAD,
		OPR_ADD,
		OPR_SUB,
		OPR_AND,
		OPR_OR,
		OPR_XOR,
		OPR_BT,
		OPR_BTC,
		OPR_RCL,
		OPR_ROL,
		OPR_SAR,
	};
	OPR opr;
	vector<OPERAND*> Operands;
	OPERAND* AddOperand(Value *OperandValuePtr)
	{
		OPERAND* oprPtr = new OPERAND;
		oprPtr->parent = this;
		oprPtr->valuePtr = OperandValuePtr;
		Operands.push_back(oprPtr);
		OperandValuePtr->UseList.insert(oprPtr);
		return oprPtr;
	}

	IR(OPR _opr)
	{
		opr = _opr;
	}

	IR(OPR _opr, Value* op1)
	{
		opr = _opr;
		AddOperand(op1);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);
	}

	IR(string ValName, DWORD index, OPR _opr, Value* op1):Value(ValName, index)
	{
		opr = _opr;
		AddOperand(op1);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);
	}

	IR(OPR _opr, Value* op1, Value* op2)
	{
		opr = _opr;
		AddOperand(op1);
		AddOperand(op2);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);

		std::shared_ptr<BTreeNode>  ptr2 = std::make_shared<BTreeNode>(MakeBTreeNode(op2->Name));
		MakeRightSubTree(ast, op2->ast);
	}

	IR(OPR _opr, Value* op1, Value* op2, BYTE size)
	{
		opr = _opr;
		Size = size;
		AddOperand(op1);
		AddOperand(op2);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);

		std::shared_ptr<BTreeNode>  ptr2 = std::make_shared<BTreeNode>(MakeBTreeNode(op2->Name));
		MakeRightSubTree(ast, op2->ast);
	}

	IR(string ValName, DWORD index, OPR _opr, Value* op1, Value* op2)
	{
		opr = _opr;
		AddOperand(op1);
		AddOperand(op2);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);

		std::shared_ptr<BTreeNode>  ptr2 = std::make_shared<BTreeNode>(MakeBTreeNode(op2->Name));
		MakeRightSubTree(ast, op2->ast);

		Value(ValName, index);
	}

	IR(OPR _opr, Value* op1, Value* op2, Value* op3)
	{
		opr = _opr;
		AddOperand(op1);
		AddOperand(op2);
		AddOperand(op3);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);

		std::shared_ptr<BTreeNode>  ptr2 = std::make_shared<BTreeNode>(MakeBTreeNode(op2->Name));
		MakeRightSubTree(ast, op2->ast);

		std::shared_ptr<BTreeNode>  ptr3 = std::make_shared<BTreeNode>(MakeBTreeNode(op3->Name));
		MakeThirdSubTree(ast, op3->ast);
	}

	IR(OPR _opr, Value* op1, Value* op2, Value* op3, Value* op4)
	{
		opr = _opr;
		AddOperand(op1);
		AddOperand(op2);
		AddOperand(op3);
		AddOperand(op4);

		ast = std::make_shared<BTreeNode>(MakeBTreeNode(printOpr(_opr)));
		ast->m_NodeType = NT_OPERATOR;

		std::shared_ptr<BTreeNode> ptr1 = std::make_shared<BTreeNode>(MakeBTreeNode(op1->Name));
		MakeLeftSubTree(ast, op1->ast);

		std::shared_ptr<BTreeNode>  ptr2 = std::make_shared<BTreeNode>(MakeBTreeNode(op2->Name));
		MakeRightSubTree(ast, op2->ast);

		std::shared_ptr<BTreeNode>  ptr3 = std::make_shared<BTreeNode>(MakeBTreeNode(op3->Name));
		MakeThirdSubTree(ast, op3->ast);

		std::shared_ptr<BTreeNode>  ptr4 = std::make_shared<BTreeNode>(MakeBTreeNode(op4->Name));
		MakeFourthSubTree(ast, op4->ast);
	}
	string printOpr(OPR _opr);
};

class ConstInt : public IR
{
public:
	DWORD intVar;

public:
	ConstInt(OPR opr, DWORD var) :IR(opr)
	{
		switch (opr)
		{
		case OPR_BVV:
			Size = 32;
			break;

		case OPR_GET16INT:
			Size = 16;
			break;

		case OPR_GET8INT:
			Size = 8;
			break;
		}
		intVar = var;
	}
};

IR* CraeteLoadIR(Value* op1, IR::OPR _opr);

IR* CraeteBinaryIR(Value* op1, Value* op2, IR::OPR _opr);

IR* CreateAddIR(Value* op1, Value* op2);

IR* CreateSubIR(Value* op1, Value* op2);

IR* CraeteStoreIR(Value* op1, Value* op2, IR::OPR _opr);

IR* CraeteBVVIR(DWORD intVal, BYTE size);

bool isConstantFolding(Value* _op1, Value* _op2);