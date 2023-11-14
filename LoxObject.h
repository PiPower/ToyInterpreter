#ifndef LOX_OBJECT
#define LOX_OBJECT

enum class LoxType
{
	VALUE, // lightweight wrapper around double to keep stack management easy
	STRING
};

struct LoxObject
{
	LoxType type;
	void* data;
};


#endif // !

