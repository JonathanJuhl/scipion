/***************************************************************************
 *
 * Authors:    Sjors Scheres           scheres@cnb.csic.es (2004)
 *
 * Unidad de Bioinformatica del Centro Nacional de Biotecnologia , CSIC
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

#include "ml_refine3d.h"

#include <data/fft.h>
#include <data/args.h>
#include <data/funcs.h>
#include <data/filters.h>
#include <data/mask.h>
#include <data/morphology.h>
#include <data/grids.h>
#include <data/blobs.h>
#include <data/symmetries.h>
#include <data/projection.h>
#include "directions.h"
#include "reconstruct_art.h"
#include "reconstruct_fourier.h"
#include "fourier_filter.h"
#include "symmetrize.h"
#include "volume_segment.h"

//#define DEBUG
//Macro to obtain the iteration base name
#define FN_ITER_BASE(iter) formatString("%s_iter%06d_vols.stk", fn_root.c_str(), (iter))
#define FN_INITIAL_BASE (fn_root + "_iter000000_vols.stk")
#define FN_NOISE_MD (fn_root + "_noise.xmd")
#define FN_NOISE (fn_root + "_noise.stk")
#define FN_CREF_MD (fn_root + "_cref.xmd")
#define FN_CREF (fn_root + "_cref.stk")
#define FN_PROJECTIONS_MD (fn_root + "_projections.xmd")
#define FN_PROJECTIONS (fn_root + "_projections.stk")

ProgMLRefine3D::ProgMLRefine3D(bool fourier)
{
    fourier_mode = fourier;
    if (fourier)
        ml2d = new ProgMLF2D();
    else
        ml2d = new ProgML2D();
    rank = 0;
    size = 1;
}

ProgMLRefine3D::~ProgMLRefine3D()
{
    delete ml2d;
}

// Usage ===================================================================
void ProgMLRefine3D::defineParams()
{
    addUsageLine("Separate structurally heterogenous data sets into homogeneous classes by a");
    addUsageLine("multi-reference 3D-angular refinement using a maximum-likelihood(ML) target function.");
    //Add some params from 2D
    //Setting some flags for the params definitions
    ml2d->defaultNiter = 25;
    ml2d->referenceExclusive = false;
    ml2d->allowFastOption = false;
    ml2d->allowRestart = false;

    //basic params
    ml2d->defineBasicParams(this);
    addParamsLine(" [ --ang <float=10> ]           : Angular sampling (degrees) ");
    //extra params
    ml2d->defineAdditionalParams(this, "==+ ML additional options: ==");
    addParamsLine("==+ Additional options: ==");
    addParamsLine("--recons <recons_type=wlsART>       : Reconstruction method to be used");
    addParamsLine("       where <recons_type>           ");
    addParamsLine("           wlsART  <params=\"\">     : wlsART parameters");
    addParamsLine("           fourier <params=\"\">     : fourier parameters");
    addParamsLine(" [ --nostart ]                      : Start wlsART reconstructions from all-zero volumes ");
    addParamsLine(" [ --sym <symfile=c1> ]             : Symmetry group ");
    addParamsLine(" [ --low_pass <freq=-1> ]           : Low-pass filter volume every iteration ");
    addParamsLine(" [ --sym_mask <maskfile=\"\"> ]     : Local symmetry (only inside mask) ");
    addParamsLine(" [ --tilt <min=-91.> <max=91.> ]    : Minimum and maximum values for restriction tilt angle search ");
    addParamsLine(" [ --perturb ]                      : Randomly perturb reference projection directions ");
    //hidden params
    addParamsLine("==+++++ Hidden arguments ==");
    addParamsLine(" [--solvent <filename=\"\">]");
    addParamsLine(" [--prob_solvent]");
    addParamsLine(" [--threshold_solvent <float=999.>]");
    addParamsLine(" [--deblob_solvent]");
    addParamsLine(" [--dilate_solvent <int=0>]");
    addParamsLine(" [--skip_reconstruction]");
}

// Read ===================================================================
void ProgMLRefine3D::readParams()
{
    bool do_restart = false;

    // Generate new command line for restart procedure
    /// FIXME: restart has to be re-thought
    /*
    if (checkParam( "-restart"))
{
        String   comment, cline = "";
        DocFile  DFi;
        FileName fn_tmp;

        do_restart = true;
        DFi.read(getParameter(argc, argv, "-restart"));
        DFi.go_beginning();
        comment = (DFi.get_current_line()).get_text();
        if (fourier_mode && strstr(comment.c_str(), "MLFalign2D-logfile") == NULL)
        {
            std::cerr << "Error!! Docfile is not of MLFalign2D-logfile type. " << std::endl;
            exit(1);
        }
        else if (!fourier_mode && strstr(comment.c_str(), "MLalign2D-logfile") == NULL)
        {
            std::cerr << "Error!! Docfile is not of MLalign2D-logfile type. " << std::endl;
            exit(1);
        }
        else
        {
            char *copy;
            copy = NULL;
            DFi.next();
            // get new part of command line (with -istart)
            comment = (DFi.get_current_line()).get_text();
            DFi.next();
            // get original command line
            cline = (DFi.get_current_line()).get_text();
            comment = comment + cline;
            // regenerate command line
            argv2 = NULL;
            argc2 = 0;
            generateCommandLine(comment, argc2, argv2, copy);
            // Get number of volumes and names to generate SFvol
            if (fourier_mode)
                fn_root = getParam( "-o", "mlf3d");
            else
                fn_root = getParam( "-o", "ml3d");
            fn_vol = getParam( "-vol");
            istart = getIntParam( "-istart"));
            if (Is_VolumeXmipp(fn_vol))
            {
                SFvol.clear();
                SFvol.addObject();
                SFvol.setValue(MDL_IMAGE, fn_vol);
                SFvol.setValue(MDL_ENABLED, 1);
            }
            else
            {
                SFvol.read(fn_vol);
            }
            SFvol.removeObjects(MDValueEQ(MDL_ENABLED, -1));
            Nvols = SFvol.size();

            SFvol.clear();
            for (int ivol = 0; ivol < Nvols; ivol++)
            {
                fn_tmp = fn_root + "_it";
                fn_tmp.compose(fn_tmp, istart - 1, "");
                if (Nvols > 1)
                {
                    fn_tmp += "_vol";
                    fn_tmp.compose(fn_tmp, ivol + 1, "");
                }
                fn_tmp += ".vol";
                SFvol.addObject();
                SFvol.setValue(MDL_IMAGE, fn_tmp);
                SFvol.setValue(MDL_ENABLED, 1):
                    }
            fn_vol = fn_root + "_it";
            fn_vol.compose(fn_vol, istart - 1, "");
            fn_vol += "_restart.sel";
            SFvol.write(fn_vol);
        }
}
    else
{
    */
    // no restart, just copy argc to argc2 and argv to argv2
    //argc2 = argc;
    //argv2 = argv;
    //   }


    //Read Refine3d parameters
    fn_sel = getParam( "-i");
    fn_root = getParam("--oroot");

    //    if (fourier_mode)
    //        fn_root = getParam( "-o", "mlf3d");
    //    else
    //        fn_root = getParam( "-o", "ml3d");

    if (!do_restart)
    {
        // Fill volume selfile
        fn_ref = getParam( "--ref");
        mdVol.read(fn_ref);
        Nvols = mdVol.size();
    }

    angular = getDoubleParam( "--ang");
    fn_sym = getParam( "--sym");
    eps = getDoubleParam( "--eps");
    Niter = getIntParam( "--iter");
    istart = 1;//getIntParam( "-istart");
    tilt_range0 = getDoubleParam( "--tilt", 0);
    tilt_rangeF = getDoubleParam( "--tilt", 1);
    fn_symmask = getParam( "--sym_mask");
    lowpass = getDoubleParam( "--low_pass");

    wlsart_no_start = checkParam( "--nostart");
    //    if (checkParam("--wlsart"))
    //    {
    //        wlsart_lambda = getDoubleParam("--wlsart", 0);
    //        wlsart_kappa = getDoubleParam("--wlsart", 1);
    //        wlsart_Niter = getIntParam("--wlsart", 2);
    //    }
    do_perturb = checkParam( "--perturb");

    // Hidden for now
    fn_solv = getParam( "--solvent");

    if (STR_EQUAL(getParam("--recons"), "wlsART"))
        recons_type = RECONS_ART;
    else if (STR_EQUAL(getParam("--recons"), "fourier"))
        recons_type = RECONS_FOURIER;

    do_prob_solvent = checkParam( "--prob_solvent");
    threshold_solvent = getDoubleParam( "--threshold_solvent");
    do_deblob_solvent = checkParam( "--deblob_solvent");
    dilate_solvent = getIntParam( "--dilate_solvent");
    skip_reconstruction = checkParam( "--skip_reconstruction");

    // Checks
    if (lowpass > 0.5)
        REPORT_ERROR(ERR_VALUE_INCORRECT, "Digital frequency for low-pass filter should be smaller than 0.5");

    //Read ml2d params
    ml2d->do_ML3D = true;
    ml2d->read(argc, argv, false);

    if (!checkParam("--psi_step"))
        ml2d->psi_step = angular;
    ml2d->fn_img = fn_sel;
    ml2d->fn_root = fn_root + "_ml2d";
    ml2d->fn_ref = FN_PROJECTIONS_MD;
}

