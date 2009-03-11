#include "ScriptInterface.hpp"
#include "ParticleSystemBase.hpp"



////////////////////////////////////////////////////////////////////////////////
ScriptInterface::ScriptInterface()
                : m_Context(NULL)
                , m_Object(NULL)
                , m_Protected(false)
                , m_OnUpdateFunc(JSVAL_NULL)
                , m_OnRenderFunc(JSVAL_NULL)
                , m_OnBirthFunc(JSVAL_NULL)
                , m_OnDeathFunc(JSVAL_NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
ScriptInterface::ScriptInterface(const ScriptInterface& interface)
                : m_Context(NULL)
                , m_Object(NULL)
                , m_Protected(false)
                , m_OnUpdateFunc(interface.m_OnUpdateFunc)
                , m_OnRenderFunc(interface.m_OnRenderFunc)
                , m_OnBirthFunc(interface.m_OnBirthFunc)
                , m_OnDeathFunc(interface.m_OnDeathFunc)
{
}

////////////////////////////////////////////////////////////////////////////////
ScriptInterface::~ScriptInterface()
{
    if (m_Context)
    {
        JS_RemoveRoot(m_Context, &m_OnUpdateFunc);
        JS_RemoveRoot(m_Context, &m_OnRenderFunc);
        JS_RemoveRoot(m_Context, &m_OnBirthFunc);
        JS_RemoveRoot(m_Context, &m_OnDeathFunc);
    }
}

////////////////////////////////////////////////////////////////////////////////
/*
 * - Routine used to handle all important initialization.
 * - If 'false' is returned, the interface must not be used.
 */
bool
ScriptInterface::Init(JSContext* context, JSObject* object)
{
    if (!context || !object)
        return false;

    m_Context = context;
    m_Object  = object;

    if (!JS_AddRoot(m_Context, &m_OnUpdateFunc))
    {
        m_Context = NULL;
        m_Object  = NULL;
        return false;
    }

    if (!JS_AddRoot(m_Context, &m_OnRenderFunc))
    {
        m_Context = NULL;
        m_Object  = NULL;
        JS_RemoveRoot(m_Context, &m_OnUpdateFunc);
        return false;
    }

    if (!JS_AddRoot(m_Context, &m_OnBirthFunc))
    {
        m_Context = NULL;
        m_Object  = NULL;
        JS_RemoveRoot(m_Context, &m_OnUpdateFunc);
        JS_RemoveRoot(m_Context, &m_OnRenderFunc);
        return false;
    }

    if (!JS_AddRoot(m_Context, &m_OnDeathFunc))
    {
        m_Context = NULL;
        m_Object  = NULL;
        JS_RemoveRoot(m_Context, &m_OnUpdateFunc);
        JS_RemoveRoot(m_Context, &m_OnRenderFunc);
        JS_RemoveRoot(m_Context, &m_OnBirthFunc);
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool
ScriptInterface::StartProtection()
{
    bool succeeded = JS_AddRoot(m_Context, &m_Object) == JS_TRUE;

    if (succeeded)
        m_Protected = true;

    return succeeded;

}

////////////////////////////////////////////////////////////////////////////////
void
ScriptInterface::EndProtection()
{
    JS_RemoveRoot(m_Context, &m_Object);

    m_Protected = false;

}

////////////////////////////////////////////////////////////////////////////////
void
ScriptInterface::OnUpdate() const
{
    jsval rval;
    JS_CallFunctionValue(m_Context, m_Object, m_OnUpdateFunc, 0, NULL, &rval);

}

////////////////////////////////////////////////////////////////////////////////
void
ScriptInterface::OnRender() const
{
    jsval rval;
    JS_CallFunctionValue(m_Context, m_Object, m_OnRenderFunc, 0, NULL, &rval);

}

////////////////////////////////////////////////////////////////////////////////
void
ScriptInterface::OnBirth(ParticleSystemBase* parent) const
{
    jsval rval;
    jsval arg = JSVAL_NULL;

    if (parent)
        arg = OBJECT_TO_JSVAL(parent->GetScriptInterface().GetObject());

    JS_CallFunctionValue(m_Context, m_Object, m_OnBirthFunc, 1, &arg, &rval);

}

////////////////////////////////////////////////////////////////////////////////
void
ScriptInterface::OnDeath(ParticleSystemBase* parent) const
{
    jsval rval;
    jsval arg = JSVAL_NULL;

    if (parent)
        arg = OBJECT_TO_JSVAL(parent->GetScriptInterface().GetObject());

    JS_CallFunctionValue(m_Context, m_Object, m_OnDeathFunc, 1, &arg, &rval);

}





