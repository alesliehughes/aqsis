// Aqsis
// Copyright 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
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


/** \file
		\brief Implements the RenderMan context classes.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

#include	"aqsis.h"

#include "graphicsstate.h"
#include "lights.h"
#include "csgtree.h"
#include "renderer.h"

START_NAMESPACE( Aqsis )


CqList<CqCSGTreeNode>	CqModeBlock::m_lCSGTrees;


//---------------------------------------------------------------------
/** Default constructor.
 */

CqModeBlock::CqModeBlock( CqModeBlock* pconParent ) :
		m_pattrCurrent( 0 ),
		m_ptransCurrent( 0 ),
		m_pconParent( pconParent )
{
	if( m_pconParent ) ADDREF( m_pconParent );
}

//---------------------------------------------------------------------
/** Default Destructor.
 */

CqModeBlock::~CqModeBlock()
{
	if( m_pconParent ) RELEASEREF( m_pconParent );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMainModeBlock::CqMainModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Copy the current graphics state.
	if ( pconParent != 0 )
		m_optCurrent = pconParent->optCurrent();

	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes();
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform();
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMainModeBlock::~CqMainModeBlock()
{
	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqFrameModeBlock::CqFrameModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent ), m_optCurrent(pconParent->optCurrent())
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes( *pconParent->m_pattrCurrent );
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqFrameModeBlock::~CqFrameModeBlock()
{
	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqWorldModeBlock::CqWorldModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes( *pconParent->m_pattrCurrent );
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqWorldModeBlock::~CqWorldModeBlock()
{
	// Delete any context lights
	std::vector<CqLightsource*>::iterator i;
	for ( i = m_apWorldLights.begin(); i != m_apWorldLights.end(); i++ )
		RELEASEREF( ( *i ) );

	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Hold a reference to the specified lightsource in this context.
 */

void CqWorldModeBlock::AddContextLightSource( CqLightsource* pLS )
{
	// Add the light to the context stack and reference it.
	m_apWorldLights.push_back( pLS );
	ADDREF( pLS );
}

//---------------------------------------------------------------------
/** Default constructor.
 */

CqAttributeModeBlock::CqAttributeModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes( *pconParent->m_pattrCurrent );
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqAttributeModeBlock::~CqAttributeModeBlock()
{
	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqTransformModeBlock::CqTransformModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if ( pconParent != 0 )
		m_pattrCurrent = pconParent->m_pattrCurrent;
	else
		m_pattrCurrent = new CqAttributes();

	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqTransformModeBlock::~CqTransformModeBlock()
{}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqSolidModeBlock::CqSolidModeBlock( CqString& type, CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes( *pconParent->m_pattrCurrent );
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );

	// Create a new CSG tree node of the appropriate type.
	m_pCSGNode = CqCSGTreeNode::CreateNode( type );
	ADDREF( m_pCSGNode );

	if ( pconParent && pconParent->isSolid() )
	{
		CqSolidModeBlock * pParentSolid = static_cast<CqSolidModeBlock*>( pconParent );
		// Check if we are linking under a Primitive node, if so warn, and link at the top.
		if ( pParentSolid->pCSGNode() ->NodeType() == CqCSGTreeNode::CSGNodeType_Primitive )
		{
			CqAttributeError error( RIE_BADSOLID, Severity_Normal, "Cannot add solid block under 'Primitive' solid block", m_pattrCurrent, TqTrue );
			m_lCSGTrees.LinkLast( m_pCSGNode );
		}
		else
			pParentSolid->pCSGNode() ->AddChild( m_pCSGNode );
	}
	else
		m_lCSGTrees.LinkLast( m_pCSGNode );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqSolidModeBlock::~CqSolidModeBlock()
{
	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
	//	if(m_pCSGNode)	m_pCSGNode->Release();
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqObjectModeBlock::CqObjectModeBlock( CqModeBlock* pconParent ) : CqModeBlock( pconParent )
{
	// Create new Attributes as they must be pushed/popped by the state change.
	m_pattrCurrent = new CqAttributes();
	ADDREF( m_pattrCurrent );
	m_ptransCurrent = new CqTransform( *pconParent->m_ptransCurrent );
	ADDREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqObjectModeBlock::~CqObjectModeBlock()
{
	RELEASEREF( m_pattrCurrent );
	RELEASEREF( m_ptransCurrent );
}


//---------------------------------------------------------------------
/** Default constructor.
 */

CqMotionModeBlock::CqMotionModeBlock( TqInt N, TqFloat times[], CqModeBlock* pconParent ) : CqModeBlock( pconParent ), m_pDeformingSurface( NULL )
{
	// Copy the parents attributes, as this state change doesn't save attributes.
	if ( pconParent != 0 )
		m_pattrCurrent = pconParent->m_pattrCurrent;
	else
		m_pattrCurrent = new CqAttributes();

	if ( pconParent != 0 )
	{
		m_ptransCurrent = pconParent->m_ptransCurrent;
		// Set the default 'new time slot' matrix to the current 0 time matrix, this
		// takes care of the case of moving from non-Motion to Motion.
		m_ptransCurrent->SetDefaultObject( m_ptransCurrent->GetMotionObject( m_ptransCurrent->Time( 0 ) ) );
	}
	else
		m_ptransCurrent = new CqTransform();


	// Store the array of times.
	TqInt i;
	for ( i = 0; i < N; i++ )
		m_aTimes.push_back( times[ i ] );

	m_iTime = 0;
}


//---------------------------------------------------------------------
/** Destructor.
 */

CqMotionModeBlock::~CqMotionModeBlock()
{}

/** Delete the object context.
 * \attention This is the only valid context deletion from within this block.
 */
void CqMotionModeBlock::EndMotionModeBlock()
{
	if( NULL != m_pDeformingSurface )
	{
		QGetRenderContext() ->pImage() ->PostSurface( m_pDeformingSurface );
		QGetRenderContext() ->Stats().IncGPrims();
		RELEASEREF( m_pDeformingSurface );
	}
}

/** Set the CqDeformingSurface, if generating a deformation motion blur sequence.
 */
void CqMotionModeBlock::SetDeformingSurface( CqDeformingSurface* pDeformingSurface)
{
	m_pDeformingSurface = pDeformingSurface;
	ADDREF( pDeformingSurface );
}


END_NAMESPACE( Aqsis )
