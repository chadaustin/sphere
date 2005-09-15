#include "parameters.hpp"

int argInt(JSContext* cx, jsval arg)
{
  int32 i;

  if (JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid integer (parameter is an object)");
    return false;
  }

  if (JS_ValueToInt32(cx, arg, &i) == JS_FALSE) {
    JS_ReportError(cx, "Invalid integer.");
    return 0; // invalid integer
  }

  return i;
}

///////////////////////////////////////////////////////////

const char* jsval_to_str(JSContext* cx, jsval arg)
{
  JSString* str = JS_ValueToString(cx, arg);
  if (str) {
    const char* s = JS_GetStringBytes(str);
    return (s ? s : "");
  } else {
    return "";
  }
}

///////////////////////////////////////////////////////////

const char* argStr(JSContext* cx, jsval arg)
{
  JSString* str = JS_ValueToString(cx, arg);
  if (str) {
    const char* s = JS_GetStringBytes(str);
    return (s ? s : "");
  } else {
    return "";
  }
}

///////////////////////////////////////////////////////////

bool argBool(JSContext* cx, jsval arg)
{
  JSBool b;

  if (JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid boolean (parameter is an object)");
    return false;
  }

  if (JS_ValueToBoolean(cx, arg, &b) == JS_FALSE) {
    return false;
  }

  return (b == JS_TRUE);
}

///////////////////////////////////////////////////////////

double argDouble(JSContext* cx, jsval arg)
{
  jsdouble d;

  if (JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid double (parameter is an object)");
    return 0;
  }

  if (!JSVAL_IS_INT(arg) && !JSVAL_IS_DOUBLE(arg)) {
    JS_ReportError(cx, "Invalid double.");
    return 0;
  }

  if (JS_ValueToNumber(cx, arg, &d) == JS_FALSE) {
    JS_ReportError(cx, "Invalid double.");
    return 0;
  }

  return d;
}

///////////////////////////////////////////////////////////

JSObject* argObject(JSContext* cx, jsval arg)
{
  if (!JSVAL_IS_OBJECT(arg)) {
    JS_ReportError(cx, "Invalid object (parameter is not an object)");
    return NULL;
  }

  JSObject* object;
  if (JS_ValueToObject(cx, arg, &object) == JS_FALSE) {
    JS_ReportError(cx, "Invalid object.");
    return NULL;
  }

  return object;
}

///////////////////////////////////////////////////////////

JSObject* argArray(JSContext* cx, jsval arg)
{
  JSObject* array = argObject(cx, arg);
  if (array == NULL)
    return NULL;

  if (!JS_IsArrayObject(cx, array)) {
    JS_ReportError(cx, "Invalid array (parameter is not an array)");
    return NULL;
  }

  return array;
}
