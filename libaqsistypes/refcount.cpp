// Aqsis
// Copyright � 1997 - 2001, Paul C. Gregory
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
	\brief Provides a debug-build version of CqRefCount.
	\author Jonathan Merritt (j.merritt@pgrad.unimelb.edu.au)
*/

#include "refcount.h"

/// Only include this file under a debug build.  The methods included
/// here are the debug implementations.  For the non-debug
/// implementations, please see the file refcount.h, where they are
/// inlined.
#ifdef _DEBUG

#warning Using the debug reference counting scheme.

#include "aqsis.h"
#include "sstring.h"
#include <stdlib.h>


/**
 * \class RefCountTracker
 *
 * This class, which should only be instantiated once in the program,
 * keeps a list of all un-released CqRefCount objects.  When the
 * rendering has finished, and all CqRefCount objects ought to have
 * been released, then the report_refcounts() global-scope function
 * should be called, which in turn calls the report() method on the
 * one global instance of this class.  The report will list all
 * un-released CqRefCounts objects which are still hanging around,
 * and provide a history of each one's ADDREF() and RELEASEREF()
 * calls.
 *
 * I resisted the temptation to call this class "Agent Smith" ;-)
 */
typedef std::list<CqRefCount*> RefCountVector;
typedef std::list<CqRefCount*>::iterator RefCountIterator;
class RefCountTracker {
	public:
		/**
		 * Adds a reference count object to the global list.
		 * This method should be called when the object is first
		 * created (ie: not each time ADDREF is called!).
		 *
		 * \param refCount	CqRefCount object to add.
		 */
		static void addRefCountObj(CqRefCount *refCount) {
			m_tracker->_addRefCountObj(refCount);
		}
		/**
		 * Removes a reference count object from the global list.
		 * This method should be called just before the reference
		 * count object is finally released, after reaching zero
		 * references (ie: not each time RELEASEREF is called!).
		 *
		 * \param refCount	CqRefCount object to remove.
		 */
		static void removeRefCountObj(CqRefCount *refCount) {
			m_tracker->_removeRefCountObj(refCount);
		}
		/**
		 * Reports on the list of un-released CqRefCount objects.
		 * Ideally, don't call this method directly - use the
		 * global-scope function report_refcounts() instead.
		 */
		static void report() {
			m_tracker->_report();
		}

	private:
		void _addRefCountObj(CqRefCount *refCount) {
			m_refCountObjs.push_back(refCount);
		}
		void _removeRefCountObj(CqRefCount *refCount) {
			m_refCountObjs.remove(refCount);
		}
		void _report() {
                        RefCountIterator end = m_refCountObjs.end();
                        for (RefCountIterator i = m_refCountObjs.begin(); i != end; i++)
                        {
                                std::cout << (*i)->stringHistory();
                        }
			TqInt nObjs = m_refCountObjs.size();
			if (nObjs == 0)
			{
				std::cout << "There were no un-released CqRefCounts.\n";
			}
			else
			{
	                        std::cout << "There were " << nObjs
	                                << " un-released CqRefCount-derived classes.";
			}
			std::cout.flush();
		}
	private:
		RefCountVector m_refCountObjs;
		static RefCountTracker *m_tracker;
	
};
/**
 * The one global instance of RefCountTracker.
 */
RefCountTracker *RefCountTracker::m_tracker = new RefCountTracker();
/**
 * Reports on the un-release CqRefCount objects.  This
 * function (which is declared in refcount.h) should be called
 * once rendering has finished and all CqRefCount objects ought
 * to have been released.
 */
void report_refcounts() {
	RefCountTracker::report();
}


/**
 * \class RefCountRecord
 *
 * This class stores a record of a reference counting event - either
 * an AddRef() or Release().  The class records the name of the file
 * where the event occurred, the line number from the file, and
 * whether the event was an AddRef() or a Release() call.
 *
 * This is provided so that the history of a RefCount-derived class
 * can be tracked through it's AddRef() and Release() calls, to
 * allow easier debugging.
 */
