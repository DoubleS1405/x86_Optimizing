#include <Windows.h>
#include <psapi.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <Zycore/LibC.h>
#include <Zydis/Zydis.h>

#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <functional>
#include "Value.h"

z3::context* z3Context;
z3::expr* z3Equation;

#define REG_EAX_16H (3*0)+0
#define REG_EAX_8H (3*0)+1
#define REG_EAX_8L (3*0)+2

#define REG_EBX_16H (3*1)+0
#define REG_EBX_8H (3*1)+1
#define REG_EBX_8L (3*1)+2

#define REG_ECX_16H (3*2)+0
#define REG_ECX_8H (3*2)+1
#define REG_ECX_8L (3*2)+2

#define REG_EDX_16H (3*3)+0
#define REG_EDX_8H (3*3)+1
#define REG_EDX_8L (3*3)+2

#define REG_EBP_16H (3*4)+0
#define REG_EBP_8H (3*4)+1
#define REG_EBP_8L (3*4)+2

#define REG_ESP_16H (3*5)+0
#define REG_ESP_8H (3*5)+1
#define REG_ESP_8L (3*5)+2

#define REG_ESI_16H (3*6)+0
#define REG_ESI_8H (3*6)+1
#define REG_ESI_8L (3*6)+2

#define REG_EDI_16H (3*7)+0
#define REG_EDI_8H (3*7)+1
#define REG_EDI_8L (3*7)+2

map<string, z3::expr*> symbolExprMap;
vector<Value*> StatusFlagsValue[5];
vector<Value*> RegValue[ZYDIS_REGISTER_MAX_VALUE];
map<DWORD, vector<Value*>> MemValue;
map<DWORD, vector<IR*>> IRList;

Value* GetImmValue(DWORD _immValue, BYTE _size, vector<IR*>& irList);

void findMemValue()
{
	for (auto testIt : IRList)
	{
		for (auto irIt : testIt.second)
		{
			if (irIt->opr == IR::OPR_STORE)
			{
				if (ConstInt* constIr = dynamic_cast<ConstInt*>(irIt->Operands[1]->valuePtr))
				{
					MemValue[constIr->intVar].push_back(irIt->Operands[0]->valuePtr);
				}
			}
		}
	}
}

