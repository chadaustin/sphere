/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef jspubtd_h___
#define jspubtd_h___
/*
 * JS public API typedefs.
 */
#include "jstypes.h"
#include "jscompat.h"

JS_BEGIN_EXTERN_C

/* Scalar typedefs. */
typedef uint16    jschar;
typedef int32     jsint;
typedef uint32    jsuint;
typedef float64   jsdouble;
typedef jsword    jsval;
typedef jsword    jsid;
typedef jsword    jsrefcount;

typedef enum JSVersion {
    JSVERSION_1_0     = 100,
    JSVERSION_1_1     = 110,
    JSVERSION_1_2     = 120,
    JSVERSION_1_3     = 130,
    JSVERSION_1_4     = 140,
    JSVERSION_DEFAULT = 0,
    JSVERSION_UNKNOWN = -1
} JSVersion;

#define JSVERSION_IS_ECMA(version) \
    ((version) == JSVERSION_DEFAULT || (version) >= JSVERSION_1_3)

/* Result of typeof operator enumeration. */
typedef enum JSType {
    JSTYPE_VOID,                /* undefined */
    JSTYPE_OBJECT,              /* object */
    JSTYPE_FUNCTION,            /* function */
    JSTYPE_STRING,              /* string */
    JSTYPE_NUMBER,              /* number */
    JSTYPE_BOOLEAN,             /* boolean */
    JSTYPE_LIMIT
} JSType;

/* JSObjectOps.checkAccess mode enumeration. */
typedef enum JSAccessMode {
    JSACC_PROTO,
    JSACC_PARENT,
    JSACC_IMPORT,
    JSACC_WATCH,
    JSACC_LIMIT
} JSAccessMode;

/*
 * This enum type is used to control the behavior of a JSObject property
 * iterator function that has type JSNewEnumerate.
 */
typedef enum JSIterateOp {
    JSENUMERATE_INIT,       /* Create new iterator state */
    JSENUMERATE_NEXT,       /* Iterate once */
    JSENUMERATE_DESTROY     /* Destroy iterator state */
} JSIterateOp;

/* Struct typedefs. */
typedef struct JSClass           JSClass;
typedef struct JSConstDoubleSpec JSConstDoubleSpec;
typedef struct JSContext         JSContext;
typedef struct JSErrorReport     JSErrorReport;
typedef struct JSFunction        JSFunction;
typedef struct JSFunctionSpec    JSFunctionSpec;
typedef struct JSIdArray         JSIdArray;
typedef struct JSProperty        JSProperty;
typedef struct JSPropertySpec    JSPropertySpec;
typedef struct JSObject          JSObject;
typedef struct JSObjectMap       JSObjectMap;
typedef struct JSObjectOps       JSObjectOps;
typedef struct JSRuntime         JSRuntime;
typedef struct JSRuntime         JSTaskState;	/* XXX deprecated name */
typedef struct JSScript          JSScript;
typedef struct JSString          JSString;
typedef struct JSXDRState	 JSXDRState;
typedef struct JSExceptionState  JSExceptionState;

#ifndef CRT_CALL
#ifdef XP_OS2
#define CRT_CALL _Optlink
#else
#define CRT_CALL
#endif
#endif

/* JSClass (and JSObjectOps where appropriate) function pointer typedefs. */