class RefCountRecord {
	public:
		enum EventType {
			AddRef, Release
		};
		/**
		 * Constructor.
		 *
		 * \param fileName	Name of the file where a reference
		 * 			counting event occurred.
		 * \param lineNumber	Line number from the file where
		 * 			the event occurred.
		 * \param eventType	Type of the event.
		 */
		RefCountRecord(const TqChar *fileName, TqInt lineNumber, EventType eventType)
		{
			m_fileName   = Aqsis::CqString(fileName);
			m_lineNumber = lineNumber;
			m_eventType  = eventType;
		}
		/**
		 * Returns the filename from the record.  This is the
		 * name of the file where a reference counting event
		 * occurred.
		 */
		const Aqsis::CqString fileName() const
		{
			return m_fileName;
		}
		/**
		 * Returns the line number from the record.  This is
		 * the number of the line from the file where the
		 * reference counting event occurred.
		 */
		TqInt lineNumber() const
		{
			return m_lineNumber;
		}
		/**
		 * Returns the type of event that this record represents.
		 * This can be either an AddRef or a Release event.
		 */
		EventType eventType() const
		{
			return m_eventType;
		}
		/**
		 * Returns a string representation of the event, which
		 * includes the event type, the filename and line number.
		 */
		Aqsis::CqString toString() const
		{
			Aqsis::CqString str;

			// put in the event type
			if (eventType() == AddRef)
				str += "AddRef:  ";
			else if (eventType() == Release)
				str += "Release: ";
			else
				throw;

			// now put in the file name and line
			str += fileName();
			str += ": ";
			str += lineNumber();

			return str;
		}
	
	private:
		// Name of the file where the reference count event
		//  occurred.
		Aqsis::CqString m_fileName;
		// Line number from the file where the reference
		//  count event occurred.
		TqInt m_lineNumber;
		// Type of the event (either an AddRef or a Release).
		EventType m_eventType;
	
};


CqRefCount::CqRefCount() : m_cReferences( 0 )
{
	// Add the reference count object to the global list.
	RefCountTracker::addRefCountObj( this );

}

CqRefCount::CqRefCount( const CqRefCount& From )
{

	// Add the new reference count object to the global list.
	RefCountTracker::addRefCountObj( this );
	
}

CqRefCount::~CqRefCount()
{

	// Delete any reference counting records that are stored.
	RecordIterator end = m_records.end();
	for (RecordIterator i = m_records.begin(); i != end; i++)
	{
		// This deletes an object of type RefCountRecord*
		delete ( *i );
	}
	m_records.clear();

	// Remove the reference count object from the global list
	//  so that it is no longer included in the un-released
	//  list.
	RefCountTracker::removeRefCountObj( this );
	
}

TqInt CqRefCount::RefCount() const
{
	return ( m_cReferences );
}

void CqRefCount::AddRef(const TqChar* file, TqInt line)
{
	// Increment the number of references
	m_cReferences++;

	// Record the AddRef event.
	RefCountRecord *rcr = new RefCountRecord(
		file, line, RefCountRecord::AddRef);
	m_records.push_back(rcr);
	
}

void CqRefCount::Release(const TqChar* file, TqInt line)
{
	// Decrement the number of references.
	m_cReferences--;

	// Record the Release event.
	RefCountRecord *rcr = new RefCountRecord(
		file, line, RefCountRecord::Release);
	m_records.push_back(rcr);
	
	// Delete the reference counting class if there are no more
	//  references to it.
	if (m_cReferences <= 0)
	{
		// free the memory
		delete( this );
	}
}

/**
 * Returns the history of this CqRefCount object, which is a list of
 * where it had AddRef and Release called.
 */
Aqsis::CqString CqRefCount::stringHistory() const 
{
	Aqsis::CqString str;

	str += "Reference history for un-freed CqRefCount class of type: ";
	str += className();
	str += "\n";
	
        RecordIterator end = m_records.end();
        for (RecordIterator i = m_records.begin(); i != end; i++)
	{
		str += (*i)->toString();
		str += "\n";
	}
	
	str += m_cReferences;
	str += " references were left over in excess.\n";
	
	return str;
}

#endif ///< #ifdef _DEBUG

