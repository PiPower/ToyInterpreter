#include "LoxObject.h"
#include <iostream>
using namespace std;

void printLoxObject(const LoxObject& obj)
{
	switch (obj.type)
	{
	case LoxType::VALUE:
		cout << *(double*)obj.value.data;
		break;
	case LoxType::STRING:
		cout << (char*)obj.value.data;
	default:
		break;
	}
}

LoxObject add_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	AS_DOUBLE(c.value.data) = AS_DOUBLE(leftOperand.value.data) + AS_DOUBLE(rightOperand.value.data);
	return c;
}

LoxObject subtract_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	AS_DOUBLE(c.value.data) = AS_DOUBLE(leftOperand.value.data) - AS_DOUBLE(rightOperand.value.data);
	return c;
}

LoxObject multiply_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	AS_DOUBLE(c.value.data) = AS_DOUBLE(leftOperand.value.data) * AS_DOUBLE(rightOperand.value.data);
	return c;
}

LoxObject divide_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	AS_DOUBLE(c.value.data) = AS_DOUBLE(leftOperand.value.data) / AS_DOUBLE(rightOperand.value.data);
	return c;
}

void FreeLoxObject(const LoxObject& obj)
{
	switch (obj.type)
	{
	case LoxType::STRING :
		delete[] obj.value.data;
		break;
	default:
		break;
	}
}
