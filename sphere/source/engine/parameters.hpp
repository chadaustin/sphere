#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <string>
#include <smjs/jsapi.h>

int argInt(JSContext* cx, jsval arg);
const char* jsval_to_str(JSContext* cx, jsval arg);
const char* argStr(JSContext* cx, jsval arg);
bool argBool(JSContext* cx, jsval arg);
double argDouble(JSContext* cx, jsval arg);
JSObject* argObject(JSContext* cx, jsval arg);
JSObject* argArray(JSContext* cx, jsval arg);

#endif
