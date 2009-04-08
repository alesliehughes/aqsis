/* Aqsis - bucketprocessor.h
 *
 * Copyright (C) 2007 Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/** \file
 *
 * \brief File holding code to process buckets.
 *
 * \author Manuel A. Fernadez Montecelo <mafm@users.sourceforge.net>
 */

#ifndef BUCKETPROCESSOR_H_INCLUDED
#define BUCKETPROCESSOR_H_INCLUDED 1

#include	"aqsis.h"

#include	<boost/array.hpp>

#include	"bucket.h"
#include	"channelbuffer.h"
#include	"occlusion.h"
#include	"isampler.h"


namespace Aqsis {


/**
 * \brief Class to process Buckets.
 */
class CqBucketProcessor
{
	public:
		/** Default constructor */
		CqBucketProcessor();
		/** Destructor */
		~CqBucketProcessor();

		/** Set the bucket to be processed */
		void setBucket(CqBucket* bucket);
		/** Get the bucket to be processed */
		const CqBucket* getBucket() const;

		static void	setupCacheInformation();

		/** Reset the status of the object */
		void reset();

		/** Prepare the data for the bucket to be processed */
		void preProcess(IqSampler* sampler);

		/** Process the bucket, basically rendering the waiting MPs
		 */
		void process();

		/** Post-process the bucket, which involves the operations
		 * Combine and Filter
		 */
		void postProcess( bool imager, EqFilterDepth depthfilter, const CqColor& zThreshold );

		//-------------- Reorganise -------------------------
		
		CqChannelBuffer& getChannelBuffer();

		TqUint numSamples() const;

		const CqRegion& SampleRegion() const;
		const CqRegion& DisplayRegion() const;
		const CqRegion& DataRegion() const;

		std::vector<CqImagePixelPtr>&	pixels();
		const std::vector<CqImagePixelPtr>&	pixels() const;

	private:
		void	InitialiseFilterValues();
		void	CalculateDofBounds();
		void	CombineElements(enum EqFilterDepth eDepthFilter, CqColor zThreshold);
		void	FilterBucket(bool fImager);
		void	ExposeBucket();

		void	buildCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, boost::shared_ptr<SqBucketCacheSegment>& seg);
		void	applyCacheSegment(SqBucketCacheSegment::EqBucketCacheSide side, const boost::shared_ptr<SqBucketCacheSegment>& seg);
		void	dropSegment(TqInt side);

		TqFloat	FilterXWidth() const;
		TqFloat	FilterYWidth() const;
		TqInt	PixelXSamples() const;
		TqInt	PixelYSamples() const;
		void ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixelPtr*& pie );

		CqImagePixel& ImageElement(TqUint index) const;
		/** Render any waiting MPs.
		 */
		void RenderWaitingMPs();
		void RenderSurface( boost::shared_ptr<CqSurface>& surface);
		void ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie ) const;
		/** Render a particular micropolygon.
		 *
		 * \param pMPG Pointer to the micropolygon to process.
		 * \see CqBucket, CqImagePixel
		 */
		void	RenderMicroPoly( CqMicroPolygon* pMP );
		void	RenderMPG_MBOrDof( CqMicroPolygon* pMP, bool IsMoving, bool UsingDof );
		void	StoreSample( CqMicroPolygon* pMPG, CqImagePixel* pie2, TqInt index, TqFloat D );
		void	StoreExtraData( CqMicroPolygon* pMPG, TqFloat* hitData);
		/** This function assumes that neither dof or mb are
		 * being used. It is much simpler than the general
		 * case dealt with above. */
		void	RenderMPG_Static( CqMicroPolygon* pMPG);
		/** This function assumes that either dof or mb or
		 * both are being used. */
		bool 	occlusionCullSurface(const boost::shared_ptr<CqSurface>& surface);
		const CqBound& DofSubBound(TqInt index) const;


		/// Pointer to the current bucket
		CqBucket* m_bucket;

		/// Bucket data for the current bucket
		TqInt	m_NumDofBounds;

		std::vector<CqBound>		m_DofBounds;
		std::vector<CqImagePixelPtr>	m_aieImage;

		/// Vector of filter weights precalculated.
		std::vector<TqFloat>	m_aFilterValues;

		SqMpgSampleInfo m_CurrentMpgSampleInfo;

		CqOcclusionTree m_OcclusionTree;

		// View range and clipping info (to know when to skip rendering)
		/// The total size of the array of sample available for this bucket.
		CqRegion	m_DataRegion;
		/// The area of the samples array that will be used to sample micropolygons.
		CqRegion	m_SampleRegion;
		/// The area of the samples array that will be sent to the display. 
		CqRegion	m_DisplayRegion;

		bool	m_hasValidSamples;

		CqChannelBuffer	m_channelBuffer;

		static TqInt	m_DiscreteShiftX;
		static TqInt	m_DiscreteShiftY;
		static TqInt	m_PixelXSamples;
		static TqInt	m_PixelYSamples;
		static TqFloat	m_FilterXWidth;
		static TqFloat	m_FilterYWidth;

		static TqFloat	m_clippingNear;
		static TqFloat	m_clippingFar;
		static boost::array<CqRegion, SqBucketCacheSegment::last> m_cacheRegions;
};

inline const CqRegion& CqBucketProcessor::DataRegion() const
{
	return m_DataRegion;
}

inline const CqRegion& CqBucketProcessor::SampleRegion() const
{
	return m_SampleRegion;
}

inline const CqRegion& CqBucketProcessor::DisplayRegion() const
{
	return m_DisplayRegion;
}

inline TqFloat	CqBucketProcessor::FilterXWidth() const
{
	return m_FilterXWidth;
}

inline TqFloat	CqBucketProcessor::FilterYWidth() const
{
	return m_FilterYWidth;
}

inline TqInt	CqBucketProcessor::PixelXSamples() const
{
	return m_PixelXSamples;
}

inline TqInt	CqBucketProcessor::PixelYSamples() const
{
	return m_PixelYSamples;
}

inline TqUint CqBucketProcessor::numSamples() const
{
	return DataRegion().area() * PixelXSamples() * PixelYSamples();
}

inline CqChannelBuffer& CqBucketProcessor::getChannelBuffer()
{
	return m_channelBuffer;
}

inline const CqBound& CqBucketProcessor::DofSubBound(TqInt index) const
{
	assert(index < m_NumDofBounds);
	return m_DofBounds[index];
}

inline std::vector<CqImagePixelPtr>& CqBucketProcessor::pixels()
{
	return m_aieImage;
}

inline const std::vector<CqImagePixelPtr>& CqBucketProcessor::pixels() const
{
	return m_aieImage;
}

} // namespace Aqsis

#endif
