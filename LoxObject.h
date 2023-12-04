#ifndef LOX_OBJECT
#define LOX_OBJECT
#include <string>
#include <vector>
#define AS_FUNCTION(obj) ((LoxFunction*)(obj.value.data))
#define AS_SB(obj) ((StateBuffer*)(obj.value.data))
enum class LoxType
{
	NUMBER, // lightweight wrapper around double to keep stack management easy
	BOOL,
	NIL,
	STATE_BUFFER,
	STRING,
	FUNCTION
};

struct Value
{
	union {
		bool boolean;
		double number;
		void* data;
	};
};
struct LoxObject
{
	LoxType type;
	Value value;
};

struct LoxFunction
{
	unsigned int arity;
	
	char* instruction; 
	int instruction_offset;
	unsigned int size;

	std::vector<LoxObject> upvalueTable;
};

struct StateBuffer
{
	char* instruction;
	int stack_base;
	int stack_size; 
	LoxFunction* caller;
};
typedef LoxObject(*op_type)(const LoxObject&, const LoxObject&);

void printLoxObject(const LoxObject& obj);
// aritmetic ops
LoxObject add_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject subtract_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject multiply_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject divide_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
//logical_ops;
LoxObject equalValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject notEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject lessEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject lessValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject greaterEqualValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject greaterValue(const LoxObject& leftOperand, const LoxObject& rightOperand);
//string ops
LoxObject concat_strings(const LoxObject& leftOperand, const LoxObject& rightOperand);
//function
LoxObject newLoxFunction();
LoxObject newStateBuffer();
void FreeLoxObject(const LoxObject& obj);
#endif // !

