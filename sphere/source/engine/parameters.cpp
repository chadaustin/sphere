#include "parameters.hpp"

/*
 * The last GC thing of each type (object, string, double, external string
 * types) created on a given context is kept alive until another thing of the
 * same type is created, using a newborn root in the context.  These newborn
 * roots help native code protect newly-created GC-things from GC invocations
 * activated before those things can be rooted using local or global roots.
 * This is why we do not need to root some of the values here.
 */

///////////////////////////////////////////////////////////
/*
 * real numbers will be converted (rounded) automatically to the nearest integer, and so are strings
 * and objects. Objects need the method .valueOf() to be converted to number.
 */
int argInt(JSContext* cx, jsval arg)
{
    int32 i;
    if (JS_ValueToECMAInt32(cx, arg, &i)) //used to be JS_ValueToInt32, which rounded differently.
		return i;

    JS_ReportError(cx, "Invalid integer.");
    return 0; // invalid integer
}

///////////////////////////////////////////////////////////
/*
 * This will fail unless its an integer, 1.2 is not accepted as valid input
 */
int argStrictInt(JSContext* cx, jsval arg)
{
    int32 i;
    if (!JSVAL_IS_INT(arg))
    {
        JS_ReportError(cx, "Invalid integer");
        return false;
    }
    if (JS_ValueToECMAInt32(cx, arg, &i))
		return i;

    JS_ReportError(cx, "Invalid integer.");
    return 0; // invalid integer
}
///////////////////////////////////////////////////////////
const char* argStr(JSContext* cx, jsval arg)
{
    JSString* str = JS_ValueToString(cx, arg);

    if (str)
    {
        JS_AddRoot(cx, &str);
        const char* s = JS_GetStringBytes(str);
        JS_RemoveRoot(cx, &str);
        return (s ? s : "");
    }
    // else
    return "";
}
///////////////////////////////////////////////////////////
/*
 * accept numbers, booleans, strings but not objects to convert them. objects always evaluate to true, btw.
 */
bool argBool(JSContext* cx, jsval arg)
{
    JSBool b;
    if (JSVAL_IS_OBJECT(arg))
    {
        JS_ReportError(cx, "Invalid boolean (parameter is an object)");
        return false;
    }
    if (JS_ValueToBoolean(cx, arg, &b))
		return 	(b ? true : false);
	else
        return false;
}
///////////////////////////////////////////////////////////
double argDouble(JSContext* cx, jsval arg)
{
    jsdouble d;
    if (JSVAL_IS_NUMBER(arg) && JS_ValueToNumber(cx, arg, &d))
		return (double)d;

	// else
    JS_ReportError(cx, "Invalid double.");
    return 0;
}
///////////////////////////////////////////////////////////
JSObject* argObject(JSContext* cx, jsval arg)
{
    if (!JSVAL_IS_OBJECT(arg))
    {
        JS_ReportError(cx, "Invalid object (parameter is not an object)");
        return NULL;
    }
    JSObject* jobject;
    if (JS_ValueToObject(cx, arg, &jobject))
		return jobject; // non rooted, you need to root it asap

	// else
        JS_ReportError(cx, "Invalid object.");
        return NULL;
}
///////////////////////////////////////////////////////////
JSObject* argArray(JSContext* cx, jsval arg)
{
    JSObject* jarray = argObject(cx, arg);
    if (jarray && JS_IsArrayObject(cx, jarray))
        return jarray; // non rooted, you need to root it asap

    // else
    JS_ReportError(cx, "Invalid array (parameter is not an array)");
    return NULL;
}
///////////////////////////////////////////////////////////
JSObject* argFunctionObject(JSContext* cx, jsval arg)
{
    if (!JSVAL_IS_OBJECTNOTNULL(arg))
    {
        JS_ReportError(cx, "Invalid function (parameter is not an object - or null)");
        return NULL;
    }

    JSObject* jobject = JSVAL_TO_OBJECT(arg);

    if (jobject && JS_ObjectIsFunction(cx, jobject))
		return jobject; // non rooted, you need to root it asap

	// else
    JS_ReportError(cx, "Invalid function (parameter is not a function)");
    return NULL;
}