void ProgMLRefine3D::show()
{
    if (verbose == 0)
        return;

    std::cout << " -----------------------------------------------------------------" << std::endl;
    std::cout << " | Read more about this program in the following publication:    |" << std::endl;
    if (fourier_mode)
        std::cout << " |  Scheres ea. (2007)  Structure, 15, 1167-1177                 |" << std::endl;
    else
        std::cout << " |  Scheres ea. (2007)  Nature Methods, 4, 27-29                 |" << std::endl;
    std::cout << " |                                                               |" << std::endl;
    std::cout << " |    *** Please cite it if this program is of use to you! ***   |" << std::endl;
    std::cout << " -----------------------------------------------------------------" << std::endl;
    std::cout << "--> Maximum-likelihood multi-reference 3D-refinement" << std::endl;
    if (Nvols == 1)
        std::cout << "  Initial reference volume : " << fn_ref << std::endl;
    else
    {
        std::cout << "  Selfile with references  : " << fn_ref << std::endl;
        std::cout << "    with # of volumes      : " << Nvols << std::endl;
    }
    std::cout << "  Experimental images:     : " << fn_sel << std::endl;
    std::cout << "  Angular sampling rate    : " << angular << std::endl;
    std::cout << "  Symmetry group:          : " << fn_sym << std::endl;
    if (fn_symmask != "")
        std::cout << "  Local symmetry mask      : " << fn_symmask << std::endl;
    std::cout << "  Output rootname          : " << fn_root << std::endl;
    std::cout << "  Convergence criterion    : " << eps << std::endl;
    if (lowpass > 0)
        std::cout << "  Low-pass filter          : " << lowpass << std::endl;
    if (tilt_range0 > -91. || tilt_rangeF < 91.)
        std::cout << "  Limited tilt range       : " << tilt_range0 << "  " << tilt_rangeF << std::endl;
    if (wlsart_no_start)
        std::cout << "  -> Start wlsART reconstructions from all-zero volumes " << std::endl;
    if (recons_type == RECONS_FOURIER)
        std::cout << "  -> Use fourier-interpolation instead of wlsART for reconstruction" << std::endl;
    if (do_prob_solvent)
        std::cout << "  -> Perform probabilistic solvent flattening" << std::endl;
    std::cout << " -----------------------------------------------------------------" << std::endl;
}

