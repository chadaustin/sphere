#include "ParticleSystemParent.hpp"



////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Update()
{
    if (IsExtinct() || IsHalted())
        return;

    if (IsDead() && m_Descendants.size() == 0)
    {
        m_Extinct = true;
        return;
    }

    // callback
    if (m_ScriptInterface.HasOnUpdate())
        m_ScriptInterface.OnUpdate();

    // update descendants
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end();)
    {
        Descendant d = *iter;

        // update adopted descendant's body
        if (d.Type == ADOPTED)
            m_Updater(d.System->GetBody());

        // handle death
        if (!d.System->IsDead() && d.System->GetBody().Life <= 0)
            d.System->Kill(this);

        d.System->Update();

        // handle extinction
        if (d.System->IsExtinct())
        {
            if (d.Type == ADOPTED && !IsCursed() && !IsDead())
            {
                m_Initializer(m_Body, d.System->GetBody());
                d.System->Revive(this);
            }
            else
            {
                d.System->Release();
                iter = m_Descendants.erase(iter);
            }
        }

        if (iter != m_Descendants.end())
             ++iter;

    } // end update descendants
}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Render()
{
    if (IsExtinct() || IsHidden())
        return;

    // callback
    if (m_ScriptInterface.HasOnRender())
        m_ScriptInterface.OnRender();

    // render descendants
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end(); ++iter)
        (*iter).System->Render();

    // render itself, if alive
    if (!IsDead())
        m_Renderer(m_Body);

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Adopt(ParticleSystemBase* system)
{
    if (!system || !system->Borrow())
        return;

    m_Initializer(m_Body, system->GetBody());
    system->Revive(this);

    m_Descendants.push_back(Descendant(system, ADOPTED));

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Host(ParticleSystemBase* system)
{
    if (!system || !system->Borrow())
        return;

    m_Descendants.push_back(Descendant(system, HOSTED));

}

////////////////////////////////////////////////////////////////////////////////
ParticleSystemBase*
ParticleSystemParent::Find(dword id)
{
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end(); ++iter)
        if ((*iter).System->GetID() == id)
            return (*iter).System;

    return NULL;

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Remove(dword id)
{
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end(); ++iter)
    {
        if ((*iter).System->GetID() == id)
        {
            (*iter).System->Release();
            iter = m_Descendants.erase(iter);
        }
    }

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::RemoveGroup(int group)
{
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end(); ++iter)
    {
        if ((*iter).System->GetGroup() == group)
        {
            (*iter).System->Release();
            iter = m_Descendants.erase(iter);
        }
    }

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleSystemParent::Clear()
{
    std::list<Descendant>::iterator iter;

    for (iter = m_Descendants.begin(); iter != m_Descendants.end(); ++iter)
        (*iter).System->Release();

    m_Descendants.clear();

}