// Value에 대해서 각 레지스터의 파트 별로 저장한다. (상위 16비트, 상위 8비트, 하위 8비트)
void SaveRegisterValue(ZydisRegister _zydisReg, Value* _regValue, BYTE _size, vector<IR*>& irList)
{
	DWORD op1ConstValue;
	DWORD op2ConstValue;
	DWORD op3ConstValue;

	if (_size == 32)
	{
		IR* reg16hIR;
		IR* reg8hIR;
		IR* reg8lIR;

		switch (_zydisReg)
		{
		case ZYDIS_REGISTER_EAX:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_EAX_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_EAX_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_EAX_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("EAX16H", RegValue[REG_EAX_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			reg16hIR->Size = 16;
			RegValue[REG_EAX_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("EAX8H", RegValue[REG_EAX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			reg8hIR->Size = 8;
			RegValue[REG_EAX_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("EAX8L", RegValue[REG_EAX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			reg8lIR->Size = 8;
			RegValue[REG_EAX_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_EBX:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_EBX_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_EBX_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_EBX_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("EBX16H", RegValue[REG_EBX_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			reg16hIR->Size = 16;
			RegValue[REG_EBX_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("EBX8H", RegValue[REG_EBX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			reg8hIR->Size = 8;
			RegValue[REG_EBX_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("EBX8L", RegValue[REG_EBX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			reg8lIR->Size = 8;
			RegValue[REG_EBX_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_ECX:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_ECX_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_ECX_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_ECX_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("ECX16H", RegValue[REG_ECX_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			reg16hIR->Size = 16;
			RegValue[REG_ECX_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("ECX8H", RegValue[REG_ECX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			reg8hIR->Size = 8;
			RegValue[REG_ECX_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("ECX8L", RegValue[REG_ECX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			reg8lIR->Size = 8;
			RegValue[REG_ECX_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_EDX:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_EDX_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_EDX_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_EDX_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("EDX16H", RegValue[REG_EDX_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			RegValue[REG_EDX_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("EDX8H", RegValue[REG_EDX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EDX_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("EDX8L", RegValue[REG_EDX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EDX_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_EBP:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_EBP_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_EBP_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_EBP_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("EBP16H", RegValue[REG_EBP_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			RegValue[REG_EBP_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("EBP8H", RegValue[REG_EBP_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EBP_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("EBP8L", RegValue[REG_EBP_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EBP_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		//case ZYDIS_REGISTER_ESP:
		//	if (dynamic_cast<IR*>(_regValue))
		//	{
		//		if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
		//		{
		//			op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
		//			reg16hIR = CraeteBVVIR(op1ConstValue, 16);
		//			RegValue[REG_ESP_16H].push_back(reg16hIR);
		//			irList.push_back(reg16hIR);

		//			op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
		//			reg8hIR = CraeteBVVIR(op2ConstValue, 8);
		//			RegValue[REG_ESP_8H].push_back(reg8hIR);
		//			irList.push_back(reg8hIR);

		//			op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
		//			reg8lIR = CraeteBVVIR(op3ConstValue, 8);
		//			RegValue[REG_ESP_8L].push_back(reg8lIR);
		//			irList.push_back(reg8lIR);

		//			return;
		//		}
		//	}
		//	reg16hIR = new IR("ESP16H", RegValue[REG_ESP_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
		//	RegValue[REG_ESP_16H].push_back(reg16hIR);
		//	irList.push_back(reg16hIR);

		//	reg8hIR = new IR("ESP8H", RegValue[REG_ESP_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
		//	RegValue[REG_ESP_8H].push_back(reg8hIR);
		//	irList.push_back(reg8hIR);

		//	reg8lIR = new IR("ESP8L", RegValue[REG_ESP_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
		//	RegValue[REG_ESP_8L].push_back(reg8lIR);
		//	irList.push_back(reg8lIR);
		//	break;

		case ZYDIS_REGISTER_ESI:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_ESI_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_ESI_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_ESI_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("ESI16H", RegValue[REG_ESI_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			RegValue[REG_ESI_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("ESI8H", RegValue[REG_ESI_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_ESI_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("ESI8LH", RegValue[REG_ESI_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_ESI_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_EDI:
			if (dynamic_cast<IR*>(_regValue))
			{
				if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
				{
					op1ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xffff0000) >> 16;
					reg16hIR = CraeteBVVIR(op1ConstValue, 16);
					RegValue[REG_EDI_16H].push_back(reg16hIR);
					irList.push_back(reg16hIR);

					op2ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff00) >> 8;
					reg8hIR = CraeteBVVIR(op2ConstValue, 8);
					RegValue[REG_EDI_8H].push_back(reg8hIR);
					irList.push_back(reg8hIR);

					op3ConstValue = (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(_regValue)->Operands[0]->valuePtr)->intVar & 0xff);
					reg8lIR = CraeteBVVIR(op3ConstValue, 8);
					RegValue[REG_EDI_8L].push_back(reg8lIR);
					irList.push_back(reg8lIR);

					return;
				}
			}
			reg16hIR = new IR("EDI16H", RegValue[REG_EDI_16H].size(), IR::OPR::OPR_EXTRACT16H, _regValue);
			RegValue[REG_EDI_16H].push_back(reg16hIR);
			irList.push_back(reg16hIR);

			reg8hIR = new IR("EDI8H", RegValue[REG_EDI_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EDI_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR("EDI8L", RegValue[REG_EDI_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EDI_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;
		}
	}

	else if (_size == 16)
	{
		IR* reg8hIR;
		IR* reg8lIR;

		switch (_zydisReg)
		{
		case ZYDIS_REGISTER_AX:
			if (dynamic_cast<ConstInt*>(_regValue))
			{
				op2ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff00) >> 8;
				Value* reg8hIR = new ConstInt(IR::OPR_GET8INT, op2ConstValue);
				RegValue[REG_EAX_8H].push_back(reg8hIR);

				op3ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff);
				Value* reg8lIR = new ConstInt(IR::OPR_GET8INT, op3ConstValue);
				RegValue[REG_EAX_8L].push_back(reg8lIR);
				break;
			}

			else
			{
				reg8hIR = new IR("EAX8H", RegValue[REG_EAX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
				RegValue[REG_EAX_8H].push_back(reg8hIR);
				irList.push_back(reg8hIR);

				reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
				RegValue[REG_EAX_8L].push_back(reg8lIR);
				irList.push_back(reg8lIR);
				break;
			}

		case ZYDIS_REGISTER_BX:
			if (dynamic_cast<ConstInt*>(_regValue))
			{
				op2ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff00) >> 8;
				Value* reg8hIR = new ConstInt(IR::OPR_GET8INT, op2ConstValue);
				RegValue[REG_EBX_8H].push_back(reg8hIR);

				op3ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff);
				Value* reg8lIR = new ConstInt(IR::OPR_GET8INT, op3ConstValue);
				RegValue[REG_EBX_8L].push_back(reg8lIR);
				break;
			}
			else
			{
				reg8hIR = new IR("EBX8H", RegValue[REG_EBX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
				RegValue[REG_EBX_8H].push_back(reg8hIR);
				irList.push_back(reg8hIR);

				reg8lIR = new IR("EBX8L", RegValue[REG_EBX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
				RegValue[REG_EBX_8L].push_back(reg8lIR);
				irList.push_back(reg8lIR);
				break;
			}

		case ZYDIS_REGISTER_CX:
			if (dynamic_cast<ConstInt*>(_regValue))
			{
				op2ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff00) >> 8;
				Value* reg8hIR = new ConstInt(IR::OPR_GET8INT, op2ConstValue);
				RegValue[REG_ECX_8H].push_back(reg8hIR);

				op3ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff);
				Value* reg8lIR = new ConstInt(IR::OPR_GET8INT, op3ConstValue);
				RegValue[REG_ECX_8L].push_back(reg8lIR);
				break;
			}

			else
			{
				reg8hIR = new IR("ECX8H", RegValue[REG_ECX_8H].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
				RegValue[REG_ECX_8H].push_back(reg8hIR);
				irList.push_back(reg8hIR);

				reg8lIR = new IR("ECX8L", RegValue[REG_ECX_8L].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
				RegValue[REG_ECX_8L].push_back(reg8lIR);
				irList.push_back(reg8lIR);
				break;
			}

		case ZYDIS_REGISTER_DX:

			if (dynamic_cast<ConstInt*>(_regValue))
			{
				op2ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff00) >> 8;
				Value* reg8hIR = new ConstInt(IR::OPR_GET8INT, op2ConstValue);
				RegValue[REG_EDX_8H].push_back(reg8hIR);

				op3ConstValue = (dynamic_cast<ConstInt*>(_regValue)->intVar & 0xff);
				Value* reg8lIR = new ConstInt(IR::OPR_GET8INT, op3ConstValue);
				RegValue[REG_EDX_8L].push_back(reg8lIR);
				break;
			}

			reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EDX_8H].push_back(reg8hIR);
			irList.push_back(reg8hIR);

			reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EDX_8L].push_back(reg8lIR);
			irList.push_back(reg8lIR);
			break;
		}
	}

	else if (_size == 8)
	{
		IR* reg8hIR;
		IR* reg8lIR;

		switch (_zydisReg)
		{
		case ZYDIS_REGISTER_AH:
			//reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EAX_8H].push_back(_regValue);
			//irList.push_back(reg8hIR);
			break;

		case ZYDIS_REGISTER_BH:
			//reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EBX_8H].push_back(_regValue);
			//irList.push_back(reg8hIR);
			break;

		case ZYDIS_REGISTER_CH:
			//reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_ECX_8H].push_back(_regValue);
			//irList.push_back(reg8hIR);
			break;

		case ZYDIS_REGISTER_DH:
			//reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);
			RegValue[REG_EDX_8H].push_back(_regValue);
			//irList.push_back(reg8hIR);
			break;

		case ZYDIS_REGISTER_AL:
			//reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EAX_8L].push_back(_regValue);
			//irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_BL:
			//reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EBX_8H].push_back(_regValue);
			//irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_CL:
			//reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_ECX_8L].push_back(_regValue);
			//irList.push_back(reg8lIR);
			break;

		case ZYDIS_REGISTER_DL:
			//reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
			RegValue[REG_EDX_8L].push_back(_regValue);
			//irList.push_back(reg8lIR);
			break;
		}
	}
}

// Store 명령어의 오퍼랜드가 Constant인 경우 해당 메모리 주소의 MemValuePool에 Value* 저장
Value* GetRegisterValue(ZydisRegister _zydisReg, vector<IR*>& irList)
{
	IR* rstIR = nullptr;
	Value * op1;
	Value* op2;
	Value* op3;

	DWORD op1ConstValue = 0;
	DWORD op2ConstValue = 0;
	DWORD op3ConstValue = 0;
	DWORD foldedConstValue = 0;

	//rstIR->opr = IR::OPR::OPR_CONCAT; 

	switch (_zydisReg)
	{
		// 32 비트 레지스터인 경우
	case ZYDIS_REGISTER_EAX:
		op1 = RegValue[REG_EAX_16H].back();
		op2 = RegValue[REG_EAX_8H].back();
		op3 = RegValue[REG_EAX_8L].back();

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		// Todo op1, op2, op3이 OPC_EXTRACT
		// OptimizeConcat(IR* concatIRPtr)

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		rstIR->OperandType = rstIR->OPERANDTYPE_REGISTER;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_EBX:
		op1 = (RegValue[REG_EBX_16H].back());
		op2 = (RegValue[REG_EBX_8H].back());
		op3 = (RegValue[REG_EBX_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		rstIR->OperandType = rstIR->OPERANDTYPE_REGISTER;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_ECX:
		op1 = (RegValue[REG_ECX_16H].back());
		op2 = (RegValue[REG_ECX_8H].back());
		op3 = (RegValue[REG_ECX_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		rstIR->OperandType = rstIR->OPERANDTYPE_REGISTER;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_EDX:
		op1 = (RegValue[REG_EDX_16H].back());
		op2 = (RegValue[REG_EDX_8H].back());
		op3 = (RegValue[REG_EDX_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_EBP:
		op1 = (RegValue[REG_EBP_16H].back());
		op2 = (RegValue[REG_EBP_8H].back());
		op3 = (RegValue[REG_EBP_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_ESP:
		op1 = (RegValue[REG_ESP_16H].back());
		op2 = (RegValue[REG_ESP_8H].back());
		op3 = (RegValue[REG_ESP_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_ESI:
		op1 = (RegValue[REG_ESI_16H].back());
		op2 = (RegValue[REG_ESI_8H].back());
		op3 = (RegValue[REG_ESI_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_EDI:
		op1 = (RegValue[REG_EDI_16H].back());
		op2 = (RegValue[REG_EDI_8H].back());
		op3 = (RegValue[REG_EDI_8L].back());

		if (dynamic_cast<IR*>(op1) &&
			dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op1)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op1ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op1)->Operands[0]->valuePtr)->intVar << 16;
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 32);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op1, op2, op3);
		rstIR->Size = 32;
		irList.push_back(rstIR);
		return rstIR;
		break;

		// 16 비트 레지스터
	case ZYDIS_REGISTER_AX:
		op2 = RegValue[REG_EAX_8H].back();
		op3 = RegValue[REG_EAX_8L].back();

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_BX:
		op2 = (RegValue[REG_EBX_8H].back());
		op3 = (RegValue[REG_EBX_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_CX:
		op2 = (RegValue[REG_ECX_8H].back());
		op3 = (RegValue[REG_ECX_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_DX:
		op2 = (RegValue[REG_EDX_8H].back());
		op3 = (RegValue[REG_EDX_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		//irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_BP:
		op2 = (RegValue[REG_EBP_8H].back());
		op3 = (RegValue[REG_EBP_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_SP:
		break;
	case ZYDIS_REGISTER_SI:
		op2 = (RegValue[REG_ESI_8H].back());
		op3 = (RegValue[REG_ESI_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;
	case ZYDIS_REGISTER_DI:
		op2 = (RegValue[REG_EDI_8H].back());
		op3 = (RegValue[REG_EDI_8L].back());

		if (dynamic_cast<IR*>(op2) &&
			dynamic_cast<IR*>(op3))
		{
			if (dynamic_cast<IR*>(op2)->opr == IR::OPR::OPR_BVV &&
				dynamic_cast<IR*>(op3)->opr == IR::OPR::OPR_BVV)
			{
				op2ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op2)->Operands[0]->valuePtr)->intVar << 8;
				op3ConstValue = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(op3)->Operands[0]->valuePtr)->intVar & 0xff;
				foldedConstValue = op2ConstValue | op3ConstValue;

				rstIR = CraeteBVVIR(foldedConstValue, 16);
				irList.push_back(rstIR);
				return rstIR;
			}

		}

		rstIR = new IR(IR::OPR::OPR_CONCAT, op2, op3);
		rstIR->Size = 16;
		irList.push_back(rstIR);
		return rstIR;
		break;

		// 8비트
	case ZYDIS_REGISTER_AH:
		op2 = (RegValue[REG_EAX_8H].back());
		return op2;
		break;

	case ZYDIS_REGISTER_AL:
		op2 = (RegValue[REG_EAX_8L].back());
		return op2;
		break;

	case ZYDIS_REGISTER_BH:
		op2 = (RegValue[REG_EBX_8H].back());
		return op2;
		break;

	case ZYDIS_REGISTER_BL:
		op2 = (RegValue[REG_EBX_8L].back());
		return op2;
		break;

	case ZYDIS_REGISTER_CH:
		op2 = (RegValue[REG_ECX_8H].back());
		return op2;
		break;

	case ZYDIS_REGISTER_CL:
		op2 = (RegValue[REG_ECX_8L].back());
		return op2;
		break;

	case ZYDIS_REGISTER_DH:
		op2 = (RegValue[REG_EDX_8H].back());
		return op2;
		break;

	case ZYDIS_REGISTER_DL:
		op2 = (RegValue[REG_EDX_8L].back());
		return op2;
		break;
	}
	return nullptr;
}

void SaveMemoryValue(ZydisDecodedInstruction* decodedInstPtr, ZydisDecodedOperand* _decodedOperandPtr, Value* _regValue, vector<IR*>& irList)

{
	IR* rstIR0 = nullptr;
	IR* rstIR1 = nullptr;
	IR* rstIR2 = nullptr;
	IR* rstIR3 = nullptr;
	IR* rstIR = nullptr;

	Value * BaseValue;
	Value* Disp;
	Value* op3;
	IR* offset1;
	IR* offset2;
	IR* offset3;
	IR* offset4;

	Value* offsetImm1;
	Value* offsetImm2;
	Value* offsetImm3;
	Value* offsetImm4;

	Value* offset0Value;
	Value* offset1Value;
	Value* offset2Value;
	Value* offset3Value;

	// Base
	if (_decodedOperandPtr->mem.base != ZYDIS_REGISTER_NONE)
	{
		BaseValue = GetRegisterValue(_decodedOperandPtr->mem.base, irList);

		if (_decodedOperandPtr->mem.disp.has_displacement)
		{
			// [Base + Index*Scale + Disp]
			Value* BaseDisp = CraeteBinaryIR(BaseValue, Disp, IR::OPR_ADD);
			if (_decodedOperandPtr->mem.index != ZYDIS_REGISTER_NONE)
			{

			}

			// [Base + Disp]
			else
			{

			}
		}

		// Displacement가 없는경우
		else
		{
			// [Base + Index*Scale]
			if (_decodedOperandPtr->mem.index != ZYDIS_REGISTER_NONE)
			{

			}
			// [Base]
			else
			{
				if (decodedInstPtr->address_width == 32)
				{
					// Base 레지스터의 Value가 상수인 경우 EA는 상수이므로 해당 EA에 대한 Memory Value Pool에 저장한다.
					if (dynamic_cast<IR*>(BaseValue)->opr == IR::OPR::OPR_BVV)
					{
						printf("SaveMemoryValue -> BaseValue is Constant\n");
						DWORD _memAddr = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar;
						IR* reg8hh;
						IR* reg8hlIR;
						IR* reg8hIR;
						IR* reg8lIR;

						// 메모리에 저장할 Value가 상수인 경우
						if (dynamic_cast<ConstInt*>(_regValue))
						{
							printf("SaveMemoryValue -> Value is Constnat\n");
						}

						// 메모리에 저장할 Value가 상수가 아닌 경우
						else
						{
							if (MemValue.find(_memAddr + 3) != MemValue.end())
							{
								reg8hh = new IR(_regValue->Name, MemValue[_memAddr + 3].size(), IR::OPR::OPR_EXTRACT8HH, _regValue);
							}
							else
								reg8hh = new IR(IR::OPR::OPR_EXTRACT8HH, _regValue);

							irList.push_back(reg8hh);
							offset3 = CraeteBVVIR(_memAddr + 3, 32);
							//offset3 = CraeteBinaryIR(BaseValue, offsetImm3, IR::OPR::OPR_ADD);
							irList.push_back(offset3);
							rstIR3 = CraeteStoreIR(offset3, reg8hh, IR::OPR_STORE);
							irList.push_back(rstIR3);
							MemValue[_memAddr + 3].push_back(reg8hh);

							if (MemValue.find(_memAddr + 2) != MemValue.end())
							{
								reg8hlIR = new IR(_regValue->Name, MemValue[_memAddr + 2].size(), IR::OPR::OPR_EXTRACT8HL, _regValue);
							}
							else
								reg8hlIR = new IR(IR::OPR::OPR_EXTRACT8HL, _regValue);

							irList.push_back(reg8hlIR);
							offset2 = CraeteBVVIR(_memAddr + 2, 32);
							//offset3 = CraeteBinaryIR(BaseValue, offsetImm3, IR::OPR::OPR_ADD);
							irList.push_back(offset2);
							rstIR2 = CraeteStoreIR(offset2, reg8hlIR, IR::OPR_STORE);
							irList.push_back(rstIR2);
							MemValue[_memAddr + 2].push_back(reg8hlIR);
							
							if (MemValue.find(_memAddr + 1) != MemValue.end())
							{
								reg8hIR = new IR(_regValue->Name, MemValue[_memAddr + 1].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
							}
							else
								reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);

							irList.push_back(reg8hIR);
							offset1 = CraeteBVVIR(_memAddr + 1, 32);
							//offset3 = CraeteBinaryIR(BaseValue, offsetImm3, IR::OPR::OPR_ADD);
							irList.push_back(offset1);
							rstIR1 = CraeteStoreIR(offset1, reg8hIR, IR::OPR_STORE);
							irList.push_back(rstIR1);
							MemValue[_memAddr + 1].push_back(reg8hIR);
							
							if (MemValue.find(_memAddr) != MemValue.end())
							{
								reg8lIR = new IR(_regValue->Name, MemValue[_memAddr].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
							}
							else
								reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);
							MemValue[_memAddr].push_back(reg8lIR);
							irList.push_back(reg8lIR);

							Value tempValue = *_regValue;
							Value Op2Value2 = tempValue;
							rstIR = CraeteStoreIR(BaseValue, _regValue, IR::OPR_STORE);
							rstIR->isHiddenRHS = true;
							irList.push_back(rstIR);
						}
					}

					// EA 가 상수가 아닌 경우 (EA가 알 수 없는 경우)
					// Store EA, Value
					else
					{
						// rstIR0 = Load BaseValue
						rstIR = CraeteStoreIR(BaseValue, _regValue, IR::OPR_STORE);
						irList.push_back(rstIR);
					}
				}
			}
		}
	}

	// SIB

	// Disp
	return;
}

void SaveMemoryValue(ZydisRegister zydisRegister, Value* _regValue, BYTE _size, vector<IR*>& irList)

{
	IR* rstIR0 = nullptr;
	IR* rstIR1 = nullptr;
	IR* rstIR2 = nullptr;
	IR* rstIR3 = nullptr;
	IR* rstIR = nullptr;

	Value * BaseValue;
	Value* Disp;
	Value* op3;
	IR* offset1;
	IR* offset2;
	IR* offset3;
	IR* offset4;

	Value* offsetImm1;
	Value* offsetImm2;
	Value* offsetImm3;
	Value* offsetImm4;

	Value* offset0Value;
	Value* offset1Value;
	Value* offset2Value;
	Value* offset3Value;

	// Base
	BaseValue = GetRegisterValue(zydisRegister, irList);

	// [Base]


	if (_size == 32)
	{
		// Base 레지스터의 Value가 상수인 경우 EA는 상수이므로 해당 EA에 대한 Memory Value Pool에 저장한다.
		if (dynamic_cast<IR*>(BaseValue)->opr == IR::OPR::OPR_BVV)
		{
			DWORD _memAddr = dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar;
			IR* reg8hh;
			IR* reg8hlIR;
			IR* reg8hIR;
			IR* reg8lIR;

			// 메모리에 저장할 Value가 상수인 경우
			if (dynamic_cast<IR*>(_regValue)->opr == IR::OPR::OPR_BVV)
			{
				printf("SaveMemoryValue -> Value is Constnat\n");
			}

			// 메모리에 저장할 Value가 상수가 아닌 경우
			else
			{
				if (MemValue.find(_memAddr + 3) != MemValue.end())
				{
					reg8hh = new IR(_regValue->Name, MemValue[_memAddr + 3].size(), IR::OPR::OPR_EXTRACT8HH, _regValue);
				}
				else
					reg8hh = new IR(IR::OPR::OPR_EXTRACT8HH, _regValue);

				irList.push_back(reg8hh);
				//offsetImm3 = GetImmValue(3, _size, irList);
				offset3 = CraeteBVVIR(_memAddr + 3, 32);
				//offset3 = CraeteBinaryIR(BaseValue, offsetImm3, IR::OPR::OPR_ADD);
				irList.push_back(offset3);
				rstIR3 = CraeteStoreIR(offset3, reg8hh, IR::OPR_STORE);
				irList.push_back(rstIR3);

				MemValue[_memAddr + 3].push_back(reg8hh);

				if (MemValue.find(_memAddr + 2) != MemValue.end())
				{
					reg8hlIR = new IR(_regValue->Name, MemValue[_memAddr + 2].size(), IR::OPR::OPR_EXTRACT8HL, _regValue);
				}
				else
					reg8hlIR = new IR(IR::OPR::OPR_EXTRACT8HL, _regValue);

				irList.push_back(reg8hlIR);
				offsetImm2 = GetImmValue(2, _size, irList);
				offset2 = CraeteBVVIR(_memAddr + 2, 32); //CraeteBinaryIR(BaseValue, offsetImm2, IR::OPR::OPR_ADD);
				irList.push_back(offset2);
				rstIR2 = CraeteStoreIR(offset2, reg8hlIR, IR::OPR_STORE);
				irList.push_back(rstIR2);

				MemValue[_memAddr + 2].push_back(reg8hlIR);
				//irList.push_back(reg8hlIR);

				if (MemValue.find(_memAddr + 1) != MemValue.end())
				{
					reg8hIR = new IR(_regValue->Name, MemValue[_memAddr + 1].size(), IR::OPR::OPR_EXTRACT8H, _regValue);
				}
				else
					reg8hIR = new IR(IR::OPR::OPR_EXTRACT8H, _regValue);

				irList.push_back(reg8hIR);
				offsetImm1 = GetImmValue(1, _size, irList);
				offset1 = CraeteBVVIR(_memAddr + 1, 32); //CraeteBinaryIR(BaseValue, offsetImm1, IR::OPR::OPR_ADD);
				irList.push_back(offset1);
				rstIR1 = CraeteStoreIR(offset1, reg8hIR, IR::OPR_STORE);
				irList.push_back(rstIR1);

				MemValue[_memAddr + 1].push_back(reg8hIR);
				//irList.push_back(reg8hIR);

				if (MemValue.find(_memAddr) != MemValue.end())
				{
					reg8lIR = new IR(_regValue->Name, MemValue[_memAddr].size(), IR::OPR::OPR_EXTRACT8L, _regValue);
				}
				else
					reg8lIR = new IR(IR::OPR::OPR_EXTRACT8L, _regValue);

				irList.push_back(reg8lIR);
				rstIR0 = CraeteStoreIR(BaseValue, reg8lIR, IR::OPR_STORE);
				irList.push_back(rstIR0);

				MemValue[_memAddr].push_back(reg8lIR);
				//irList.push_back(reg8lIR);

				//Value dasdg = *_regValue;
				//Value *Op2Test = new Value;
				//*Op2Test = dasdg;
				//Value tempValue = *_regValue;
				//rstIR = CraeteStoreIR(BaseValue, Op2Test, IR::OPR_STORE);
				//rstIR->isHiddenRHS = true;
				//irList.push_back(rstIR);
			}
		}

		// EA 가 상수가 아닌 경우 (EA가 알 수 없는 경우)
		// Store EA, Value
		else
		{
			// rstIR0 = Load BaseValue
			rstIR = CraeteStoreIR(BaseValue, _regValue, IR::OPR_STORE);
			rstIR->isHiddenRHS = true;
			irList.push_back(rstIR);
		}
	}



	return;
}

Value* GetMemoryValue(ZydisDecodedInstruction* decodedInstPtr, ZydisDecodedOperand* _decodedOperandPtr, vector<IR*>& irList)
{
	IR* rstIR0 = nullptr;
	IR* rstIR1 = nullptr;
	IR* rstIR2 = nullptr;
	IR* rstIR3 = nullptr;
	IR* rstIR = nullptr;

	Value * BaseValue;
	Value* Disp;
	Value* op3;
	IR* offset1;
	IR* offset2;
	IR* offset3;
	IR* offset4;

	Value* offsetImm1;
	Value* offsetImm2;
	Value* offsetImm3;
	Value* offsetImm4;

	Value* offset0Value;
	Value* offset1Value;
	Value* offset2Value;
	Value* offset3Value;

	// Base
	if (_decodedOperandPtr->mem.base != ZYDIS_REGISTER_NONE)
	{
		BaseValue = GetRegisterValue(_decodedOperandPtr->mem.base, irList);

		if (_decodedOperandPtr->mem.disp.has_displacement)
		{
			// [Base + Index*Scale + Disp]
			if (_decodedOperandPtr->mem.index != ZYDIS_REGISTER_NONE)
			{
				CraeteBinaryIR(BaseValue, Disp, IR::OPR_ADD);
			}

			// [Base + Disp]
			else
			{

			}
		}

		// Displacement가 없는경우
		else
		{
			// [Base + Index*Scale]
			if (_decodedOperandPtr->mem.index != ZYDIS_REGISTER_NONE)
			{

			}
			// [Base]
			else
			{
				if (decodedInstPtr->address_width == 32)
				{
					// Base 레지스터의 Value가 상수인 경우 EA는 상수이므로 해당 EA에 대한 Memory Value Pool에 저장한다.
					if (dynamic_cast<IR*>(BaseValue)->opr == IR::OPR::OPR_BVV)
					{
						//printf("[GetMemory] 메모리 주소가 상수인 경우 해당 메모리 주소에 ValuePool 존재하는지 확인\n");
						// 메모리 주소로부터 0부터 3바이트 주소에 대한 Memory Value Pool이 모두 존재해야 함
						if (MemValue.find(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar) != MemValue.end() &&
							MemValue.find(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 1) != MemValue.end() &&
							MemValue.find(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 2) != MemValue.end() &&
							MemValue.find(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 3) != MemValue.end())
						{
							//printf("test\n");
							offset0Value = MemValue[(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar)].back();
							offset1Value = MemValue[(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 1)].back();
							offset2Value = MemValue[(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 2)].back();
							offset3Value = MemValue[(dynamic_cast<ConstInt*>(dynamic_cast<IR*>(BaseValue)->Operands[0]->valuePtr)->intVar + 3)].back();
							rstIR = new IR(IR::OPR::OPR_CONCAT, offset0Value, offset1Value, offset2Value, offset3Value);
							irList.push_back(rstIR);
						}

						else
						{
							rstIR = CraeteLoadIR(BaseValue, IR::OPR_LOAD);
							irList.push_back(rstIR);
						}
					}

					// EA 가 상수가 아닌 경우 (EA가 알 수 없는 경우)
					// Store EA, Value
					else
					{
						// rstIR0 = Load BaseValue
						rstIR = CraeteLoadIR(BaseValue, IR::OPR_LOAD);
						irList.push_back(rstIR);
					}
				}
			}
		}
	}

	// SIB

	// Disp
	return rstIR;
}

Value* GetImmValue(ZydisDecodedOperand* _decodedOperandPtr, vector<IR*>& irList)
{
	IR* immValue;
	if (_decodedOperandPtr->size == 32)
	{
		immValue = CraeteBVVIR(_decodedOperandPtr->imm.value.u, 32);
		irList.push_back(immValue);
		return immValue;
	}

	if (_decodedOperandPtr->size == 16)
	{
		immValue = CraeteBVVIR(_decodedOperandPtr->imm.value.u, 16);
		irList.push_back(immValue);
		return immValue;
	}

	if (_decodedOperandPtr->size == 8)
	{
		immValue = CraeteBVVIR(_decodedOperandPtr->imm.value.u, 8);
		irList.push_back(immValue);
		return immValue;
	}
}

Value* GetImmValue(DWORD _immValue, BYTE _size, vector<IR*>& irList)
{
	IR* immValue;
	if (_size == 32)
	{
		immValue = CraeteBVVIR(_immValue, 32);
		//irList.push_back(immValue);
		return immValue;
	}

	if (_size == 16)
	{
		immValue = CraeteBVVIR(_immValue, 16);
		//irList.push_back(immValue);
		return immValue;
	}

	if (_size == 8)
	{
		immValue = CraeteBVVIR(_immValue, 8);
		//irList.push_back(immValue);
		return immValue;
	}
}

Value* GetOperand(ZydisDecodedInstruction* _decodedInstPtr, ZydisDecodedOperand* _decodedOperandPtr, vector<IR*>& irList)
{
	switch (_decodedOperandPtr->type)
	{
	case ZYDIS_OPERAND_TYPE_REGISTER:
		return GetRegisterValue(_decodedOperandPtr->reg.value, irList);
		break;

	case ZYDIS_OPERAND_TYPE_MEMORY:
		return GetMemoryValue(_decodedInstPtr, _decodedOperandPtr, irList);
		printf("Not Supported x86 Operand Type\n");
		break;

	case ZYDIS_OPERAND_TYPE_IMMEDIATE:
		return GetImmValue(_decodedOperandPtr, irList);
		break;
	}
}

void SetOperand(ZydisDecodedInstruction* _decodedInstPtr, ZydisDecodedOperand* _decodedOperandPtr, Value* _valPtr, vector<IR*>& irList)
{
	switch (_decodedOperandPtr->type)
	{
	case ZYDIS_OPERAND_TYPE_REGISTER:
		return SaveRegisterValue(_decodedOperandPtr->reg.value, _valPtr, _decodedOperandPtr->size, irList);
		break;

	case ZYDIS_OPERAND_TYPE_MEMORY:
		return SaveMemoryValue(_decodedInstPtr, _decodedOperandPtr, _valPtr, irList);
		printf("Not Supported x86 Operand Type\n");
		break;
	}
}

int CreateIR(ZydisDecodedInstruction* ptr_di, ZydisDecodedOperand* operandPTr, DWORD _offset)
{
	Value *Op1;
	Value *Op2;
	Value * RealOperand;

	IR* rst;
	IR* ExtractIR;
	IR* storeIR;

	vector<IR*> irList;

	switch (ptr_di->mnemonic)
	{
	case ZYDIS_MNEMONIC_ADD:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

		rst = CreateAddIR(Op1, Op2);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;

	case ZYDIS_MNEMONIC_MOV:
		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);
		SetOperand(ptr_di, &operandPTr[0], Op2, irList);
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;

	case ZYDIS_MNEMONIC_SAR:
		CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_SAR);
		return 1;
		break;

	case ZYDIS_MNEMONIC_ROL:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

												 // 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_ROL Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_ROL);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;

	case ZYDIS_MNEMONIC_OR:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

														 // 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_OR Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_OR);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;
	case ZYDIS_MNEMONIC_ADC:
		break;
	case ZYDIS_MNEMONIC_SBB:
		break;
	case ZYDIS_MNEMONIC_AND:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

		// 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_AND Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_AND);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		break;
	case ZYDIS_MNEMONIC_DAA:
		break;
	case ZYDIS_MNEMONIC_SUB:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

		rst = CreateSubIR(Op1, Op2);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		break;
	case ZYDIS_MNEMONIC_DAS:
		break;
	case ZYDIS_MNEMONIC_XOR:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

												 // 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_XOR Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_XOR);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;
	case ZYDIS_MNEMONIC_AAA:
		break;
	case ZYDIS_MNEMONIC_CMP:
		break;
	case ZYDIS_MNEMONIC_AAS:
		break;
	case ZYDIS_MNEMONIC_INC:
		Op1 = Op1 = GetOperand(ptr_di, &operandPTr[0], irList);
		Op2 = GetImmValue(1, operandPTr[0].size, irList);

		// 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("ZYDIS_MNEMONIC_INC Error (Operand is not matched)\n");
			return 0;
		}
		// Constant Folding 가능한 경우 CreateBinaryIR를 호출하지 않고 Imm Value 생성

		// Constant Folding 가능 조건이 아닌 경우 IR 생성
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_ADD);
		irList.push_back(rst);

		break;
	case ZYDIS_MNEMONIC_DEC:
		break;
	case ZYDIS_MNEMONIC_PUSH:
		// ESP		
		Op1 = GetRegisterValue(ZYDIS_REGISTER_ESP, irList);
		Op2 = GetImmValue(4, operandPTr[0].size, irList);
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_SUB);
		irList.push_back(rst);
		SaveRegisterValue(ZYDIS_REGISTER_ESP, rst, operandPTr[0].size, irList);

		RealOperand = GetOperand(ptr_di, &operandPTr[0], irList);
		SaveMemoryValue(ZYDIS_REGISTER_ESP, RealOperand, operandPTr[0].size, irList);

		IRList.insert(make_pair(_offset, irList));
		break;
	case ZYDIS_MNEMONIC_POP:
		break;
	case ZYDIS_MNEMONIC_PUSHAD:
		break;
	case ZYDIS_MNEMONIC_POPAD:
		break;
	case ZYDIS_MNEMONIC_ARPL:
		break;
	case ZYDIS_MNEMONIC_BOUND:
		break;
	case ZYDIS_MNEMONIC_IMUL:
		break;
	case ZYDIS_MNEMONIC_INSB:
		break;
	case ZYDIS_MNEMONIC_INSW:
		break;
	case ZYDIS_MNEMONIC_INSD:
		break;
	case ZYDIS_MNEMONIC_OUTSB:
		break;
	case ZYDIS_MNEMONIC_OUTSW:
		break;
	case ZYDIS_MNEMONIC_OUTSD:
		break;
	case ZYDIS_MNEMONIC_JO:
		break;
	case ZYDIS_MNEMONIC_JNO:
		break;
	case ZYDIS_MNEMONIC_JB:
		break;
	case ZYDIS_MNEMONIC_JNB:
		break;
	case ZYDIS_MNEMONIC_JZ:
		break;
	case ZYDIS_MNEMONIC_JNZ:
		break;
	case ZYDIS_MNEMONIC_JBE:
		break;
	case ZYDIS_MNEMONIC_JNBE:
		break;
	case ZYDIS_MNEMONIC_JS:
		break;
	case ZYDIS_MNEMONIC_JNS:
		break;
	case ZYDIS_MNEMONIC_JP:
		break;
	case ZYDIS_MNEMONIC_JNP:
		break;
	case ZYDIS_MNEMONIC_JL:
		break;
	case ZYDIS_MNEMONIC_JNL:
		break;
	case ZYDIS_MNEMONIC_JLE:
		break;
	case ZYDIS_MNEMONIC_JNLE:
		break;
	case ZYDIS_MNEMONIC_TEST:
		break;
	case ZYDIS_MNEMONIC_XCHG:
		break;
	case ZYDIS_MNEMONIC_LEA:
		break;
	case ZYDIS_MNEMONIC_CWD:
		break;
	case ZYDIS_MNEMONIC_CBW:
		break;
	case ZYDIS_MNEMONIC_CDQ:
		break;
	case ZYDIS_MNEMONIC_PUSHFD:
		break;
	case ZYDIS_MNEMONIC_POPFD:
		break;
	case ZYDIS_MNEMONIC_SAHF:
		break;
	case ZYDIS_MNEMONIC_LAHF:
		break;
	case ZYDIS_MNEMONIC_MOVSB:
		break;
	case ZYDIS_MNEMONIC_MOVSW:
		break;
	case ZYDIS_MNEMONIC_MOVSD:
		break;
	case ZYDIS_MNEMONIC_CMPSB:
	case ZYDIS_MNEMONIC_CMPSW:
	case ZYDIS_MNEMONIC_CMPSD:
		break;
	case ZYDIS_MNEMONIC_STOSB:
		break;
	case ZYDIS_MNEMONIC_STOSW:
		break;
	case ZYDIS_MNEMONIC_STOSD:
		break;
	case ZYDIS_MNEMONIC_LODSB:
		break;
	case ZYDIS_MNEMONIC_LODSW:
		break;
	case ZYDIS_MNEMONIC_LODSD:
		break;
	case ZYDIS_MNEMONIC_SCASB:
		break;
	case ZYDIS_MNEMONIC_SCASW:
		break;
	case ZYDIS_MNEMONIC_SCASD:
		break;
	case ZYDIS_MNEMONIC_SHL:
		break;
	case ZYDIS_MNEMONIC_SHR:
		break;
	case ZYDIS_MNEMONIC_RET:
		break;
	case ZYDIS_MNEMONIC_LES:
		break;
	case ZYDIS_MNEMONIC_LDS:
		break;
	case ZYDIS_MNEMONIC_ENTER:
		break;
	case ZYDIS_MNEMONIC_LEAVE:
		break;
	case ZYDIS_MNEMONIC_INT3:
		break;
	case ZYDIS_MNEMONIC_INT:
		break;
	case ZYDIS_MNEMONIC_INTO:
		break;
	case ZYDIS_MNEMONIC_IRETD:
		break;
	case ZYDIS_MNEMONIC_RCL:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

												 // 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_RCL Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_RCL);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;
	case ZYDIS_MNEMONIC_RCR:
		break;
	case ZYDIS_MNEMONIC_ROR:
		break;
	case ZYDIS_MNEMONIC_AAM:
		break;
	case ZYDIS_MNEMONIC_AAD:
		break;
	case ZYDIS_MNEMONIC_SALC:
		break;
	case ZYDIS_MNEMONIC_XLAT:
		break;
	case ZYDIS_MNEMONIC_LOOP:
		break;
	case ZYDIS_MNEMONIC_LOOPE:
		break;
	case ZYDIS_MNEMONIC_LOOPNE:
		break;
	case ZYDIS_MNEMONIC_JCXZ:
		break;
	case ZYDIS_MNEMONIC_IN:
		break;
	case ZYDIS_MNEMONIC_OUT:
		break;
	case ZYDIS_MNEMONIC_CALL:
		break;
	case ZYDIS_MNEMONIC_JMP:
		break;
	case ZYDIS_MNEMONIC_CMC:
		break;
	case ZYDIS_MNEMONIC_NOT:
		break;
	case ZYDIS_MNEMONIC_NEG:
		break;
	case ZYDIS_MNEMONIC_MUL:
		break;
	case ZYDIS_MNEMONIC_DIV:
		break;
	case ZYDIS_MNEMONIC_IDIV:
		break;
	case ZYDIS_MNEMONIC_CLC:
		break;
	case ZYDIS_MNEMONIC_STC:
		break;
	case ZYDIS_MNEMONIC_CLI:
		break;
	case ZYDIS_MNEMONIC_STI:
		break;
	case ZYDIS_MNEMONIC_CLD:
		break;
	case ZYDIS_MNEMONIC_STD:
		break;

	case ZYDIS_MNEMONIC_LLDT:
		break;
	case ZYDIS_MNEMONIC_SLDT:
		break;
	case ZYDIS_MNEMONIC_VERR:
		break;
	case ZYDIS_MNEMONIC_VERW:
		break;
	case ZYDIS_MNEMONIC_LGDT:
		break;
	case ZYDIS_MNEMONIC_SGDT:
		break;
	case ZYDIS_MNEMONIC_LMSW:
		break;
	case ZYDIS_MNEMONIC_SMSW:
		break;
	case ZYDIS_MNEMONIC_LAR:
		break;
	case ZYDIS_MNEMONIC_LSL:
		break;
	case ZYDIS_MNEMONIC_CLTS:
		break;
	case ZYDIS_MNEMONIC_INVD:
		break;
	case ZYDIS_MNEMONIC_WBINVD:
		break;
	case ZYDIS_MNEMONIC_WRMSR:
		break;
	case ZYDIS_MNEMONIC_RDTSC:
		break;
	case ZYDIS_MNEMONIC_RDMSR:
		break;
	case ZYDIS_MNEMONIC_RDPMC:
		break;
	case ZYDIS_MNEMONIC_SYSENTER:
		break;
	case ZYDIS_MNEMONIC_SYSEXIT:
		break;
	case ZYDIS_MNEMONIC_CMOVO:
		break;
	case ZYDIS_MNEMONIC_CMOVNO:
		break;
	case ZYDIS_MNEMONIC_CMOVB:
		break;
	case ZYDIS_MNEMONIC_CMOVNB:
		break;
	case ZYDIS_MNEMONIC_CMOVZ:
		break;
	case ZYDIS_MNEMONIC_CMOVNZ:
		break;
	case ZYDIS_MNEMONIC_CMOVBE:
		break;
	case ZYDIS_MNEMONIC_CMOVNBE:
		break;
	case ZYDIS_MNEMONIC_CMOVS:
		break;
	case ZYDIS_MNEMONIC_CMOVNS:
		break;
	case ZYDIS_MNEMONIC_CMOVP:
		break;
	case ZYDIS_MNEMONIC_CMOVNP:
		break;
	case ZYDIS_MNEMONIC_CMOVL:
		break;
	case ZYDIS_MNEMONIC_CMOVNL:
		break;
	case ZYDIS_MNEMONIC_CMOVLE:
		break;
	case ZYDIS_MNEMONIC_CMOVNLE:
		break;
	case ZYDIS_MNEMONIC_SETO:
		break;
	case ZYDIS_MNEMONIC_CPUID:
		break;
	case ZYDIS_MNEMONIC_BSF:
	case ZYDIS_MNEMONIC_BSR:
		break;
	case ZYDIS_MNEMONIC_BSWAP:
		break;
	case ZYDIS_MNEMONIC_BT:
	case ZYDIS_MNEMONIC_BTS:
	case ZYDIS_MNEMONIC_BTR:
	case ZYDIS_MNEMONIC_BTC:
		Op1 = GetOperand(ptr_di, &operandPTr[0], irList); // x86 오퍼랜드를 Get하는 IR을 생성한다.

		Op2 = GetOperand(ptr_di, &operandPTr[1], irList);// x86 오퍼랜드를 Get하는 IR을 생성한다.

												 // 두 개의 오퍼랜드는 동일해야 한다.
		if (Op1->Size != Op2->Size)
		{
			printf("GenerateOPR_RCL Error (Operand is not matched)\n");
			return 0;
		}
		rst = CraeteBinaryIR(Op1, Op2, IR::OPR::OPR_RCL);
		irList.push_back(rst);

		// EFLAG 관련 IR 추가

		SaveRegisterValue(operandPTr[0].reg.value, rst, operandPTr[0].size, irList); // x86 오퍼랜드를 Set하는 IR을 생성한다.
		IRList.insert(make_pair(_offset, irList));
		return 1;
		break;
	case ZYDIS_MNEMONIC_MOVSX:
		break;
	case ZYDIS_MNEMONIC_MOVZX:
		break;
	case ZYDIS_MNEMONIC_CMPXCHG:
		break;
	case ZYDIS_MNEMONIC_CMPXCHG8B:
		break;
	case ZYDIS_MNEMONIC_COMISD:
		break;
	case ZYDIS_MNEMONIC_COMISS:
		break;
	case ZYDIS_MNEMONIC_FCMOVB:
		break;
	case ZYDIS_MNEMONIC_FCOMI:
		break;
	case ZYDIS_MNEMONIC_FCOMIP:
		break;
	case ZYDIS_MNEMONIC_FUCOMI:
		break;
	case ZYDIS_MNEMONIC_FUCOMIP:
		break;
	case ZYDIS_MNEMONIC_HLT:
		break;
	case ZYDIS_MNEMONIC_INVLPG:
		break;
	case ZYDIS_MNEMONIC_UCOMISD:
		break;
	case ZYDIS_MNEMONIC_UCOMISS:
		break;
	case ZYDIS_MNEMONIC_IRET:
		break;
	case ZYDIS_MNEMONIC_LSS:
		break;
	case ZYDIS_MNEMONIC_LFS:
		break;
	case ZYDIS_MNEMONIC_LGS:
		break;
	case ZYDIS_MNEMONIC_LIDT:
		break;
	case ZYDIS_MNEMONIC_LTR:
		break;
	case ZYDIS_MNEMONIC_NOP:
		break;
	case ZYDIS_MNEMONIC_SHLD:
		break;
	case ZYDIS_MNEMONIC_SHRD:
		break;
	case ZYDIS_MNEMONIC_XADD:
		break;
		//	break;
		//case ZYDIS_MNEMONIC_MOV:
		//	//CreateMovIR(UnaryOp::UnaryOps::Mov, ptr_di, _offset);
		//	GenerateMovIR(Op1, Op2, _offset);
		//	return 1;
		//	break;
		//case ZYDIS_MNEMONIC_MOVZX:
		//	//CreateMovIR(UnaryOp::UnaryOps::Mov, ptr_di, _offset);
		//	GenerateMovIR(Op1, Op2, _offset);
		//	return 1;
		//	break;
		//case ZYDIS_MNEMONIC_MOVSX:
		//	//CreateMovIR(UnaryOp::UnaryOps::Mov, ptr_di, _offset);
		//	GenerateMovIR(Op1, Op2, _offset);
		//	return 1;
		//	break;
		//case ZYDIS_MNEMONIC_CMOVNBE:
		//	//CreateMovIR(UnaryOp::UnaryOps::Mov, ptr_di, _offset);
		//	GenerateMovIR(Op1, Op2, _offset);
		//	return 1;
		//	break;
		//case ZYDIS_MNEMONIC_PUSH:
		//	CreatePush32IR(UnaryOp::UnaryOps::Mov, ptr_di, Op1, _offset);
		//	return 2;
		//	break;
		//case ZYDIS_MNEMONIC_PUSHFD:
		//	CreatePushfd32IR(UnaryOp::UnaryOps::Mov, ptr_di, _offset);
		//	return 2;
		//	break;
		//case ZYDIS_MNEMONIC_POP:
		//	CreatePop32IR(UnaryOp::UnaryOps::Mov, ptr_di, Op1, _offset);
		//	return 2;
		//	break;
		//case ZYDIS_MNEMONIC_XCHG:
		//	GenerateXchgIR(Op1, Op2, _offset);
		//	return 3;
		//	break;
	default:
		printf("Not Implemented Instruction :%s\n", ZydisMnemonicGetString(ptr_di->mnemonic));
		return 0;
		break;
	}
}

void printIR(IR* _irPtr)
{
	if (dynamic_cast<ConstInt*>(_irPtr))
	{
		if (dynamic_cast<ConstInt*>(_irPtr)->opr == IR::OPR::OPR_BVV)
		{

			printf("%x\n", dynamic_cast<ConstInt*>(_irPtr)->intVar);
		}
	}
	switch (_irPtr->opr)
	{
	case IR::OPR::OPR_CONCAT:
		if (_irPtr->Operands.size() == 3)
		{
			printf("%s = CONCAT %s %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str(), _irPtr->Operands[2]->valuePtr->Name.c_str());
		}

		else if (_irPtr->Operands.size() == 4)
		{
			printf("%s = CONCAT %s %s %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str(), _irPtr->Operands[2]->valuePtr->Name.c_str(), _irPtr->Operands[3]->valuePtr->Name.c_str());
		}
		break;
	case IR::OPR::OPR_BVV:
		printf("%s =  BVV(%x, %d)\n", _irPtr->Name.c_str(), dynamic_cast<ConstInt*>(_irPtr->Operands[0]->valuePtr)->intVar, dynamic_cast<ConstInt*>(_irPtr->Operands[1]->valuePtr)->intVar);
		break;
	case IR::OPR::OPR_EXTRACT16H:
		printf("%s = OPR_EXTRACT16H %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_EXTRACT8HH:
		printf("%s = OPR_EXTRACT8HH %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_EXTRACT8HL:
		printf("%s = OPR_EXTRACT8HL %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_EXTRACT8H:
		printf("%s = OPR_EXTRACT8H %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_EXTRACT8L:
		printf("%s = OPR_EXTRACT8L %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_ADD:
		printf("%s = OPR_ADD %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_SUB:
		printf("%s = OPR_SUB %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_AND:
		printf("%s = OPR_AND %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_XOR:
		printf("%s = OPR_XOR %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_ROL:
		printf("%s = OPR_ROL %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_SAR:
		printf("%s = OPR_SAR %s %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_LOAD:
		printf("%s = LOAD %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_STORE:
		printf("STORE %s %s\n", _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	}
}

void CheckTaintIR(IR* _irPtr)
{
	_irPtr->isTainted = false;

	if (dynamic_cast<ConstInt*>(_irPtr))
	{
		if (dynamic_cast<ConstInt*>(_irPtr)->opr == IR::OPR::OPR_BVV)
		{

			printf("%x\n", dynamic_cast<ConstInt*>(_irPtr)->intVar);
		}
	}
	switch (_irPtr->opr)
	{
	case IR::OPR::OPR_CONCAT:
		for (int i = 0; i < _irPtr->Operands.size(); i++)
		{
			if (_irPtr->Operands[0]->valuePtr->isTainted)
			{
				_irPtr->isTainted = true;
				//return;
			}
		}
		break;
	case IR::OPR::OPR_BVV:
		//printf("%s =  BVV(%x, %d)\n", _irPtr->Name.c_str(), dynamic_cast<ConstInt*>(_irPtr->Operands[0]->valuePtr)->intVar, dynamic_cast<ConstInt*>(_irPtr->Operands[1]->valuePtr)->intVar);
		break;
	case IR::OPR::OPR_EXTRACT16H:
	case IR::OPR::OPR_EXTRACT8HH:
	case IR::OPR::OPR_EXTRACT8HL:
	case IR::OPR::OPR_EXTRACT8H:
	case IR::OPR::OPR_EXTRACT8L:
		for (int i = 0; i < _irPtr->Operands.size(); i++)
		{
			if (_irPtr->Operands[0]->valuePtr->isTainted)
			{
				_irPtr->isTainted = true;
				//return;
			}
		}
		break;
	case IR::OPR::OPR_ADD:
	case IR::OPR::OPR_SUB:
	case IR::OPR::OPR_XOR:
	case IR::OPR::OPR_ROL:
	case IR::OPR::OPR_SAR:
		for (int i = 0; i < _irPtr->Operands.size(); i++)
		{
			if (_irPtr->Operands[0]->valuePtr->isTainted)
			{
				_irPtr->isTainted = true;
				//return;
			}
		}
		break;
	case IR::OPR::OPR_LOAD:
		//printf("%s = LOAD %s\n", _irPtr->Name.c_str(), _irPtr->Operands[0]->valuePtr->Name.c_str());
		break;
	case IR::OPR::OPR_STORE:
		//printf("STORE %s %s\n", _irPtr->Operands[0]->valuePtr->Name.c_str(), _irPtr->Operands[1]->valuePtr->Name.c_str());
		break;
	}

	if (_irPtr->isTainted)
	{
		printf("Taint IR %s\n", _irPtr->Name.c_str());
	}
}

void initReg()
{	
	/*
		1. EAX Sym Value 생성
		2. z3Context->bv_const("EAX_SYM", 32)
		3. 16H, 8H, 8L 에 Extract 하여 저장
	*/

	Value* eaxValue = new Value("EAX",0);
	eaxValue->ast = std::make_shared<BTreeNode>(MakeBTreeNode(eaxValue->Name));
	eaxValue->ast->m_NodeType = NT_SYMVAR;
	eaxValue->Size = 32;
	z3::expr* z3eax = new z3::expr(z3Context->bv_const("EAX(0)", 32));
	z3Equation = new z3::expr(*z3eax);
	symbolExprMap["EAX(0)"] = z3eax;

	IR * reg16hIR = new IR("EAX16H", RegValue[REG_EAX_16H].size(), IR::OPR::OPR_EXTRACT16H, eaxValue);
	reg16hIR->Size = 16;
	RegValue[REG_EAX_16H].push_back(reg16hIR);

	IR* regEax8hIR = new IR("EAX8H", RegValue[REG_EAX_8H].size(), IR::OPR::OPR_EXTRACT8H, eaxValue);
	regEax8hIR->Size = 8;
	RegValue[REG_EAX_8H].push_back(regEax8hIR);

	IR* regEax8lIR = new IR("EAX8L", RegValue[REG_EAX_8L].size(), IR::OPR::OPR_EXTRACT8L, eaxValue);
	regEax8lIR->Size = 8;
	RegValue[REG_EAX_8L].push_back(regEax8lIR);

	//Value* eax_16h = new Value("EAX.16H", RegValue[REG_EAX_16H].size());//CraeteBVVIR(0xaaaa, 16);
	//eax_16h->ast = std::make_shared<BTreeNode>(MakeBTreeNode(eax_16h->Name));
	//eax_16h->ast->m_NodeType = NT_SYMVAR;
	//eax_16h->Size = 16;
	//RegValue[REG_EAX_16H].push_back(eax_16h);
	//z3::expr*  z3eax_16hVar = new z3::expr(z3Context->bv_const("EAX.16H", 16));
	//z3Equation = new z3::expr(*z3eax_16hVar);
	//symbolExprMap["EAX.16H"] = z3eax_16hVar;
	//eax_16h->isTainted = true;

	//Value* eax_lh = new Value("EAX.8H", RegValue[REG_EAX_8H].size()); // CraeteBVVIR(0xa2, 8);
	//eax_lh->ast = std::make_shared<BTreeNode>(MakeBTreeNode(eax_lh->Name));
	//eax_lh->ast->m_NodeType = NT_SYMVAR;
	//eax_lh->Size = 8;
	//RegValue[REG_EAX_8H].push_back(eax_lh);
	//z3::expr* z3eax_8hVar = new z3::expr(z3Context->bv_const("EAX.8H", 8));
	//symbolExprMap["EAX.8H"] = z3eax_8hVar;
	//eax_lh->isTainted = true;

	//Value* eax_ll = new Value("EAX.8L", RegValue[REG_EAX_8L].size()); //CraeteBVVIR(0xa1, 8);
	//eax_ll->Size = 8;
	//eax_ll->ast = std::make_shared<BTreeNode>(MakeBTreeNode(eax_ll->Name));
	//eax_ll->ast->m_NodeType = NT_SYMVAR;
	//RegValue[REG_EAX_8L].push_back(eax_ll);
	//z3::expr* z3eax_8lVar = new z3::expr(z3Context->bv_const("EAX.8L", 8));
	//symbolExprMap["EAX.8L"] = z3eax_8lVar;
	//eax_ll->isTainted = true;

	Value* ebxValue = new Value("EBX", 0);
	ebxValue->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ebxValue->Name));
	ebxValue->ast->m_NodeType = NT_SYMVAR;
	ebxValue->Size = 32;
	z3::expr* z3ebx = new z3::expr(z3Context->bv_const("EBX(0)", 32));
	//z3Equation = new z3::expr(*z3ecx);
	symbolExprMap["EBX(0)"] = z3ebx;


	IR* regEbx16hIR = new IR("EBX16H", RegValue[REG_EBX_16H].size(), IR::OPR::OPR_EXTRACT16H, ebxValue);
	regEbx16hIR->Size = 16;
	RegValue[REG_EBX_16H].push_back(regEbx16hIR);

	IR* regEbx8hIR = new IR("EBX8H", RegValue[REG_EBX_8H].size(), IR::OPR::OPR_EXTRACT8H, ebxValue);
	regEbx8hIR->Size = 8;
	RegValue[REG_EBX_8H].push_back(regEbx8hIR);

	IR* regEbx8lIR = new IR("EBX8L", RegValue[REG_EBX_8L].size(), IR::OPR::OPR_EXTRACT8L, ebxValue);
	regEbx8lIR->Size = 8;
	RegValue[REG_EBX_8L].push_back(regEbx8lIR);

	//Value* ebx_16h = new Value("EBX.16H", RegValue[REG_EBX_16H].size()); // CraeteBVVIR(0xbbbb, 16);
	//ebx_16h->Size = 16;
	//ebx_16h->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ebx_16h->Name));
	//ebx_16h->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ebx_16hVar = new z3::expr(z3Context->bv_const("EBX.16H", 16));
	//symbolExprMap["EBX.16H"] = z3ebx_16hVar;
	//RegValue[REG_EBX_16H].push_back(ebx_16h);

	//Value* ebx_lh = new Value("EBX.8H", RegValue[REG_EBX_8H].size()); //CraeteBVVIR(0xb1, 8);
	//ebx_lh->Size = 8;
	//ebx_lh->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ebx_lh->Name));
	//ebx_lh->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ebx_8hVar = new z3::expr(z3Context->bv_const("EBX.8H", 8));
	//symbolExprMap["EBX.8H"] = z3ebx_8hVar;
	//RegValue[REG_EBX_8H].push_back(ebx_lh);

	//Value* ebx_ll = new Value("EBX.8L", RegValue[REG_EBX_8L].size()); //CraeteBVVIR(0xb2, 8);
	//ebx_ll->Size = 8;
	//ebx_ll->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ebx_ll->Name));
	//ebx_ll->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ebx_8lVar = new z3::expr(z3Context->bv_const("EBX.8L", 8));
	//symbolExprMap["EBX.8L"] = z3ebx_8lVar;
	//RegValue[REG_EBX_8L].push_back(ebx_ll);

	Value* ecxValue = new Value("ECX", 0);
	ecxValue->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ecxValue->Name));
	ecxValue->ast->m_NodeType = NT_SYMVAR;
	ecxValue->Size = 32;
	z3::expr* z3ecx = new z3::expr(z3Context->bv_const("ECX(0)", 32));
	//z3Equation = new z3::expr(*z3ecx);
	symbolExprMap["ECX(0)"] = z3ecx;


	IR* regEcx16hIR = new IR("ECX16H", RegValue[REG_ECX_16H].size(), IR::OPR::OPR_EXTRACT16H, ecxValue);
	regEcx16hIR->Size = 16;
	RegValue[REG_ECX_16H].push_back(regEcx16hIR);

	IR* regEcx8hIR = new IR("ECX8H", RegValue[REG_ECX_8H].size(), IR::OPR::OPR_EXTRACT8H, ecxValue);
	regEcx8hIR->Size = 8;
	RegValue[REG_ECX_8H].push_back(regEcx8hIR);

	IR* regEcx8lIR = new IR("ECX8L", RegValue[REG_ECX_8L].size(), IR::OPR::OPR_EXTRACT8L, ecxValue);
	regEcx8lIR->Size = 8;
	RegValue[REG_ECX_8L].push_back(regEcx8lIR);

	//Value* ECX_16h = new Value("ECX.16H", RegValue[REG_ECX_16H].size());
	//ECX_16h->Size = 16;
	//ECX_16h->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ECX_16h->Name));
	//ECX_16h->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ecx_16hVar = new z3::expr(z3Context->bv_const("ECX.16H", 16));
	//symbolExprMap["ECX.16H(0)"] = z3ecx_16hVar;
	//RegValue[REG_ECX_16H].push_back(ECX_16h);

	//Value* ECX_lh = new Value("ECX.8H", RegValue[REG_ECX_8H].size());
	//ECX_lh->Size = 8;
	//ECX_lh->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ECX_lh->Name));
	//ECX_lh->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ecx_8hVar = new z3::expr(z3Context->bv_const("ECX.8H", 8));
	//symbolExprMap["ECX.8H(0)"] = z3ecx_8hVar;
	//RegValue[REG_ECX_8H].push_back(ECX_lh);

	//Value* ECX_ll = new Value("ECX.8L", RegValue[REG_ECX_8L].size());
	//ECX_ll->Size = 8;
	//ECX_ll->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ECX_ll->Name));
	//ECX_ll->ast->m_NodeType = NT_SYMVAR;
	//z3::expr* z3ecx_8lVar = new z3::expr(z3Context->bv_const("ECX.8L", 8));
	//symbolExprMap["ECX.8L(0)"] = z3ecx_8lVar;
	//RegValue[REG_ECX_8L].push_back(ECX_ll);

	Value* edxValue = new Value("EDX", 0);
	edxValue->ast = std::make_shared<BTreeNode>(MakeBTreeNode(edxValue->Name));
	edxValue->ast->m_NodeType = NT_SYMVAR;
	edxValue->Size = 32;
	z3::expr* z3edx = new z3::expr(z3Context->bv_const("EDX(0)", 32));
	//z3Equation = new z3::expr(*z3ecx);
	symbolExprMap["EDX(0)"] = z3edx;


	IR* regEdx16hIR = new IR("EDX16H", RegValue[REG_EDX_16H].size(), IR::OPR::OPR_EXTRACT16H, ecxValue);
	regEdx16hIR->Size = 16;
	RegValue[REG_EDX_16H].push_back(regEdx16hIR);

	IR* regEdx8hIR = new IR("EDX8H", RegValue[REG_EDX_8H].size(), IR::OPR::OPR_EXTRACT8H, ecxValue);
	regEdx8hIR->Size = 8;
	RegValue[REG_EDX_8H].push_back(regEdx8hIR);

	IR* regEdx8lIR = new IR("EDX8L", RegValue[REG_EDX_8L].size(), IR::OPR::OPR_EXTRACT8L, ecxValue);
	regEdx8lIR->Size = 8;
	RegValue[REG_EDX_8L].push_back(regEdx8lIR);

	//Value* EDX_16h = new Value("EDX.16H", RegValue[REG_EDX_16H].size());
	//EDX_16h->ast = std::make_shared<BTreeNode>(MakeBTreeNode("EDX_16h"));
	//EDX_16h->Size = 16;
	//RegValue[REG_EDX_16H].push_back(EDX_16h);

	//Value* EDX_lh = new Value("EDX.8H", RegValue[REG_EDX_8H].size());
	//EDX_lh->ast = std::make_shared<BTreeNode>(MakeBTreeNode("EDX_lh"));
	//EDX_lh->Size = 8;
	//RegValue[REG_EDX_8H].push_back(EDX_lh);

	//Value* EDX_ll = new Value("EDX.8L", RegValue[REG_EDX_8L].size());
	//EDX_ll->ast = std::make_shared<BTreeNode>(MakeBTreeNode("EDX_ll"));
	//EDX_ll->Size = 8;
	//RegValue[REG_EDX_8L].push_back(EDX_ll);

	Value* ESI_16h = new Value("ESI.16H", RegValue[REG_ESI_16H].size());//CraeteBVVIR(0xeeee, 16);
	ESI_16h->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ESI_16h->Name));
	RegValue[REG_ESI_16H].push_back(ESI_16h);

	Value* ESI_lh = new Value("EDX.8H", RegValue[REG_ESI_8H].size());//CraeteBVVIR(0xe2, 8);
	ESI_lh->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ESI_lh->Name));
	RegValue[REG_ESI_8H].push_back(ESI_lh);

	Value* ESI_ll = new Value("ESI.8L", RegValue[REG_ESI_8L].size());//CraeteBVVIR(0xe1, 8);
	ESI_ll->ast = std::make_shared<BTreeNode>(MakeBTreeNode(ESI_ll->Name));
	RegValue[REG_ESI_8L].push_back(ESI_ll);

	Value* EDI_16h = CraeteBVVIR(0xffff, 16);
	RegValue[REG_EDI_16H].push_back(EDI_16h);

	Value* EDI_lh = CraeteBVVIR(0xf2, 8);
	RegValue[REG_EDI_8H].push_back(EDI_lh);

	Value* EDI_ll = CraeteBVVIR(0xf1, 8);
	RegValue[REG_EDI_8L].push_back(EDI_ll);

	Value* EBP_16h = new Value("EBP.16H", RegValue[REG_EBP_16H].size());
	EBP_16h->Size = 16;
	RegValue[REG_EBP_16H].push_back(EBP_16h);

	Value* EBP_lh = new Value("EBP.8H", RegValue[REG_EBP_8H].size());
	EBP_lh->Size = 8;
	RegValue[REG_EBP_8H].push_back(EBP_lh);

	Value* EBP_ll = new Value("EBP.8L", RegValue[REG_EBP_8L].size());
	EBP_ll->Size = 8;
	RegValue[REG_EBP_8L].push_back(EBP_ll);

	Value* ESP_16h = CraeteBVVIR(0x9000, 16);
	ESP_16h->Size = 16;
	RegValue[REG_ESP_16H].push_back(ESP_16h);

	Value* ESP_lh = CraeteBVVIR(0x10, 8);
	ESP_lh->Size = 8;
	RegValue[REG_ESP_8H].push_back(ESP_lh);

	Value* ESP_ll = CraeteBVVIR(0x00, 8);
	ESP_ll->Size = 8;
	RegValue[REG_ESP_8L].push_back(ESP_ll);
}

int main()
{
	z3Context = new z3::context;
	DWORD offset = 0;
	ZydisDecodedInstruction g_inst;
	ZydisDecoder decoder;
	BYTE buf[] = //{ 0x9C,0x56,0x52,0xC7,0x04,0x24,0xA2,0x53,0x10,0x52,0x89,0x1C,0x24,0x68,0xB6,0x01,0x00,0x00,0x5B,0x89,0x5C,0x24,0x04,0x5B,0x51,0x56,0xBE,0xF2,0x63,0x78,0x05,0x89,0x74,0x24,0x04,0x5E,0x89,0x0C,0x24,0x52,0xBA,0x2C,0xC3,0x48,0x00,0x89,0x54,0x24,0x04,0x5A,0x51,0x89,0xE1,0x81,0xC1,0x04,0x00,0x00,0x00,0x81,0xE9,0x04,0x00,0x00,0x00,0x87,0x0C,0x24,0x5C,0x89,0x1C,0x24,0x89,0x2C,0x24,0x89,0x04,0x24,0x57,0x55,0x68,0x14,0x17,0x24,0x48,0x5D,0x89,0x6C,0x24,0x04,0x5D,0x89,0x3C,0x24,0x89,0x1C,0x24,0xFF,0x74,0x24,0x10,0xFF,0x34,0x24,0x8B,0x04,0x24,0x55,0x89,0xE5,0x81,0xC5,0x04,0x00,0x00,0x00,0x83,0xC5,0x04,0x87,0x2C,0x24,0x5C,0x81,0xC4,0x04,0x00,0x00,0x00,0xFF,0x74,0x24,0x08,0xFF,0x34,0x24,0x5B,0x81,0xC4,0x04,0x00,0x00,0x00,0x89,0x44,0x24,0x08,0x89,0x5C,0x24,0x10,0x8B,0x1C,0x24,0x52,0x81,0xEC,0x04,0x00,0x00,0x00,0x89,0x24,0x24,0x81,0x04,0x24,0x04,0x00,0x00,0x00,0x5A,0x81,0xC2,0x04,0x00,0x00,0x00,0x81,0xC2,0x04,0x00,0x00,0x00,0x87,0x14,0x24,0x8B,0x24,0x24,0xFF,0x34,0x24,0x58,0x57,0x57,0x89,0x24,0x24,0x81,0x04,0x24,0x04,0x00,0x00,0x00,0x5F,0x81,0xC7,0x04,0x00,0x00,0x00,0x83,0xC7,0x04,0x87,0x3C,0x24,0x5C,0xE9,0xFC,0xB0,0xE0 };
						 //{ 0x55,0xb9,0x12,0x34,0x43,0x87,0x89,0xd0,0x66,0x89,0xd9,0x89,0xca,0x89,0xc1,0x51, 0x87,0xc8,0x89,0xd1 };//{ 0x56,0x52,0xC7,0x04,0x24,0xA2,0x53,0x10,0x52,0x89,0x1C,0x24,0x68,0xB6,0x01,0x00,0x00,0x5B,0x89,0x5C,0x24,0x04,0x5B,0x51,0x56,0xBE,0xF2,0x63,0x78,0x05,0x89,0x74,0x24,0x04,0x5E,0x89,0x0C,0x24,0x52,0xBA,0x2C,0xC3,0x48,0x00,0x89,0x54,0x24,0x04,0x5A,0x51,0x89,0xE1,0x81,0xC1,0x04,0x00,0x00,0x00,0x81,0xE9,0x04,0x00,0x00,0x00 };//{ 0x89,0xca,0x66,0x89,0xda,0x88,0xc6, 0x01,0x04,0x24, 0x03,0x54,0x24,0x04, 0x01,0xf3,0x01,0xd9, 0x01,0x04,0x24,0x01,0xf3,0x89,0x1c,0x24, 0x89,0x3c,0x24,0x03,0x34,0x24,0x89,0x0c,0x24 };
						 //VMP{ 0x8B,0xC4,0xD0,0xC9,0x8B,0xC3,0xFE,0xC9,0x66,0x0F,0xAB,0xD8,0x0F,0xBF,0xC7,0x32,0xD9,0x2A,0xE4,0xF6,0xC5,0xAE,0x8B,0x04,0x0C };
						 //, 0xD2, 0xE5, 0xC1, 0xF9, 0xB4, 0xD2, 0xC1, 0x8D, 0xB6, 0xFC, 0xFF, 0xFF, 0xFF, 0x89, 0x06, 0x66, 0xD3, 0xC9, 0x2A, 0xEA, 0x8D, 0xAD, 0xFC, 0xFF, 0xFF, 0xFF, 0x0F, 0xBA, 0xF1, 0xBF, 0x66, 0x0F, 0xC9, 0x8B, 0x4C, 0x25, 0x00, 0x66, 0x3B, 0xE6, 0x33, 0xCB, 0xF5, 0xF8, 0xD1, 0xC9, 0xF9, 0x66, 0xF7, 0xC5, 0x50, 0x57, 0x81, 0xC1, 0xBC, 0x6D, 0x46, 0x3D, 0xF8, 0xC1, 0xC1, 0x02, 0xF7, 0xD9, 0x81, 0xF1, 0xE4, 0x0A, 0x81, 0x11};
						 //{0x0f,0x47,0xcb,0x0f,0xb7,0xcd,0x0f,0xbf,0xce,0xd3,0xf1,0xd3,0xf9,0x8a,0xe8,0x81,0xed,0x04,0x00,0x00,0x00,0x8b,0x4c,0x25,0x00};
	//{ 0x89, 0xD8,0x89, 0xd9, 0x01,0xc1,0x89,0xd0, 0x01,0xce, 0x00,0xed, 0x89,0xc1, 0x89,0xc6,0x01,0xc1 };
	//{ 0xD2,0xC8,0x89,0xe8,0x89,0xcd,0x33,0xC3,0xF8,0x05,0x92,0x45,0x6F,0x67,0xF9,0xF7,0xD0,0x35,0x3D,0x21,0x33,0x61,0xF8,0x89,0xD8,0xF8,0x35,0xEF,0x46,0x71,0x55,0xF9,0x33,0xD8,0x81,0xFE,0x9F,0x1B,0xBA,0x6C,0x03,0xF8 };
	//{ 0x2B,0xC8,0x33,0xCA,0x03,0xC3,0x23,0xC2,0x03,0xC1 };
	{ 0xba, 0x84,0xaf,0x37,0x11,0x23,0xc8,0x2B,0xC8,0x33,0xCA,0x03,0xC3,0x22,0xC6,0x02,0xE6 };
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_COMPAT_32, ZYDIS_STACK_WIDTH_32);

	initReg();

	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
	char buffer[256];
	int length = 0;
	int irIdx = 0;

	ZydisDecodedOperand DecodedOperand[5];
	//for (int i = 0; length < sizeof(buf); i++)
	//{
	//	ZydisDecoderDecodeFull(&decoder, (buf + length), 15, &g_inst, DecodedOperand,
	//		ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY);
	//	printf("[%p] ", 0x401000 + length);
	//	ZydisFormatterFormatInstruction(&formatter, &g_inst, DecodedOperand, g_inst.operand_count_visible, buffer, sizeof(buffer), ZYDIS_RUNTIME_ADDRESS_NONE);
	//	puts(buffer);
	//	length += g_inst.length;
	//}

	length = 0;
	for (int i = 0; length < sizeof(buf); i++)
	{
		ZydisDecoderDecodeFull(&decoder, (buf + length), 15, &g_inst, DecodedOperand,
			ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY);
		printf("CreateIR [%p] ", 0x401000 + length);
		ZydisFormatterFormatInstruction(&formatter, &g_inst, DecodedOperand, g_inst.operand_count_visible, buffer, sizeof(buffer), ZYDIS_RUNTIME_ADDRESS_NONE);
		puts(buffer);
		irIdx += CreateIR(&g_inst, DecodedOperand, 0x401000 + length);
		length += g_inst.length;
	}

	printf("MemValue.size() :%d\n", MemValue.rbegin()->second.size());
	system("pause");

	int cnt = 0;
	for (auto it1 : IRList)
	{
		printf("--------------------------------\n");
		for (auto it : it1.second)
		{
			printf("[%p]", it1.first);
			printIR(it);
			cnt++;
		}
		printf("--------------------------------\n");
	}
	printf("Total IR %d\n", cnt);

	for (auto it1 : IRList)
	{
		printf("--------------------------------\n");
		for (auto it : it1.second)
		{
			printf("[%p]", it1.first);
			CheckTaintIR(it);
			printf("\n");
			cnt++;
		}
		printf("--------------------------------\n");
	}

#pragma region Dead Store Elimination
	for (int i = 0; i < cnt; i++)
	{
		map<DWORD, vector<IR*>>::iterator it1 = IRList.begin();
		while (it1 != IRList.end())
		{
			if (it1->second.size() == 0)
			{
				printf("Zero IR\n");
				continue;
			}
			//printf("--------------------------------\n");
			vector<IR*>::iterator it = it1->second.begin();
			while (it != it1->second.end())
				//for (auto it : it1->second)
			{
				IR * tmpIRPtr = *it;
				BOOL isLastMemValue = false;
				//printf("[%p]", it1->first);
				//printIR(*it);
				//printf("[%p} IR :%d opcnt:%d\n", it1.first, it->opr, it->Operands.size());
				if (tmpIRPtr->opr == IR::OPR::OPR_STORE)
				{
					//printf("[Dead Store Elimination] dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->opr %d\n", dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->opr);
					if (dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->opr == IR::OPR::OPR_BVV)
					{
						if (dynamic_cast<ConstInt*>(dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->Operands[0]->valuePtr))
						{
							//printf("[Dead Store Elimination] %s Use Count:%d\n", dynamic_cast<IR*>(tmpIRPtr->Operands[1]->valuePtr)->Name.c_str(), dynamic_cast<IR*>(tmpIRPtr->Operands[1]->valuePtr)->UseList.size());
							Value *Op1IR = dynamic_cast<Value*>(tmpIRPtr->Operands[1]->valuePtr);
							if (Op1IR && tmpIRPtr->Operands[1]->valuePtr->UseList.size() == 1)
								//if (MemValue[dynamic_cast<ConstInt*>(dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->Operands[0]->valuePtr)->intVar].back()->UseList.size() == 0)
							{
								set<OPERAND*>::iterator iter = Op1IR->UseList.begin();
								{
									if (dynamic_cast<IR*>(dynamic_cast<OPERAND*>(*iter)->parent) == tmpIRPtr)
									{
										printf("[Dead Store Elimination] MemAddress %p\n",
											dynamic_cast<ConstInt*>(dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->Operands[0]->valuePtr)->intVar);

										if (tmpIRPtr->Operands[0]->valuePtr->UseList.find(tmpIRPtr->Operands[0]) != tmpIRPtr->Operands[0]->valuePtr->UseList.end())
										{
											//printf("Before Remove this IR's Operand 1 %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
											tmpIRPtr->Operands[0]->valuePtr->UseList.erase(tmpIRPtr->Operands[0]);
											//printf("After Remove this IR's Operand 1 %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
										}

										if (tmpIRPtr->Operands[1]->valuePtr->UseList.find(tmpIRPtr->Operands[1]) != tmpIRPtr->Operands[1]->valuePtr->UseList.end())
										{
											//printf("Before Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
											tmpIRPtr->Operands[1]->valuePtr->UseList.erase(tmpIRPtr->Operands[1]);
											//printf("After Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
										}
										it = it1->second.erase(it);
										//it++;
										system("pause");
									}
									else
										++it;
								}


							}
							else
								++it;
						}
						else
							++it;
					}
					else
						++it;
				}

				else if ((tmpIRPtr->UseList.size() == 0) && (tmpIRPtr->isHiddenRHS == false))
				{

					for (auto memPoolItr : MemValue)
					{
						if (tmpIRPtr == memPoolItr.second.back())
						{
							//printf("Last MemValue\n");
							isLastMemValue = true;
							break;
						}
					}

					if (isLastMemValue == true)
					{
						it++;
						continue;
					}

					if (
						tmpIRPtr != RegValue[REG_EAX_16H].back() &&
						tmpIRPtr != RegValue[REG_EAX_8H].back() &&
						tmpIRPtr != RegValue[REG_EAX_8L].back() &&
						tmpIRPtr != RegValue[REG_EBX_16H].back() &&
						tmpIRPtr != RegValue[REG_EBX_8H].back() &&
						tmpIRPtr != RegValue[REG_EBX_8L].back() &&
						tmpIRPtr != RegValue[REG_ECX_16H].back() &&
						tmpIRPtr != RegValue[REG_ECX_8H].back() &&
						tmpIRPtr != RegValue[REG_ECX_8L].back() &&
						tmpIRPtr != RegValue[REG_EDX_16H].back() &&
						tmpIRPtr != RegValue[REG_EDX_8H].back() &&
						tmpIRPtr != RegValue[REG_EDX_8L].back() &&
						tmpIRPtr != RegValue[REG_EBP_16H].back() &&
						tmpIRPtr != RegValue[REG_EBP_8H].back() &&
						tmpIRPtr != RegValue[REG_EBP_8L].back() &&
						tmpIRPtr != RegValue[REG_EDI_16H].back() &&
						tmpIRPtr != RegValue[REG_EDI_8H].back() &&
						tmpIRPtr != RegValue[REG_EDI_8L].back())
					{

						if (tmpIRPtr->Operands.size() > 0)
							printf("Daed Store %p %s %d %s\n", it1->first, tmpIRPtr->Name.c_str(), tmpIRPtr->Operands.size(), tmpIRPtr->Operands[0]->valuePtr->Name.c_str());

						if (tmpIRPtr->Operands.size() == 1)
						{
							if (tmpIRPtr->Operands[0]->valuePtr->UseList.find(tmpIRPtr->Operands[0]) != tmpIRPtr->Operands[0]->valuePtr->UseList.end())
							{
								//printf("Before Remove this IR's Operand %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
								tmpIRPtr->Operands[0]->valuePtr->UseList.erase(tmpIRPtr->Operands[0]);
								//printf("After Remove this IR's Operand %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
							}
						}

						else if (tmpIRPtr->Operands.size() == 2)
						{
							//printf("[DSE ] Operands.size() == 2 %s\n", tmpIRPtr->Name.c_str());

							if (tmpIRPtr->Operands[0]->valuePtr->UseList.find(tmpIRPtr->Operands[0]) != tmpIRPtr->Operands[0]->valuePtr->UseList.end())
							{
								//printf("Before Remove this IR's Operand 1 %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
								tmpIRPtr->Operands[0]->valuePtr->UseList.erase(tmpIRPtr->Operands[0]);
								//printf("After Remove this IR's Operand 1 %s Opsize %d %d\n", tmpIRPtr->Operands[0]->valuePtr->Name.c_str(), dynamic_cast<IR*>(tmpIRPtr->Operands[0]->valuePtr)->Operands.size(),tmpIRPtr->Operands[0]->valuePtr->UseList.size());
							}

							if (tmpIRPtr->Operands[1]->valuePtr->UseList.find(tmpIRPtr->Operands[1]) != tmpIRPtr->Operands[1]->valuePtr->UseList.end())
							{
								//printf("Before Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
								tmpIRPtr->Operands[1]->valuePtr->UseList.erase(tmpIRPtr->Operands[1]);
								//printf("After Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
							}
							//system("pause");
						}

						else if (tmpIRPtr->Operands.size() == 3)
						{
							if (tmpIRPtr->Operands[0]->valuePtr->UseList.find(tmpIRPtr->Operands[0]) != tmpIRPtr->Operands[0]->valuePtr->UseList.end())
							{
								//("Before Remove this IR's Operand 1 %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
								tmpIRPtr->Operands[0]->valuePtr->UseList.erase(tmpIRPtr->Operands[0]);
								//printf("After Remove this IR's Operand 1 %d\n", tmpIRPtr->Operands[0]->valuePtr->UseList.size());
							}

							if (tmpIRPtr->Operands[1]->valuePtr->UseList.find(tmpIRPtr->Operands[1]) != tmpIRPtr->Operands[1]->valuePtr->UseList.end())
							{
								//printf("Before Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
								tmpIRPtr->Operands[1]->valuePtr->UseList.erase(tmpIRPtr->Operands[1]);
								//printf("After Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[1]->valuePtr->UseList.size());
							}

							if (tmpIRPtr->Operands[2]->valuePtr->UseList.find(tmpIRPtr->Operands[2]) != tmpIRPtr->Operands[2]->valuePtr->UseList.end())
							{
								//printf("Before Remove this IR's Operand 3 %d\n", tmpIRPtr->Operands[2]->valuePtr->UseList.size());
								tmpIRPtr->Operands[2]->valuePtr->UseList.erase(tmpIRPtr->Operands[2]);
								//printf("After Remove this IR's Operand 2 %d\n", tmpIRPtr->Operands[2]->valuePtr->UseList.size());
							}
						}

						it = it1->second.erase(it);
						//printf("it1->second.size() %d\n", it1->second.size());
					}
					else
					{
						it++;
					}

				}
				else
				{
					it++;
				}
			}
			//printf("--------------------------------\n");
			if (it1->second.size() == 0)
			{
				printf("Zero IR Remove Inst\n");
				system("pause");
				IRList.erase(it1++);
			}
			else ++it1;
		}
	}
#pragma endregion

	for (auto it1 : IRList)
	{
		printf("--------------------------------\n");
		for (auto it : it1.second)
		{
			printf("[%p]", it1.first);
			printIR(it);
		}
		printf("--------------------------------\n");
	}

	system("pause");

	DWORD op1ConstValue = 0;
	DWORD op2ConstValue = 0;
	DWORD op3ConstValue = 0;
	DWORD foldedConstValue = 0;

#pragma region Constant Propation
	{
		for (int i = 0; i < cnt; i++)
		{
			map<DWORD, vector<IR*>>::iterator it1 = IRList.begin();
			while (it1 != IRList.end())
			{
				if (it1->second.size() == 0)
				{
					printf("Zero IR\n");
					continue;
				}
				printf("--------------------------------\n");
				vector<IR*>::iterator it = it1->second.begin();
				while (it != it1->second.end())
					//for (auto it : it1->second)
				{
					printf("[%p]", it1->first);
					IR * tmpIRPtr = *it;
					printIR(*it);

					switch (tmpIRPtr->opr)
					{
					case IR::OPR_CONCAT:

						if (tmpIRPtr->Size == 32)
						{
							if (dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr) &&
								dynamic_cast<ConstInt*>(tmpIRPtr->Operands[1]->valuePtr) &&
								dynamic_cast<ConstInt*>(tmpIRPtr->Operands[2]->valuePtr))
							{
								op1ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr)->intVar << 16;
								op2ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[1]->valuePtr)->intVar << 8;
								op3ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[2]->valuePtr)->intVar & 0xff;
								foldedConstValue = op1ConstValue | op2ConstValue | op3ConstValue;

								Value* foldedVal = new ConstInt(IR::OPR_BVV, foldedConstValue);
								for (auto it3 : dynamic_cast<Value*>(*it)->UseList)
								{
									it3->valuePtr = foldedVal;
								}


								printf("[Constant Progatation d ][%s] %x\n", dynamic_cast<ConstInt*>(foldedVal)->Name.c_str(), dynamic_cast<ConstInt*>(foldedVal)->intVar);
							}
						}

						else if (tmpIRPtr->Size == 16)
						{
							if (dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr) &&
								dynamic_cast<ConstInt*>(tmpIRPtr->Operands[1]->valuePtr))
							{
								op2ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr)->intVar << 8;
								op3ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[1]->valuePtr)->intVar & 0xff;
								foldedConstValue = op2ConstValue | op3ConstValue;

								Value* foldedVal = new ConstInt(IR::OPR_GET16INT, foldedConstValue);
								for (auto it3 : dynamic_cast<Value*>(*it)->UseList)
								{
									it3->valuePtr = foldedVal;
								}
							}
						}

						else if (tmpIRPtr->Size == 8)
						{
							if (dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr))
							{
								op3ConstValue = dynamic_cast<ConstInt*>(tmpIRPtr->Operands[0]->valuePtr)->intVar & 0xff;
								foldedConstValue = op3ConstValue;

								Value* foldedVal = new ConstInt(IR::OPR_GET8INT, foldedConstValue);
								for (auto it3 : dynamic_cast<Value*>(*it)->UseList)
								{
									it3->valuePtr = foldedVal;
								}
							}
						}

						break;

						/*case IR::OPR_EXTRACT16H:
							op1ConstValue = tmpIRPtr->Operands[0]->valuePtr->ImmValue;
							op1ConstValue = op1ConstValue >> 16;
							break;

						case IR::OPR_EXTRACT8H:
							op1ConstValue = tmpIRPtr->Operands[0]->valuePtr->ImmValue;
							op1ConstValue = op1ConstValue >> 8;
							break;

						case IR::OPR_EXTRACT8L:
							op1ConstValue = tmpIRPtr->Operands[0]->valuePtr->ImmValue;
							op1ConstValue = 0xff;
							break;*/
					}
					it++;
				}
				it1++;
				printf("--------------------------------\n");
			}
		}
	}
#pragma endregion

	for (auto it1 : IRList)
	{
		printf("--------------------------------\n");
		for (auto it : it1.second)
		{
			printf("[%p]", it1.first);
			PostorderTraverse(it->ast);
			//PreorderTraverse(it->ast);
			printf("\n");
		}
		printf("--------------------------------\n");
	}

	for (auto it1 : IRList)
	{
		printf("--------------------------------\n");
		for (auto it : it1.second)
		{
			printf("[%p]", it1.first);
			//InorderTraverse(it->ast);
			printf("%s", EvaluateExpTree(it->ast).c_str());
			z3::expr z3Expression =  GetZ3ExprFromTree(it->ast);
			printf("\n");
			// 생성된 Z3 AST 출력
			std::cout << "생성된 Z3 AST: " << z3Expression.simplify() << std::endl;
			printf("\n");
			if (it1.first == 0x40100F)
			{
				z3::solver solver(*z3Context);
				z3Expression = z3Expression == 0x41;
				solver.add(z3Expression);
				z3::check_result result = solver.check();

				if (result == z3::sat) {
					// The constraints are satisfiable
					std::cout << "Satisfiable" << std::endl;

					// Get the model
					z3::model model = solver.get_model();

					// Print the model
					std::cout << "Model:" << std::endl;
					std::cout << model << std::endl;
				}
				else if (result == z3::unsat) {
					// The constraints are unsatisfiable
					std::cout << "Unsatisfiable" << std::endl;
				}
			}
		}
		printf("--------------------------------\n");
	}

	delete z3Context;
	printf("Finish\n");
	system("pause");
}

z3::expr GetZ3ExprFromTree(std::shared_ptr<BTreeNode> bt)
{
	if (bt->left == NULL)
	{
		if (bt->m_NodeType == NT_SYMVAR)
		{
			if (symbolExprMap.find(bt->nodeName) != symbolExprMap.end())
			{
				return *symbolExprMap[bt->nodeName];
			}
		}
		else
		{
			if (dynamic_cast<ConstInt*>(bt->valPtr))
			{
				if (dynamic_cast<ConstInt*>(bt->valPtr)->Size == 32)
				{
					z3::expr z3Expression = z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 32);
					std::cout << "test Z3 AST(32): " << z3Expression << std::endl;
					return z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 32);
				}

				else if (dynamic_cast<ConstInt*>(bt->valPtr)->Size == 16)
				{
					z3::expr z3Expression = z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 16);
					std::cout << "test Z3 AST(16): " << z3Expression << std::endl;
					return z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 16);
				}

				else if (dynamic_cast<ConstInt*>(bt->valPtr)->Size == 8)
				{
					z3::expr z3Expression = z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 8);
					std::cout << "test Z3 AST(8): " << z3Expression << std::endl;
					return z3Context->bv_val((unsigned int)dynamic_cast<ConstInt*>(bt->valPtr)->intVar, 8);
				}

				else
				{
					std::cout << "test Z3 AST(else)" << endl;
				}
			}
		}
	}

	else
	{
		z3::expr op1 = GetZ3ExprFromTree(bt->left);
		if (bt->right != NULL)
		{
			z3::expr op2 = GetZ3ExprFromTree(bt->right);
		}

		if (bt->nodeName.compare("OPR_BVV") == 0)
		{
			return op1;

		}

		else if (bt->nodeName.compare("OPR_ADD") == 0) {

			z3::expr op2 = GetZ3ExprFromTree(bt->right);
			return op1 + op2;
		}

		else if (bt->nodeName.compare("OPR_SUB") == 0) {

			z3::expr op2 = GetZ3ExprFromTree(bt->right);
			return op1 - op2;
		}

		else if (bt->nodeName.compare("OPR_AND") == 0) {

			z3::expr op2 = GetZ3ExprFromTree(bt->right);
			std::cout << "test Z3 OPR_AND AST: " << op1 << "Op2 : " << op2 << std::endl;
			return op1 & op2;
		}

		else if (bt->nodeName.compare("OPR_XOR") == 0) {

			z3::expr op2 = GetZ3ExprFromTree(bt->right);
			return op1 ^ op2;
		}

		else if (bt->nodeName.compare("Concat") == 0)
		{
			if (bt->third != NULL)
			{
				z3::expr op3 = GetZ3ExprFromTree(bt->third);
			}
			if (bt->fourth != NULL)
				z3::expr op4 = GetZ3ExprFromTree(bt->fourth);
			if (bt->fourth != nullptr)
			{
				z3::expr_vector exprVec(*z3Context);
				z3::expr op2 = GetZ3ExprFromTree(bt->right);
				z3::expr op3 = GetZ3ExprFromTree(bt->third);
				z3::expr op4 = GetZ3ExprFromTree(bt->fourth);

				exprVec.push_back(op1);
				exprVec.push_back(op2);
				exprVec.push_back(op3);
				exprVec.push_back(op4);


				return z3::concat(exprVec);
			}

			else if (bt->third != nullptr && bt->fourth == nullptr)
			{
				z3::expr_vector exprVec(*z3Context);
				z3::expr op2 = GetZ3ExprFromTree(bt->right);
				z3::expr op3 = GetZ3ExprFromTree(bt->third);

				exprVec.push_back(op1);
				exprVec.push_back(op2);
				exprVec.push_back(op3);


				return z3::concat(exprVec);
			}

			else
			{
				printf("Concat 2 Operands\n");
				z3::expr op2 = GetZ3ExprFromTree(bt->right);
				return z3::concat(op1, op2);
			}
		}

		else if (bt->nodeName.compare("EXTRACT16H") == 0)
		{
			return op1.extract(31, 16);
		}

		else if (bt->nodeName.compare("EXTRACT8H") == 0)
		{
			return op1.extract(15, 8);
		}

		else if (bt->nodeName.compare("EXTRACT8L") == 0)
		{
			return op1.extract(7, 0);
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

			}
		}
	}
}