void ProgMLRefine3D::createSampling()
{
    FileName fn_sym_loc;
    // Precalculate sampling
    mysampling.SetSampling(angular);
    fn_sym_loc = fn_symmask.empty() ? fn_sym : "c1";

    if (!mysampling.SL.isSymmetryGroup(fn_sym_loc, symmetry, sym_order))
        REPORT_ERROR(ERR_NUMERICAL, (String)"ml_refine3d::run Invalid symmetry" +  fn_sym_loc);
    mysampling.SL.read_sym_file(fn_sym_loc);
    mysampling.Compute_sampling_points(true, tilt_rangeF, tilt_range0);
    mysampling.remove_redundant_points_exhaustive(symmetry, sym_order, true, 0.75 * angular);
    nr_projections = mysampling.no_redundant_sampling_points_angles.size();

}

// Fill sampling and create DFlib
void ProgMLRefine3D::produceSideInfo()
{
    //Create sampling
    createSampling();
    show();
    // Write starting volume(s) to disc with correct name for iteration loop
    copyVolumes();
    // Project volumes and store projections in a metadata
    projectVolumes(ml2d->MDref);
    //2d initialization
    ml2d->produceSideInfo();
}

void ProgMLRefine3D::produceSideInfo2()
{
    ml2d->produceSideInfo2();
    ml2d->refs_per_class = nr_projections;
    ml2d->show();
    Nvols *= ml2d->factor_nref;
    ml2d->Iold.clear(); // To save memory
}

void ProgMLRefine3D::run()
{
    bool converged = false;

    // Get input parameters
    produceSideInfo();

    produceSideInfo2();

    ml2d->createThreads();

    // Loop over all iterations
    for (ml2d->iter = ml2d->istart; !converged && ml2d->iter <= ml2d->Niter; ml2d->iter++)
    {
        iter = ml2d->iter; //keep updated the iter class variable

        if (verbose)
            std::cout << formatString("--> 3D-EM volume refinement:  iteration %d of %d", iter, Niter) << std::endl;

        for (ml2d->current_block = 0; ml2d->current_block < ml2d->blocks; ml2d->current_block++)
        {
            // Project volumes, already done for first iteration, first block
            if (ml2d->iter > ml2d->istart || ml2d->current_block > 0)
            {
                projectVolumes(ml2d->MDref);
                size_t refno = 0;
                FileName fn;
                // Read new references from disc (I could just as well keep them in memory, maybe...)
                FOR_ALL_OBJECTS_IN_METADATA(ml2d->MDref)
                {
                    ml2d->MDref.getValue(MDL_IMAGE, fn, __iter.objId);
                    ml2d->model.Iref[refno].read(fn);
                    ml2d->model.Iref[refno]().setXmippOrigin();
                    ++refno;
                }
            }

            // Integrate over all images
            ml2d->expectation();

            ml2d->maximization();

            // Write out 2D reference images (to be used in reconstruction)
            ml2d->writeOutputFiles(ml2d->model, OUT_REFS);

            // Jump out before 3D reconstruction
            // (Useful for some parallelization protocols)
            if (skip_reconstruction)
                exit(1);

            // Reconstruct new volumes from the reference images
            reconstructVolumes(ml2d->MDref, FN_ITER_BASE(iter));

            // Update the reference volume selection file
            //remakeSFvol(ml2d->iter, false, false);
            updateVolumesMetadata();
            // post-process the volumes
            postProcessVolumes();

        } // end loop blocks

        // Check convergence
        converged = checkConvergence();

        // Write output ML2D files
        ml2d->addPartialDocfileData(ml2d->docfiledata, ml2d->myFirstImg, ml2d->myLastImg);
        ml2d->writeOutputFiles(ml2d->model, OUT_IMGS);

        //concatenateSelfiles(ml2d->iter);

    } // end loop iterations

    if (verbose)
    {
        std::cout << (converged ?
                      "--> Optimization converged!" :
                      "--> Optimization was stopped before convergence was reached!")
        << std::endl;
    }

    // Write converged output ML2D files
    ml2d->writeOutputFiles(ml2d->model, OUT_FINAL);
    ml2d->destroyThreads();

}//end of function run

