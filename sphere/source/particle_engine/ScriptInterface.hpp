#ifndef SCRIPT_INTERFACE_HPP
#define SCRIPT_INTERFACE_HPP


#include <jsapi.h>


// forward declaration
class ParticleSystemBase;


class ScriptInterface
{
    protected:

        JSContext* m_Context; // javascript context
        JSObject*  m_Object;  // javascript object, which owns the particle system

        bool       m_Protected; // object protected from being garbage-collected

        // callback functions
        jsval    m_OnUpdateFunc;
        jsval    m_OnRenderFunc;
        jsval    m_OnBirthFunc;
        jsval    m_OnDeathFunc;

    public:

        ScriptInterface();
        ScriptInterface(const ScriptInterface& interface);

        ~ScriptInterface();

        void      SetContext(JSContext* cx);

        JSObject* GetObject() const;
        void      SetObject(JSObject* obj);

        bool IsProtected() const;
        bool StartProtection();
        void EndProtection();

        bool  HasOnUpdate() const;
        jsval GetOnUpdate() const;
        void  SetOnUpdate(jsval on_update);

        bool  HasOnRender() const;
        jsval GetOnRender() const;
        void  SetOnRender(jsval on_render);

        bool  HasOnBirth() const;
        jsval GetOnBirth() const;
        void  SetOnBirth(jsval on_birth);

        bool  HasOnDeath() const;
        jsval GetOnDeath() const;
        void  SetOnDeath(jsval on_death);

        void OnUpdate() const;
        void OnRender() const;
        void OnBirth(ParticleSystemBase* parent) const;
        void OnDeath(ParticleSystemBase* parent) const;

};

////////////////////////////////////////////////////////////////////////////////
inline
ScriptInterface::~ScriptInterface()
{
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetContext(JSContext* cx)
{
    m_Context = cx;
}

////////////////////////////////////////////////////////////////////////////////
inline JSObject*
ScriptInterface::GetObject() const
{
    return m_Object;
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetObject(JSObject* obj)
{
    m_Object = obj;
}

////////////////////////////////////////////////////////////////////////////////
inline bool
ScriptInterface::IsProtected() const
{
    return m_Protected;
}

////////////////////////////////////////////////////////////////////////////////
inline bool
ScriptInterface::HasOnUpdate() const
{
    return m_OnUpdateFunc != JSVAL_NULL;

}

////////////////////////////////////////////////////////////////////////////////
inline jsval
ScriptInterface::GetOnUpdate() const
{
    return m_OnUpdateFunc;
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetOnUpdate(jsval on_update)
{
    m_OnUpdateFunc = on_update;
}

////////////////////////////////////////////////////////////////////////////////
inline bool
ScriptInterface::HasOnRender() const
{
    return m_OnRenderFunc != JSVAL_NULL;

}

////////////////////////////////////////////////////////////////////////////////
inline jsval
ScriptInterface::GetOnRender() const
{
    return m_OnRenderFunc;
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetOnRender(jsval on_render)
{
    m_OnRenderFunc = on_render;
}

////////////////////////////////////////////////////////////////////////////////
inline bool
ScriptInterface::HasOnBirth() const
{
    return m_OnBirthFunc != JSVAL_NULL;

}

////////////////////////////////////////////////////////////////////////////////
inline jsval
ScriptInterface::GetOnBirth() const
{
    return m_OnBirthFunc;
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetOnBirth(jsval on_birth)
{
    m_OnBirthFunc = on_birth;
}

////////////////////////////////////////////////////////////////////////////////
inline bool
ScriptInterface::HasOnDeath() const
{
    return m_OnDeathFunc != JSVAL_NULL;

}

////////////////////////////////////////////////////////////////////////////////
inline jsval
ScriptInterface::GetOnDeath() const
{
    return m_OnDeathFunc;
}

////////////////////////////////////////////////////////////////////////////////
inline void
ScriptInterface::SetOnDeath(jsval on_death)
{
    m_OnDeathFunc = on_death;
}




#endif





