#include "Value.h"

void calcBit(IR* concatIR)
{
	IR *extractIR;
	for (int i = 0; i < concatIR->Operands.size(); i++)
	{
		if ((extractIR = dynamic_cast<IR*>((concatIR->Operands[i]->valuePtr))) != nullptr)
		{
			//extractIR
		}
	}
}

void ConcatOptimize(IR* concatIR)
{
	bool isValueSame = true;
	// Concat ���۷����� Value* �� ��� ��ġ�ϴ��� Ȯ��
	for (int i = 0; i < concatIR->Operands.size(); i++)
	{
		if (concatIR->Operands[0]->valuePtr != concatIR->Operands[i]->valuePtr)
		{
			isValueSame = false;
			break;
		}
	}

	if (isValueSame == true)
	{
		
	}
}