// Projection of the reference (blob) volume =================================
void ProgMLRefine3D::projectVolumes(MetaData &mdProj)
{

    Image<double>                vol;
    FileName                      fn_base = FN_PROJECTIONS, fn_tmp;
    Projection                    proj;
    double                       rot, tilt, psi = 0.;
    size_t                        nl, nr_dir, my_rank, id, bar_step;
    int                           volno;


    // Here all nodes fill SFlib and DFlib, but each node actually projects
    // only a part of the projections. In this way parallellization is obtained
    // Total number of projections
    nl = Nvols * nr_projections;
    bar_step = XMIPP_MAX(1, nl / 60);

    // Initialize projections output metadata
    mdProj.clear();

    if (verbose)
    {
        std::cout << formatString("--> projecting %d volumes x %d projections...", Nvols, nr_projections) << std::endl;
        init_progress_bar(nl);
    }

    // Loop over all reference volumes
    volno = nr_dir = 0;

    FOR_ALL_OBJECTS_IN_METADATA(mdVol)
    {
        mdVol.getValue(MDL_IMAGE, fn_tmp, __iter.objId);
        vol.read(fn_tmp);
        vol().setXmippOrigin();
        ++volno;

        for (int ilib = 0; ilib < nr_projections; ++ilib)
        {
            ++nr_dir;
            fn_tmp.compose(nr_dir, fn_base);
            rot = XX(mysampling.no_redundant_sampling_points_angles[ilib]);
            tilt = YY(mysampling.no_redundant_sampling_points_angles[ilib]);

            // Parallelization: each rank projects and writes a different direction
            my_rank = ilib % size;
            if (rank == my_rank)
            {
                projectVolume(vol(), proj, vol().rowNumber(), vol().colNumber(), rot, tilt, psi);
                //proj.setEulerAngles(rot, tilt, psi);
                proj.write(fn_tmp);
                //proj.write(formatString("%s_iter%d.stk", fn_tmp.c_str(), iter));
            }

            id = mdProj.addObject();
            mdProj.setValue(MDL_IMAGE, fn_tmp, id);
            mdProj.setValue(MDL_ENABLED, 1, id);
            mdProj.setValue(MDL_ANGLEROT, rot, id);
            mdProj.setValue(MDL_ANGLETILT, tilt, id);
            mdProj.setValue(MDL_ANGLEPSI, psi, id);
            mdProj.setValue(MDL_REF3D, volno, id);

            if (verbose && (nr_dir % bar_step == 0))
                progress_bar(nr_dir);
        }

    }

    if (verbose)
    {
        progress_bar(nl);
        std::cout << " -----------------------------------------------------------------" << std::endl;
    }

    // Only the master write the complete SFlib
    if (rank == 0)
    {
        fn_tmp = FN_PROJECTIONS_MD;
        mdProj.write(fn_tmp);
    }

}

// Make noise images for 3D SSNR calculation ===================================
void ProgMLRefine3D::makeNoiseImages(std::vector<Image<double> > &Iref)
{

    Image<double> img;
    FileName   fn_noise(FN_NOISE), fn_img;
    MetaData    mdNoise;
    size_t volno, id;


    for (int i = 0; i < Iref.size(); i++)
    {
        volno = i / nr_projections + 1;
        img = Iref[i];
        img().initZeros();
        img().addNoise(0, 1, "gaussian");

        if (Iref[i].weight() > 1.)
            img() /= sqrt(Iref[i].weight());

        fn_img.compose(volno, fn_noise);
        img.write(fn_img);

        id = mdNoise.addObject();
        mdNoise.setValue(MDL_IMAGE, fn_img, id);
        mdNoise.setValue(MDL_ENABLED, 1, id);
        //New for metadata: store angles and weights in metadata
        mdNoise.setValue(MDL_ANGLEROT, Iref[i].rot(), id);
        mdNoise.setValue(MDL_ANGLETILT, Iref[i].tilt(), id);
        mdNoise.setValue(MDL_ANGLEPSI, Iref[i].psi(), id);
        mdNoise.setValue(MDL_WEIGHT, Iref[i].weight(), id);
        mdNoise.setValue(MDL_REF3D, volno, id);
    }
    mdNoise.write(FN_NOISE_MD);

}

ProgReconsBase * ProgMLRefine3D::createReconsProgram()
{
    //get reconstruction extra params
    String arguments = getParam("--recons", 1);

    if (recons_type == RECONS_FOURIER)
    {
        ProgRecFourier * program = new ProgRecFourier();
        //force use of weights and the verbosity will be the same of this program
        //-i and -o options are passed for avoiding errors, this should be changed
        //when reconstructing
        arguments += formatString(" --weight -v 0 --thr %d -i iii -o ooo", ml2d->threads);
        program->read(arguments);
        return program;
    }
    else if (recons_type == RECONS_ART)//use of wlsArt
    {
        REPORT_ERROR(ERR_NOT_IMPLEMENTED,"not implemented reconstruction throught wlsArt");
        //        Basic_ART_Parameters   art_prm;
        //        Plain_ART_Parameters   dummy;
        //        GridVolume             new_blobs;
        //        GridVolume             start_blobs;
        //        if (verbose)
        //            std::cerr << "--> weighted least-squares ART reconstruction " << std::endl;
        //
        //        // Read ART parameters from command line & I/O with outer loop of Refine3d
        //        art_prm.read(argc, argv);
        //        art_prm.WLS = true;
        //        if (fn_symmask != "")
        //            art_prm.fn_sym = "";
        //        if (!checkParam( "-n"))
        //            art_prm.no_it = 10;
        //        if (!checkParam( "-l"))
        //        {
        //            art_prm.lambda_list.resize(1);
        //            art_prm.lambda_list.initConstant(0.2);
        //        }
        //        if (!checkParam( "-k"))
        //        {
        //            art_prm.kappa_list.resize(1);
        //            art_prm.kappa_list.initConstant(0.5);
        //        }
        //        art_prm.fn_sel = "xxxx"; //fixme
        //        art_prm.fn_root = "xxxx";
        //        if (noise == 1 || noise == 2)
        //        {
        //            art_prm.fn_start = "";
        //            art_prm.tell = false;
        //        }
        //        else if (!wlsart_no_start)
        //        {
        //            art_prm.tell = TELL_SAVE_BASIS;
        //            art_prm.fn_start = fn_blob;
        //        }
        //        // Reconstruct using weighted least-squares ART
        //        Basic_ROUT_Art(art_prm, dummy, new_vol, new_blobs);
    }
}

