// Aqsis
// Copyright (C) 2014, Alistair Leslie-Hughes and the other authors and contributors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name of the software's owners nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// (This is the New BSD license)

/** \file
 *
 * \brief Interface wrapper functions and factory functions for texture file
 * interface.
 *
 * \author Alistair Leslie-Hughes
 */
#ifndef JPEGINPUTFILE_H_INCLUDED
#define JPEGINPUTFILE_H_INCLUDED

#include <aqsis/aqsis.h>

#include <aqsis/tex/io/itexinputfile.h>

namespace Aqsis
{

class AQSIS_TEX_SHARE CqJPEGInputFile : public IqTexInputFile
{
	public:
		CqJPEGInputFile(const boostfs::path& fileName);

		virtual boostfs::path fileName() const;
		virtual EqImageFileType fileType() const;
		virtual const CqTexFileHeader& header() const;

		virtual void readPixelsImpl(TqUint8* buffer, TqInt startLine,
				TqInt numScanlines) const;
	private:
		/// Header information
		CqTexFileHeader m_header;
		/// Name of the file
		const boostfs::path m_fileName;
		/// Input stream from which to read data
		mutable std::ifstream m_fileStream;
		/// Position of the start of depth data in the file.
		std::istream::pos_type m_dataBegin;
};


};



#endif /* JPEGINPUTFILE_H_INCLUDED */

