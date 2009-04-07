#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <jsapi.h>

// Addition to jsapi.h
#define JSVAL_IS_OBJECTNOTNULL(v)      ( (JSVAL_TAG(v) == JSVAL_OBJECT) && ((v) != JSVAL_NULL) && ((v) != JSVAL_VOID))


int argInt(JSContext* cx, jsval arg);
int argStrictInt(JSContext* cx, jsval arg);
const char* argStr(JSContext* cx, jsval arg);
bool argBool(JSContext* cx, jsval arg);
double argDouble(JSContext* cx, jsval arg);
JSObject* argObject(JSContext* cx, jsval arg);
JSObject* argArray(JSContext* cx, jsval arg);
JSObject* argFunctionObject(JSContext* cx, jsval arg);

#endif