// Reconstruction using the ML-weights ==========================================
void ProgMLRefine3D::reconstructVolumes(const MetaData &mdProj, const FileName &outBase)
{
    FileName               fn_vol, fn_one;
    MetaData               mdOne;

    ProgReconsBase * reconsProgram;// = createReconsProgram();

    for (int volno = 1; volno <= Nvols; ++volno)
    {
        reconsProgram = createReconsProgram();
        //for now each node reconstruct one volume
        if ((volno - 1) % size == rank)
        {
            fn_vol.compose(volno, outBase);
            fn_one.compose(outBase, volno, "projections.xmd");
            // Select only relevant projections to reconstruct
            mdOne.importObjects(mdProj, MDValueEQ(MDL_REF3D, volno));
            mdOne.write(fn_one);
            // Set input/output for the reconstruction algorithm
            //std::cerr << "DEBUG_JM: reconstructing from " << fn_one << " volume " << fn_vol << std::endl;
            reconsProgram->setIO(fn_one, fn_vol);
            reconsProgram->tryRun();
        }
        // Free reconsProgram
        delete reconsProgram;
    }

    // Free reconsProgram
    //delete reconsProgram;
}

void ProgMLRefine3D::calculate3DSSNR(MultidimArray<double> &spectral_signal)
{

    MetaData                    mdNoiseAll, mdNoiseOne;
    MultidimArray<std::complex<double> >  Faux;
    Image<double>              vol, nvol;
    FileName                    fn_tmp, fn_tmp2;
    MultidimArray<double>      alpha_signal, alpha_noise, input_signal, avg_alphaS, avg_alphaN;
    MultidimArray<double>      alpha_T, alpha_N, Msignal, Maux, Mone, mask;
    Projection                  proj;
    int                         c, dim, idum;
    size_t                idumLong;
    double                      ssnr, issnr, alpha, resol, volweight, sum, weight, rot, tilt, psi = 0.;
    Matrix1D<int>               center(2);
    MultidimArray<int>          radial_count;

    // Read in noise reconstruction and calculate alpha's
    mdNoiseAll.read(fn_root + "_noise.xmd");
    ImgSize(mdNoiseAll, dim, idum, idum, idumLong);

    center.initZeros();
    proj().resize(dim, dim);
    proj().setXmippOrigin();
    mask.resize(dim, dim);
    mask.setXmippOrigin();
    RaisedCosineMask(mask, dim / 2 - 2, dim / 2);

    if (verbose)
    {
        std::cout << "--> calculating 3D-SSNR ..." << std::endl;
        init_progress_bar(mdNoiseAll.size());
    }

    for (int volno = 0; volno < Nvols; volno++)
    {

        fn_tmp = fn_root + "_noise";
        fn_tmp2 = fn_root + "_cref";
        if (Nvols > 1)
        {
            fn_tmp += "_vol";
            fn_tmp.compose(fn_tmp, volno + 1, "");
            fn_tmp2 += "_vol";
            fn_tmp2.compose(fn_tmp2, volno + 1, "");
        }
        fn_tmp += ".vol";
        fn_tmp2 += ".vol";
        nvol.read(fn_tmp);
        vol.read(fn_tmp2);
        nvol().setXmippOrigin();
        vol().setXmippOrigin();
        Mone.resize(dim, dim);
        Mone.initConstant(1. / (double)(dim*dim));
        Mone.setXmippOrigin();

        mdNoiseOne.clear();
        mdNoiseOne.importObjects(mdNoiseAll, MDValueEQ(MDL_REF3D, volno + 1));
        c = 0;
        volweight = 0.;
        FOR_ALL_OBJECTS_IN_METADATA(mdNoiseOne)
        {
            mdNoiseOne.getValue(MDL_WEIGHT, weight, __iter.objId);
            mdNoiseOne.getValue(MDL_ANGLEROT, rot, __iter.objId);
            mdNoiseOne.getValue(MDL_ANGLETILT, tilt, __iter.objId);
            // alpha denominator
            if (c == 0)
                alpha_N = Mone * weight;
            else
                alpha_N += Mone * weight;
            // alpha nominator
            projectVolume(nvol(), proj, dim, dim, rot, tilt, psi);
            apply_cont_mask(mask, proj(), proj());
            FourierTransform(proj(), Faux);
            FFT_magnitude(Faux, Maux);
            CenterFFT(Maux, true);
            Maux.setXmippOrigin();
            Maux *= Maux;
            if (c == 0)
                alpha_T = Maux * weight;
            else
                alpha_T += Maux * weight;
            // input signal
            projectVolume(vol(), proj, dim, dim, rot, tilt, psi);
            apply_cont_mask(mask, proj(), proj());
            FourierTransform(proj(), Faux);
            FFT_magnitude(Faux, Maux);
            CenterFFT(Maux, true);
            Maux.setXmippOrigin();
            Maux *= Maux;
            if (c == 0)
                Msignal = Maux * weight;
            else
                Msignal += Maux * weight;
            volweight += weight;
            c++;
            if (c % XMIPP_MAX(1, mdNoiseAll.size() / 60) == 0 && verbose)
                progress_bar(c);
        }

        alpha_T.setXmippOrigin();
        alpha_N.setXmippOrigin();
        Msignal.setXmippOrigin();
        alpha_signal.initZeros();
        alpha_noise.initZeros();
        input_signal.initZeros();
        radialAverage(alpha_T, center, alpha_signal, radial_count, true);
        radialAverage(alpha_N, center, alpha_noise, radial_count, true);
        radialAverage(Msignal, center, input_signal, radial_count, true);
        input_signal /= volweight;

        // Calculate spectral_signal =input_signal/alpha!!
        // Also store averages of alphaN and alphaS for output
        FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY1D(input_signal)
        {
            dAi(input_signal, i) = dAi(input_signal, i) * dAi(alpha_noise, i) / dAi(alpha_signal, i);
        }
        if (volno == 0)
        {
            spectral_signal = input_signal;
            avg_alphaN = alpha_noise;
            avg_alphaS = alpha_signal;
        }
        else
        {
            spectral_signal += input_signal;
            avg_alphaN += alpha_noise;
            avg_alphaS += alpha_signal;
        }
    }
    if (verbose)
        progress_bar(mdNoiseAll.size());
    spectral_signal /= (double)Nvols;
    avg_alphaN /= (double)Nvols;
    avg_alphaS /= (double)Nvols;

    if (verbose)
    {
        fn_tmp = fn_root + "_it";
        fn_tmp.compose(fn_tmp, iter, "3dssnr");
        std::ofstream out(fn_tmp.c_str(), std::ios::out);
        out  << "#        signal    1/alpha    alpha-S    alpha-N" << std::endl;
        FOR_ALL_ELEMENTS_IN_ARRAY1D(spectral_signal)
        {
            if (i > 0 && i < dim / 2)
            {
                out.width(5);
                out  << integerToString(i);
                out.width(10);
                out <<  floatToString(A1D_ELEM(spectral_signal, i));
                out.width(10);
                out <<  floatToString(A1D_ELEM(avg_alphaN, i) / A1D_ELEM(avg_alphaS, i));
                out.width(10);
                out <<  floatToString(A1D_ELEM(avg_alphaS, i));
                out.width(10);
                out <<  floatToString(A1D_ELEM(avg_alphaN, i));
                out << std::endl;
            }
        }
        out.close();
    }

}

