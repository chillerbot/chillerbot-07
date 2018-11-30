/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <engine/demo.h>

#include <generated/client_data.h>
#include "particles.h"

CParticles::CParticles()
{
	OnReset();
	m_RenderTrail.m_pParts = this;
	m_RenderExplosions.m_pParts = this;
	m_RenderGeneral.m_pParts = this;
}


void CParticles::OnReset()
{

}

void CParticles::Add(int Group, CParticle *pPart)
{

}

void CParticles::Update(float TimePassed)
{

}

void CParticles::OnRender()
{

}

void CParticles::RenderGroup(int Group)
{

}