typedef JSBool
(* CRT_CALL JSPropertyOp)(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

/*
 *  This function type is used for callbacks that enumerate the properties of a
 *  JSObject.  The behavior depends on the value of enum_op:
 *
 *    JSENUMERATE_INIT - A new, opaque iterator state should be allocated and
 *           stored in *statep.  (You can use PRIVATE_TO_JSVAL() to store
 *           a pointer in *statep).  The number of properties that will be
 *           enumerated should be returned as an integer jsval in *idp, if idp
 *           is non-NULL.
 *    JSENUMERATE_NEXT - A previously allocated opaque iterator state is passed
 *           in via statep.  Return the next jsid in the iteration using *idp.
 *           The opaque iterator state pointed at by statep is destroyed and
 *           *statep is set to JSVAL_NULL if there are no properties left to
 *           enumerate.
 *    JSENUMERATE_DESTROY - Destroy the opaque iterator previous allocated by
 *           a call to this function with enum_op set to JSENUMERATE_INIT.
 *
 *   The return value is always used to indicate success, with a value of JS_FALSE
 *   for failure.
 */
typedef JSBool
(* CRT_CALL JSNewEnumerateOp)(JSContext *cx, JSObject *obj,
			      JSIterateOp enum_op,
			      jsval *statep, jsid *idp);

typedef JSBool
(* CRT_CALL JSEnumerateOp)(JSContext *cx, JSObject *obj);

typedef JSBool
(* CRT_CALL JSResolveOp)(JSContext *cx, JSObject *obj, jsval id);

typedef JSBool
(* CRT_CALL JSNewResolveOp)(JSContext *cx, JSObject *obj, jsval id, uintN flags,
			    JSObject **objp);

typedef JSBool
(* CRT_CALL JSConvertOp)(JSContext *cx, JSObject *obj, JSType type, jsval *vp);

typedef void
(* CRT_CALL JSFinalizeOp)(JSContext *cx, JSObject *obj);

typedef JSObjectOps *
(* CRT_CALL JSGetObjectOps)(JSContext *cx, JSClass *clasp);

typedef JSBool
(* CRT_CALL JSCheckAccessOp)(JSContext *cx, JSObject *obj, jsval id,
			     JSAccessMode mode, jsval *vp);

typedef JSBool
(* CRT_CALL JSXDRObjectOp)(JSXDRState *xdr, JSObject **objp);

typedef JSBool
(* CRT_CALL JSHasInstanceOp)(JSContext *cx, JSObject *obj, jsval v,
			     JSBool *bp);

/* JSObjectOps function pointer typedefs. */

typedef JSObjectMap *
(* CRT_CALL JSNewObjectMapOp)(JSContext *cx, jsrefcount nrefs,
			      JSObjectOps *ops, JSClass *clasp,
			      JSObject *obj);

typedef void
(* CRT_CALL JSObjectMapOp)(JSContext *cx, JSObjectMap *map);

typedef JSBool
(* CRT_CALL JSLookupPropOp)(JSContext *cx, JSObject *obj, jsid id,
			    JSObject **objp, JSProperty **propp
#if defined JS_THREADSAFE && defined DEBUG
			    , const char *file, uintN line
#endif
			    );

typedef JSBool
(* CRT_CALL JSDefinePropOp)(JSContext *cx, JSObject *obj, jsid id, jsval value,
			    JSPropertyOp getter, JSPropertyOp setter,
			    uintN attrs, JSProperty **propp);

typedef JSBool
(* CRT_CALL JSPropertyIdOp)(JSContext *cx, JSObject *obj, jsid id, jsval *vp);

typedef JSBool
(* CRT_CALL JSAttributesOp)(JSContext *cx, JSObject *obj, jsid id,
			    JSProperty *prop, uintN *attrsp);

typedef JSBool
(* CRT_CALL JSCheckAccessIdOp)(JSContext *cx, JSObject *obj, jsid id,
			       JSAccessMode mode, jsval *vp, uintN *attrsp);

typedef JSObject *
(* CRT_CALL JSObjectOp)(JSContext *cx, JSObject *obj);

typedef void
(* CRT_CALL JSPropertyRefOp)(JSContext *cx, JSObject *obj, JSProperty *prop);

/* Typedef for native functions called by the JS VM. */

typedef JSBool
(* CRT_CALL JSNative)(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
		      jsval *rval);

/* Callbacks and their arguments. */

typedef enum JSGCStatus {
    JSGC_BEGIN,
    JSGC_END
} JSGCStatus;

typedef JSBool
(* CRT_CALL JSGCCallback)(JSContext *cx, JSGCStatus status);

typedef JSBool
(* CRT_CALL JSBranchCallback)(JSContext *cx, JSScript *script);

typedef void
(* CRT_CALL JSErrorReporter)(JSContext *cx, const char *message,
			     JSErrorReport *report);

typedef struct JSErrorFormatString {
    const char *format;
    const uintN argCount;
} JSErrorFormatString;

typedef const JSErrorFormatString *
(* CRT_CALL JSErrorCallback)(void *userRef, const char *locale,
			     const uintN errorNumber);

JS_END_EXTERN_C

#endif /* jspubtd_h___ */