void ProgMLRefine3D::copyVolumes()
{
    ImageGeneric img;
    FileName fn_vol, fn_base = FN_INITIAL_BASE;
    size_t volno = 0;

    FOR_ALL_OBJECTS_IN_METADATA(mdVol)
    {
        mdVol.getValue(MDL_IMAGE, fn_vol, __iter.objId);
        img.read(fn_vol);
        fn_vol.compose(++volno, fn_base);
        img.write(fn_vol);
        mdVol.setValue(MDL_IMAGE, fn_vol, __iter.objId);
        mdVol.setValue(MDL_ENABLED, 1, __iter.objId);
    }
}

void ProgMLRefine3D::updateVolumesMetadata()
{
    FileName fn, fn_base = FN_ITER_BASE(iter);

    mdVol.clear();

    for (size_t volno = 1; volno <= Nvols; ++volno)
    {
        fn.compose(volno, fn_base);
        mdVol.setValue(MDL_IMAGE, fn, mdVol.addObject());
        mdVol.setValue(MDL_ENABLED, 1);
    }
}

//
//void ProgRefine3D::remakeSFvol(int iter, bool rewrite, bool include_noise)
//{
//
//    FileName               fn_tmp, fn_tmp2, fn_vol;
//    int                    volno = 0;
//    Image<double>         ref_vol;
//
//    fn_tmp = fn_root + "_it";
//    fn_tmp.compose(fn_tmp, iter, "");
//
//    // Initial iteration: copy volumes to correct name for iteration
//    // loop, and rewrite with this name to disc
//    if (rewrite)
//    {
//        FOR_ALL_OBJECTS_IN_METADATA(SFvol)
//        {
//            SFvol.getValue(MDL_IMAGE, fn_vol, __iter.objId);
//            ref_vol.read(fn_vol);
//            ref_vol().setXmippOrigin();
//            if (Nvols > 1)
//            {
//                fn_tmp2 = fn_tmp + "_vol";
//                fn_tmp2.compose(fn_tmp2, volno + 1, "vol");
//            }
//            else
//                fn_tmp2 = fn_tmp + ".vol";
//            ref_vol.write(fn_tmp2);
//            volno++;
//        }
//    }
//
//    // Update selection file for reference volumes
//    SFvol.clear();
//    size_t id;
//    if (Nvols > 1)
//    {
//        fn_tmp += "_vol";
//        volno = 0;
//        while (volno < Nvols)
//        {
//            fn_tmp2.compose(fn_tmp, volno + 1, "vol");
//            id = SFvol.addObject();
//            SFvol.setValue(MDL_IMAGE, fn_tmp2, id);
//            SFvol.setValue(MDL_ENABLED, 1, id);
//            volno++;
//        }
//    }
//    else
//    {
//        id = SFvol.addObject();
//        SFvol.setValue(MDL_IMAGE, fn_tmp + ".vol", id);
//        SFvol.setValue(MDL_ENABLED, 1, id);
//    }
//    if (include_noise)
//    {
//        fn_tmp = fn_root + "_noise";
//        if (Nvols > 1)
//        {
//            fn_tmp += "_vol";
//            volno = 0;
//            while (volno < Nvols)
//            {
//                fn_tmp2.compose(fn_tmp, volno + 1, "vol");
//                id = SFvol.addObject();
//                SFvol.setValue(MDL_IMAGE, fn_tmp2, id);
//                SFvol.setValue(MDL_ENABLED, 1, id);
//                volno++;
//            }
//        }
//        else
//        {
//            id = SFvol.addObject();
//            SFvol.setValue(MDL_IMAGE, fn_tmp + ".vol", id);
//            SFvol.setValue(MDL_ENABLED, 1, id);
//        }
//        // Besides noise volumes, also include cref volumes
//        fn_tmp = fn_root + "_cref";
//        if (Nvols > 1)
//        {
//            fn_tmp += "_vol";
//            volno = 0;
//            while (volno < Nvols)
//            {
//                fn_tmp2.compose(fn_tmp, volno + 1, "vol");
//                id = SFvol.addObject();
//                SFvol.setValue(MDL_IMAGE, fn_tmp2, id);
//                SFvol.setValue(MDL_ENABLED, 1, id);
//                volno++;
//            }
//        }
//        else
//        {
//            id = SFvol.addObject();
//            SFvol.setValue(MDL_IMAGE, fn_tmp + ".vol", id);
//            SFvol.setValue(MDL_ENABLED, 1, id);
//        }
//    }
//
//}
//
//// Concatenate MLalign2D selfiles ==============================================
//void ProgRefine3D::concatenateSelfiles(int iter)
//{
//#ifdef DEBUG
//    std::cerr << "Entering concatenate_selfiles" <<std::endl;
//#endif
//
//    FileName fn_tmp, fn_class;
//    MetaData MDin, MDout;
//    fn_tmp = fn_root + "_it";
//    fn_tmp.compose(fn_tmp, iter, "");
//    MDin.read(fn_tmp + "_img.xmd");
//
//    int minval, maxval;
//    // Concatenate all hard-classification selfiles
//    for (int volno = 0; volno < Nvols; volno++)
//    {
//        minval = volno * nr_projections + 1;
//        maxval = (volno + 1) * nr_projections;
//        MDout.clear();
//        MDout.importObjects(MDin, MDValueRange(MDL_REF, minval, maxval));
//        fn_class = fn_tmp + "_class_vol";
//        fn_class.compose(fn_class, volno + 1, "");
//        fn_class += "_img.xmd";
//        MDout.write(fn_class);
//    }
//#ifdef DEBUG
//    std::cerr << "Leaving concatenate_selfiles" <<std::endl;
//#endif
//
//}

