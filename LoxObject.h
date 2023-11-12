#ifndef LOX_OBJECT
#define LOX_OBJECT

enum class LoxType
{
	VALUE,
	STRING
};

struct LoxObject
{
	LoxType type;
	void* data;
};


#endif // !

