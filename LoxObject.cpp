#include "LoxObject.h"
#include <iostream>
using namespace std;

void printLoxObject(const LoxObject& obj)
{
	switch (obj.type)
	{
	case LoxType::NIL:
		cout << "NIL";
		break;
	case LoxType::BOOL:
	{
		string text = obj.value.boolean ? "True" : "False";
		cout << text;
		break;
	}
	case LoxType::NUMBER:
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
	c.type = LoxType::NUMBER;
	c.value.data = new double;
	c.value.number = leftOperand.value.number + rightOperand.value.number;
	return c;
}

LoxObject subtract_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::NUMBER;
	c.value.data = new double;
	c.value.number = leftOperand.value.number - rightOperand.value.number;
	return c;
}

LoxObject multiply_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::NUMBER;
	c.value.data = new double;
	c.value.number = leftOperand.value.number * rightOperand.value.number;
	return c;
}

LoxObject divide_number(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	LoxObject c;
	c.type = LoxType::NUMBER;
	c.value.data = new double;
	c.value.number = leftOperand.value.number / rightOperand.value.number;
	return c;
}


LoxObject notEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{

	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}
	LoxObject out;
	out.type = LoxType::BOOL;
	switch (leftOperand.type)
	{
	case LoxType::NIL:
		out.value.boolean = false;
		break;
	case LoxType::BOOL:
		out.value.boolean = !(leftOperand.value.boolean == rightOperand.value.boolean);
		break;
	case LoxType::NUMBER:
		out.value.boolean = !(leftOperand.value.number == rightOperand.value.number);
		break;
	case LoxType::STRING:
		int left_size = strlen((char*)leftOperand.value.data);
		int right_size = strlen((char*)rightOperand.value.data);
		if (left_size != right_size)
		{
			out.value.boolean = true;
			break;
		}
		out.value.boolean = !(memcmp(leftOperand.value.data, rightOperand.value.data, right_size) == 0);
		break;
	}
	return out;
}

LoxObject lessEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{

	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}

	if (leftOperand.type != LoxType::NUMBER)
	{
		cout << "ERROR: types must be numbers \n";
		exit(-1);
	}

	LoxObject out;
	out.type = LoxType::BOOL;
	out.value.boolean = leftOperand.value.number <= rightOperand.value.number;
	return out;
}

LoxObject lessValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}

	if (leftOperand.type != LoxType::NUMBER)
	{
		cout << "ERROR: types must be numbers \n";
		exit(-1);
	}

	LoxObject out;
	out.type = LoxType::BOOL;
	out.value.boolean = leftOperand.value.number < rightOperand.value.number;
	return out;
}

LoxObject greaterEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}

	if (leftOperand.type != LoxType::NUMBER)
	{
		cout << "ERROR: types must be numbers \n";
		exit(-1);
	}

	LoxObject out;
	out.type = LoxType::BOOL;
	out.value.boolean = leftOperand.value.number >= rightOperand.value.number;
	return out;
}

LoxObject greaterValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}

	if (leftOperand.type != LoxType::NUMBER)
	{
		cout << "ERROR: types must be numbers \n";
		exit(-1);
	}

	LoxObject out;
	out.type = LoxType::BOOL;
	out.value.boolean = leftOperand.value.number > rightOperand.value.number;
	return out;
}

LoxObject equalValue(const LoxObject& leftOperand, const LoxObject& rightOperand)
{
	if (leftOperand.type != rightOperand.type)
	{
		cout << "ERROR: Not matching types \n";
		exit(-1);
	}
	LoxObject out;
	out.type = LoxType::BOOL;
	switch (leftOperand.type)
	{
	case LoxType::NIL:
		out.value.boolean = true;
		break;
	case LoxType::BOOL:
		out.value.boolean = leftOperand.value.boolean == rightOperand.value.boolean ;
		break;
	case LoxType::NUMBER:
		out.value.boolean = leftOperand.value.number == rightOperand.value.number;
		break;
	case LoxType::STRING:
		int left_size = strlen((char*)leftOperand.value.data);
		int right_size = strlen((char*)rightOperand.value.data);
		if (left_size != right_size)
		{
			out.value.boolean = false;
			break;
		}
		out.value.boolean = memcmp(leftOperand.value.data, rightOperand.value.data, right_size) == 0;
		break;
	}
	return out;
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

LoxObject newLoxFunction()
{
	LoxObject out;
	out.type = LoxType::FUNCTION;
	out.value.data = new LoxFunction();
	((LoxFunction*)(out.value.data))->arity = 0;
	((LoxFunction*)(out.value.data))->size = 0;
	((LoxFunction*)(out.value.data))->instruction_offset = 0;
	((LoxFunction*)(out.value.data))->instruction = nullptr;
	return out;
}

LoxObject newStateBuffer()
{
	LoxObject out;
	out.type = LoxType::STATE_BUFFER;
	out.value.data = new StateBuffer();
	((StateBuffer*)(out.value.data))->instruction = nullptr;
	((StateBuffer*)(out.value.data))->stack_size = 0;
	return out;
}

void ColorObject(const LoxObject* obj)
{
	if (obj->trackState != TrackingState::WHITE) return;
	
	switch (obj->type)
	{
	case LoxType::NUMBER:
	case LoxType::BOOL:
	case LoxType::NIL:
	case LoxType::STRING:
		obj->trackState = TrackingState::BLACK;
		break;
	case LoxType::FUNCTION:
		obj->trackState = TrackingState::GRAY;
		AS_FUNCTION(obj)->trackState = TrackingState::GRAY;
		for (const LoxObject* subObj : AS_FUNCTION(obj)->upvalueTable)
		{
			ColorObject(subObj);
		}
		AS_FUNCTION(obj)->trackState = TrackingState::BLACK;
		obj->trackState = TrackingState::BLACK;
		break;
	case LoxType::STATE_BUFFER:
	{
		obj->trackState = TrackingState::GRAY;
		AS_SB(obj)->trackState = TrackingState::GRAY;
		LoxFunction* caller = AS_SB(obj)->caller;
		caller->trackState = TrackingState::GRAY;
		for (const LoxObject* subObj : caller->upvalueTable)
		{
			ColorObject(subObj);
		}
		caller->trackState = TrackingState::BLACK;
		AS_SB(obj)->trackState = TrackingState::BLACK;
		obj->trackState = TrackingState::BLACK;
		break;
	}
	default:
		cout << "Uknown object to color" << endl;
		exit(-1);
	}
}

void FreeLoxObject(const LoxObject* obj)
{
	switch (obj->type)
	{
	case LoxType::NUMBER:
	case LoxType::BOOL:
	case LoxType::NIL:
		delete obj;
		return;
	case LoxType::STRING:

		delete (char*)obj->value.data;
		obj->value.data = nullptr;
		delete obj;
		return;
	case LoxType::FUNCTION:
		if(AS_FUNCTION(obj)->trackState == TrackingState::WHITE) delete AS_FUNCTION(obj);
		delete obj;
		return;
	case LoxType::STATE_BUFFER:

		if (AS_SB(obj)->caller !=nullptr &&
			AS_SB(obj)->caller->trackState == TrackingState::WHITE)
		{
			delete AS_SB(obj)->caller;
		}
		if (AS_SB(obj)->trackState == TrackingState::WHITE) delete AS_SB(obj);
		delete obj;
		return;
	}

}