// Modify reference volume ======================================================
void ProgMLRefine3D::postProcessVolumes()
{

    ProgVolumeSegment            segm_prm;
    FileName               fn_vol, fn_tmp;
    Image<double>          vol, Vaux, Vsymmask, Vsolv;
    MultidimArray<int>     mask3D;
    double                 avg, dummy, in, out;
    int                    dim;
    Sampling               locsampling;

    // Use local sampling because of symmask
    if (!locsampling.SL.isSymmetryGroup(fn_sym, symmetry, sym_order))
        REPORT_ERROR(ERR_NUMERICAL, (String)"ml_refine3d::run Invalid symmetry" +  fn_sym);
    locsampling.SL.read_sym_file(fn_sym);

    if ( !(fn_sym == "c1" || fn_sym == "C1" ) || (lowpass > 0) ||
         (fn_solv != "") || (do_prob_solvent) || (threshold_solvent != 999))
    {

        FOR_ALL_OBJECTS_IN_METADATA(mdVol)
        {
            mdVol.getValue(MDL_IMAGE, fn_vol, __iter.objId);
            // Read corresponding volume from disc
            vol.read(fn_vol);
            vol().setXmippOrigin();
            dim = vol().rowNumber();
            // Store the original volume on disc
            fn_tmp = fn_vol + ".original";
            vol.write(fn_tmp);

            // Symmetrize if requested
            if (!fn_sym.empty())
            {
                Vaux().resize(vol());
                symmetrizeVolume(locsampling.SL, vol(), Vaux());
                // Read local symmetry mask if requested
                if (!fn_symmask.empty())
                {
                    Vsymmask.read(fn_symmask);
                    Vsymmask().setXmippOrigin();
                    if (Vsymmask().computeMax() > 1. || Vsymmask().computeMin() < 0.)
                        REPORT_ERROR(ERR_VALUE_INCORRECT, "ERROR: sym_mask should have values between 0 and 1!");
                    FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY3D(Vsymmask())
                    {
                        in = dAkij(Vsymmask(), k, i, j);
                        out = 1. - in;
                        dAkij(vol(), k, i, j) = out * dAkij(vol(), k, i, j) + in * dAkij(Vaux(), k, i, j);
                    }
                    Vsymmask.clear();
                }
                else
                {
                    vol = Vaux;
                }
                Vaux.clear();
            }

            // Filtering the volume
            if (lowpass > 0)
            {
                FourierFilter fmask;
                fmask.raised_w = 0.02;
                fmask.FilterShape = RAISED_COSINE;
                fmask.FilterBand = LOWPASS;
                fmask.w1 = lowpass;
                fmask.applyMaskSpace(vol());
            }

            // Different types of solvent flattening
            if (do_prob_solvent || !fn_solv.empty() || (threshold_solvent != 999))
            {
                if (do_prob_solvent)
                {
                    // A. Probabilistic solvent flattening
                    // Write already processed volume to disc (for segment program)
                    vol.write(fn_vol);
                    segm_prm.read(argc, argv);
                    segm_prm.fn_vol = fn_vol;
                    segm_prm.fn_mask = fn_vol + ".solv";
                    segm_prm.do_prob = true;
                    segm_prm.show();
                    segm_prm.produce_side_info();
                    segm_prm.segment(Vsolv);
                }
                else if (threshold_solvent != 999)
                {
                    // B. Perform flooding and separate_objects-like solvent mask
                    Vsolv = vol;
                    Vsolv().threshold("below", threshold_solvent, 0.);
                    // The following is because binarize() seems buggy
                    FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY3D(Vsolv())
                    {
                        if (dAkij(Vsolv(), k, i, j) != 0.)
                            dAkij(Vsolv(), k, i, j) = 1.;
                    }
                }
                else if (fn_solv != "")
                {
                    // C. Read user-provided solvent mask from disc
                    Vsolv.read(fn_solv);
                    if (Vsolv().computeMax() > 1. || Vsolv().computeMin() < 0.)
                        REPORT_ERROR(ERR_VALUE_INCORRECT, "ERROR: solvent mask should have values between 0 and 1!");
                }
                // Binarize Vsolv, avoiding buggy Vsolv().binarize()
                if (do_deblob_solvent || dilate_solvent > 0)
                {
                    Vsolv().threshold("below", 0.5, 0.);
                    // The following is because binarize() seems buggy
                    FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY3D(Vsolv())
                    {
                        if (dAkij(Vsolv(), k, i, j) != 0.)
                            dAkij(Vsolv(), k, i, j) = 1.;
                    }
                }
                if (do_deblob_solvent)
                {
                    int object_no, maxo;
                    double nr_vox, max_vox = 0.;
                    Image<double> label;
                    object_no = labelImage3D(Vsolv(), label());
                    max_vox = 0;
                    for (int o = 0; o <= object_no; o++)
                    {
                        Vaux() = label();
                        FOR_ALL_ELEMENTS_IN_ARRAY3D(Vaux())
                        {
                            Vaux(k, i, j) = Vaux(k, i, j) == o;
                        }
                        nr_vox = Vaux().sum();
                        if (o != 0 && (nr_vox > max_vox))
                        {
                            max_vox = nr_vox;
                            maxo = o;
                            Vsolv() = Vaux();
                        }
                    }
                    label.clear();
                }
                // Dilate solvent mask (only for binary masks)
                // Dilate several times, result is summed iteratively
                if (dilate_solvent > 0)
                {
                    Image<double> Vsum;
                    Vsum() = Vsolv();
                    for (int i = 0; i < dilate_solvent; i++)
                    {
                        dilate3D(Vsolv(), Vaux(), 18, 0, 1);
                        Vsum() = Vsum() + Vaux();
                        Vsolv() = Vaux();
                    }
                    Vsum() /= (double)(dilate_solvent + 1);
                    Vsolv() = Vsum();
                    Vsum.clear();
                }
                // Apply solvent mask
                Vsolv() = 1. - Vsolv();
                double solvavg = 0., sumsolv = 0.;
                FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY3D(vol())
                {
                    solvavg += dAkij(Vsolv(), k, i, j) * dAkij(vol(), k, i, j);
                    sumsolv += dAkij(Vsolv(), k, i, j);
                }
                solvavg /= sumsolv;
                FOR_ALL_DIRECT_ELEMENTS_IN_ARRAY3D(Vsolv())
                {
                    dAkij(vol(), k, i, j) -= dAkij(Vsolv(), k, i, j) * (dAkij(vol(), k, i, j) - solvavg);
                }
            }

            // (Re-) write post-processed volume to disc
            vol.write(fn_vol);

        }
        if (verbose)
            std::cout << " -----------------------------------------------------------------" << std::endl;
    }

}

