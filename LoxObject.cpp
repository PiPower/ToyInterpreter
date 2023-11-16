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
