#ifndef PARTICLE_SYSTEM_PARENT_HPP
#define PARTICLE_SYSTEM_PARENT_HPP


#include <list>

#include "ParticleSystemBase.hpp"



class ParticleSystemParent : public ParticleSystemBase
{
    private:

        enum DescendantType
        {
            ADOPTED = 0,
            HOSTED
        };

        class Descendant
        {
            public:

                ParticleSystemBase* System;
                DescendantType      Type;

                explicit Descendant(ParticleSystemBase* s, DescendantType t) : System(s)
                                                                             , Type(t)
                {
                }

                Descendant(const Descendant& d) : System(d.System)
                                                , Type(d.Type)
                {
                }

                ~Descendant()
                {
                }

                Descendant& operator=(const Descendant& d)
                {
                    System = d.System;
                    Type   = d.Type;
                    return *this;
                }

        };

        std::list<Descendant> m_Descendants;

    public:

        ParticleSystemParent();

        virtual ~ParticleSystemParent();

        virtual void Update();
        virtual void Render();

        dword Size() const;

        void Adopt(ParticleSystemBase*);
        void Host(ParticleSystemBase*);

        ParticleSystemBase* Find(dword id);

        void  Remove(dword id);
        void  RemoveGroup(int group);
        void  Clear();

};

////////////////////////////////////////////////////////////////////////////////
inline
ParticleSystemParent::ParticleSystemParent()
{
}

////////////////////////////////////////////////////////////////////////////////
inline
ParticleSystemParent::~ParticleSystemParent()
{
    Clear();
}

////////////////////////////////////////////////////////////////////////////////
inline dword
ParticleSystemParent::Size() const
{
    return m_Descendants.size();
}





#endif