// Convergence check ===============================================================
bool ProgMLRefine3D::checkConvergence()
{

    Image<double>        vol, old_vol, diff_vol;
    FileName               fn_base, fn_base_old, fn_vol;
    Mask            mask_prm;
    MultidimArray<int>    mask3D;
    double                 signal, change;
    int                    dim;
    bool                   converged = true;

    if (iter == 0)
        return false;

    if (verbose)
        std::cout << "--> Checking convergence " << std::endl;

    fn_base = FN_ITER_BASE(iter);
    fn_base_old = FN_ITER_BASE(iter - 1);

    for (size_t volno = 1; volno <= Nvols; ++volno)
    {
        // Read corresponding volume from disc
        fn_vol.compose(volno, fn_base);
        vol.read(fn_vol);
        vol().setXmippOrigin();
        dim = vol().rowNumber();
        old_vol().initZeros(vol());
        diff_vol().initZeros(vol());

        // Only consider voxels within the spherical mask
        mask_prm.R1 = dim / 2;
        mask_prm.type = BINARY_CIRCULAR_MASK;
        mask_prm.mode = INNER_MASK;
        mask_prm.generate_mask(vol());
        fn_vol.compose(volno, fn_base_old);
        old_vol.read(fn_vol);
        diff_vol() = vol() - old_vol();
        mask_prm.apply_mask(old_vol(), old_vol());
        mask_prm.apply_mask(diff_vol(), diff_vol());
        change = diff_vol().sum2();
        signal = old_vol().sum2();
        if (change / signal > eps)
            converged = false;
        if (verbose)
            std::cout << formatString("--> Relative signal change volume %d = %f", volno, change / signal) << std::endl;
    }

    return converged;

}
