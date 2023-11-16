#ifndef LOX_OBJECT
#define LOX_OBJECT

enum class LoxType
{
	VALUE, // lightweight wrapper around double to keep stack management easy
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

void printLoxObject(const LoxObject& obj);
#endif // !

