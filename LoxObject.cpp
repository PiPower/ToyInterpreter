#include "LoxObject.h"
#include <iostream>
using namespace std;

void printLoxObject(const LoxObject& obj)
{
	switch (obj.type)
	{
	case LoxType::VALUE:
		cout << obj.value.number;
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
	c.value.number = leftOperand.value.number + rightOperand.value.number;
	return c;
}

LoxObject subtract_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	c.value.number = leftOperand.value.number - rightOperand.value.number;
	return c;
}

LoxObject multiply_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	c.value.number = leftOperand.value.number * rightOperand.value.number;
	return c;
}

LoxObject divide_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::VALUE;
	c.value.data = new double;
	c.value.number = leftOperand.value.number / rightOperand.value.number;
	return c;
}

LoxObject concat_strings(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	int left_size = strlen((char*)leftOperand.value.data);
	int right_size = strlen((char*)rightOperand.value.data);

	LoxObject c;
	c.type = LoxType::STRING;
	c.value.data = new char[left_size+ right_size + 1];
	memcpy(c.value.data, leftOperand.value.data, left_size);
	memcpy((char*)c.value.data + left_size, rightOperand.value.data, right_size);
	((char*)c.value.data)[left_size + right_size] = '\0';
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
