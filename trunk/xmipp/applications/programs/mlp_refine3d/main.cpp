/***************************************************************************
 *
 * Authors: Sjors Scheres (scheres@cnb.uam.es)
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
 *  e-mail address 'xmipp@cnb.uam.es'
 ***************************************************************************/

#include <reconstruction/ml_refine3d.h>
#include <reconstruction/mlp_align2d.h>

int main(int argc, char **argv)
{

    int                         c, iter, volno, converged = 0;
    std::vector<double>              conv;
    double                      LL, sumw_allrefs, convv, sumcorr, wsum_sigma_noise, wsum_sigma_offset;
    std::vector< Polar <complex<double> > > fP_wsum_imgs;
    std::vector<double>              sumw, sumw_cv, sumw_mirror;
    DocFile                     DFo;

    Prog_Refine3d_prm           prm;
    Prog_MLPalign2D_prm         ML2D_prm;

    // Get input parameters
    try
    {

        // Read command line
        prm.read(argc, argv);
        prm.show();
        // Write starting volume(s) to disc with correct name for iteration loop
        prm.remake_SFvol(prm.istart - 1, true);

        // Read MLalign2D-stuff
        ML2D_prm.read(argc, argv, true);
        ML2D_prm.fn_root = prm.fn_root;
        ML2D_prm.do_mirror = true;
        ML2D_prm.fn_ref = prm.fn_root + "_lib.sel";
        // Project volume and read lots of stuff into memory
        prm.project_reference_volume(ML2D_prm.SFr);
        ML2D_prm.produceSideInfo();
        ML2D_prm.produceSideInfo2(prm.Nvols);
        ML2D_prm.show(true);

        // Initialize some stuff
        for (int refno = 0; refno < ML2D_prm.nr_ref; refno++) conv.push_back(-1.);
        ML2D_prm.Iold.clear(); // To save memory

    }
    catch (Xmipp_error XE)
    {
        std::cout << XE;
        prm.usage();
        exit(0);
    }

    try
    {

        // Loop over all iterations
        iter = prm.istart;
        while (!converged && iter <= prm.Niter)
        {

            if (prm.verb > 0)
            {
                std::cerr        << "--> 3D-EM volume refinement:  iteration " << iter << " of " << prm.Niter << std::endl;
                prm.fh_hist << "--> 3D-EM volume refinement:  iteration " << iter << " of " << prm.Niter << std::endl;
            }

            DFo.clear();
	    DFo.append_comment("Headerinfo columns: rot (1), tilt (2), psi (3), Xoff (4), Yoff (5), Ref (6), Flip (7), Pmax/sumP (8)");

            // Pre-calculate pdfs
            ML2D_prm.updatePdfTranslations();

            // Integrate over all images
            ML2D_prm.sumOverAllImages(ML2D_prm.SF, ML2D_prm.Iref,
				      LL, sumcorr, DFo, fP_wsum_imgs,
				      wsum_sigma_noise, wsum_sigma_offset, sumw, sumw_mirror);

            // Update model parameters
            ML2D_prm.updateParameters(fP_wsum_imgs, wsum_sigma_noise, wsum_sigma_offset,
				      sumw, sumw_mirror, sumcorr, sumw_allrefs);

            // Write intermediate output files
            ML2D_prm.writeOutputFiles(iter, DFo, sumw_allrefs, LL, sumcorr, conv);
            prm.concatenate_selfiles(iter);

	    // Jump out before 3D reconstruction 
	    // (Useful for some parallelization protocols)
	    if (prm.skip_reconstruction) exit(1);

            // Reconstruct new volumes from the reference images
            for (volno = 0; volno < prm.Nvols; volno++)
                prm.reconstruction(argc, argv, iter, volno, 0);

            // Update the reference volume selection file
            // and post-process the volumes (for -FS also the noise volumes!)
            prm.remake_SFvol(iter, false, false);
            prm.post_process_volumes(argc, argv);
            prm.remake_SFvol(iter, false, false);

            // Check convergence
            if (prm.check_convergence(iter))
            {
                converged = 1;
                if (prm.verb > 0) std::cerr << "--> Optimization converged!" << std::endl;
            }

            // Re-project volumes
            if (!converged && iter + 1 <= prm.Niter)
            {
                prm.project_reference_volume(ML2D_prm.SFr);
                // Read new references from disc 
                ML2D_prm.SFr.go_beginning();
                c = 0;
                while (!ML2D_prm.SFr.eof())
                {
                    FileName fn_img=ML2D_prm.SFr.NextImg();
                    if (fn_img=="") break;
                    ML2D_prm.Iref[c].read(fn_img, false, false, false, false);
                    ML2D_prm.Iref[c]().setXmippOrigin();
                    c++;
                }
            }

            iter++;
        } // end loop iterations

	// Write out converged doc and logfiles
        ML2D_prm.writeOutputFiles(-1, DFo, sumw_allrefs, LL, sumcorr, conv);

        if (!converged && prm.verb > 0) std::cerr << "--> Optimization was stopped before convergence was reached!" << std::endl;
    }

    catch (Xmipp_error XE)
    {
        std::cout << XE;
        prm.usage();
        exit(0);
    }

}




