#ifndef LOX_OBJECT
#define LOX_OBJECT

#define AS_DOUBLE(ptr) *(double*)(ptr)

enum class LoxType
{
	NUMBER, // lightweight wrapper around double to keep stack management easy
	BOOL,
	NIL,
	STRING
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

typedef LoxObject(*op_type)(const LoxObject&, const LoxObject&);

void printLoxObject(const LoxObject& obj);
// aritmetic ops
LoxObject add_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject subtract_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject multiply_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject divide_number(const LoxObject& leftOperand, const LoxObject& rightOperand);
LoxObject equalValues(const LoxObject& leftOperand, const LoxObject& rightOperand);
//string ops
LoxObject concat_strings(const LoxObject& leftOperand, const LoxObject& rightOperand);

void FreeLoxObject(const LoxObject& obj);
#endif // !

