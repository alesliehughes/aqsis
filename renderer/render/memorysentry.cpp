// Aqsis
// Copyright © 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/**
 * \file memorysentry.cpp
 *
 * \brief Implementation of memory sentry class.
 *
 * \author Chris Foster  chris42f _at_ gmail.com
 */

#include "memorysentry.h"

#include "logging.h"

namespace Aqsis
{

//------------------------------------------------------------------------------
// Implementation for CqMemorySentry
//------------------------------------------------------------------------------
CqMemorySentry::CqMemorySentry(const TqMemorySize maxMemory)
	: m_maxMemory(maxMemory)
{
}

//------------------------------------------------------------------------------
void CqMemorySentry::registerAsManaged(const boost::shared_ptr<CqMemoryMonitored>& managedObject)
{
	m_managedList.push_back(boost::weak_ptr<CqMemoryMonitored>(managedObject));
}

//------------------------------------------------------------------------------
void CqMemorySentry::incrementTotalMemory(const TqMemorySize numBytes)
{
	m_totalMemory += numBytes;
	if(m_totalMemory > m_maxMemory)
	{
		/// \todo: Walk through the object list zapping some memory.
		Aqsis::log() << warning << "Exceeded global memory for textures.\n";
	}
}

//------------------------------------------------------------------------------
CqMemorySentry::~CqMemorySentry()
{ }

//------------------------------------------------------------------------------
// Implementation for CqMemoryMonitored
//------------------------------------------------------------------------------
CqMemoryMonitored::CqMemoryMonitored(CqMemorySentry& memorySentry)
	: m_memorySentry(memorySentry)
{
}

//------------------------------------------------------------------------------
CqMemorySentry& CqMemoryMonitored::getMemorySentry() const
{
	return m_memorySentry;
}

//------------------------------------------------------------------------------
// Virtual destructor empty implementation.
CqMemoryMonitored::~CqMemoryMonitored()
{ }

//------------------------------------------------------------------------------

} // namespace Aqsis
