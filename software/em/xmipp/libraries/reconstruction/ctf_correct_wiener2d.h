/***************************************************************************
 * Authors:     AUTHOR_NAME (jvargas@cnb.csic.es)
 *
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#ifndef CTF_CORRECT_WIENER2D_H_
#define CTF_CORRECT_WIENER2D_H_

#include <data/xmipp_program.h>
#include <data/xmipp_funcs.h>
#include <data/metadata.h>
#include <data/xmipp_fftw.h>
#include <data/args.h>
#include <data/ctf.h>
#include <data/xmipp_image.h>
#include <data/filters.h>

/**@defgroup Correct CTF by Wiener filter in 2D
   @ingroup ReconsLibrary */
//@{
class ProgCorrectWiener2D: public XmippProgram
{


public:

    size_t rank, Nprocessors;

    /** Input metadata file */
    FileName fn_input;

    /** Output metadata file */
    FileName fn_out;

    bool phase_flipped;

    /** Padding factor */
    double	pad;

    bool isIsotropic;

protected:

    size_t Xdim, Ydim, Zdim, Ndim;

    ///auxiliary matrices to speed up process
    MultidimArray<double> diff;
    MultidimArray<int> dd;

    /// Wiener filter constant
    double wiener_constant;

    //Input metadata
	MetaData md;


public:

    void readParams();

    void defineParams();

    void run();

public:

    ProgCorrectWiener2D();

    void applyWienerFilter(MultidimArray<double> & Mwien, MultidimArray<double> & img);

    void generateWienerFilter(MultidimArray<double> &Mwien, CTFDescription &ctf);

    /// Gather alignment
    virtual void gatherClusterability() {}

    /// Synchronize with other processors
    virtual void synchronize() {}

protected:

	void produceSideInfo();

};




#endif /* CTF_CORRECT_WIENER2D_H_ */