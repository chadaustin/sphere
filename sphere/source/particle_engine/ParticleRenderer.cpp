#include "ParticleRenderer.hpp"


////////////////////////////////////////////////////////////////////////////////
ParticleRenderer::ParticleRenderer(const ParticleRenderer& renderer)
                 : m_BlendMode(renderer.m_BlendMode)
                 , m_Texture(NULL)
                 , m_OffsetX(renderer.m_OffsetX)
                 , m_OffsetY(renderer.m_OffsetY)
{
    SetTexture(renderer.m_Texture);
}

////////////////////////////////////////////////////////////////////////////////
void
ParticleRenderer::operator()(const Particle& p) const
{
    if (m_Texture)
        BlitImage(m_Texture, (int)(p.Pos.X) + m_OffsetX, (int)(p.Pos.Y) + m_OffsetY, m_BlendMode);

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleRenderer::SetTexture(IMAGE image)
{
    if (!image)
        return;

    IMAGE clone = CloneImage(image);

    if (!clone)
        return;

    if (m_Texture)
        DestroyImage(m_Texture);

    m_Texture = clone;

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleRenderer::SetBlendMode(CImage32::BlendMode blendmode)
{
    switch (blendmode)
    {
        case CImage32::BLEND:    m_BlendMode = CImage32::BLEND;    break;
        case CImage32::ADD:      m_BlendMode = CImage32::ADD;      break;
        case CImage32::SUBTRACT: m_BlendMode = CImage32::SUBTRACT; break;
        case CImage32::MULTIPLY: m_BlendMode = CImage32::MULTIPLY; break;
    }

}

////////////////////////////////////////////////////////////////////////////////
void
ParticleRenderer::SetBlendMode(int blendmode)
{
    switch (blendmode)
    {
        case CImage32::BLEND:    m_BlendMode = CImage32::BLEND;    break;
        case CImage32::ADD:      m_BlendMode = CImage32::ADD;      break;
        case CImage32::SUBTRACT: m_BlendMode = CImage32::SUBTRACT; break;
        case CImage32::MULTIPLY: m_BlendMode = CImage32::MULTIPLY; break;
    }

}




