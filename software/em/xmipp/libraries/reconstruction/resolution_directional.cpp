/***************************************************************************
 *
 * Authors:    Jose Luis Vilas, 					  jlvilas@cnb.csic.es
 * 			   Carlos Oscar S. Sorzano            coss@cnb.csic.es (2016)
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

#include "resolution_directional.h"
//#define DEBUG
//#define DEBUG_MASK
#define DEBUG_DIR
//define DEBUG_FILTER
//define MONO_AMPLITUDE
//define DEBUG_SYMMETRY

void ProgResDir::readParams()
{
	fnVol = getParam("--vol");
	fnVol2 = getParam("--vol2");
	fnMeanVol = getParam("--meanVol");
	fnOut = getParam("-o");
	fnMask = getParam("--mask");
	fnMaskOut = getParam("--mask_out");
	fnchim = getParam("--chimera_volume");
	sampling = getDoubleParam("--sampling_rate");
	ang_sampling = getDoubleParam("--angular_sampling");
	R = getDoubleParam("--volumeRadius");
	minRes = getDoubleParam("--minRes");
	maxRes = getDoubleParam("--maxRes");
	fnMaxVol = getParam("--maxVol");
	fnMinVol = getParam("--minVol");
	fnVar = getParam("--varVol");
	fnSym = getParam("--sym");
	N_freq = getDoubleParam("--number_frequencies");
	noiseOnlyInHalves = checkParam("--noiseonlyinhalves");
	significance = getDoubleParam("--significance");
	fnMd = getParam("--md_resdir");
	fnDoA = getParam("--doa_vol");
}


void ProgResDir::defineParams()
{
	addUsageLine("This function determines the local resolution of a map");
	addParamsLine("  --vol <vol_file=\"\">        : Input volume");
	addParamsLine("  [--mask <vol_file=\"\">]     : Mask defining the macromolecule");
	addParamsLine("                               :+ If two half volume are given, the noise is estimated from them");
	addParamsLine("                               :+ Otherwise the noise is estimated outside the mask");
	addParamsLine("  [--mask_out <vol_file=\"\">] : sometimes the provided mask is not perfect, and contains voxels out of the particle");
	addParamsLine("                               :+ Thus the algorithm calculated a tight mask to the volume");
	addParamsLine("  [--vol2 <vol_file=\"\">]     : Half volume 2");
	addParamsLine("  [-o <output=\"MGresolution.vol\">]: Local resolution volume (in Angstroms)");
	addParamsLine("  [--meanVol <vol_file=\"\">]  : Mean volume of half1 and half2 (only it is neccesary the two haves are used)");
	addParamsLine("  --sym <symmetry>: Symmetry (c1, c2, c3,..d1, d2, d3,...)");
	addParamsLine("  [--chimera_volume <output=\"Chimera_resolution_volume.vol\">]: Local resolution volume for chimera viewer (in Angstroms)");
	addParamsLine("  [--sampling_rate <s=1>]      : Sampling rate (A/px)");
	addParamsLine("  [--angular_sampling <s=15>]  : Angular Sampling rate (degrees)");
	addParamsLine("  [--volumeRadius <s=100>]     : This parameter determines the radius of a sphere where the volume is");
	addParamsLine("  [--number_frequencies <w=50>]       : The resolution is computed at a number of frequencies between mininum and");
	addParamsLine("                               : maximum resolution px/A. This parameter determines that number");
	addParamsLine("  [--minRes <s=30>]            : Minimum resolution (A)");
	addParamsLine("  [--maxRes <s=1>]             : Maximum resolution (A)");
	addParamsLine("  [--maxVol <vol_file=\"\">]   : Output filename with maximum resolution volume");
	addParamsLine("  [--minVol <vol_file=\"\">]   : Output filename with minimum resolution volume");
	addParamsLine("  [--varVol <vol_file=\"\">]   : Output filename with varianze resolution volume");
	addParamsLine("  [--noiseonlyinhalves]        : The noise estimation is only performed inside the mask");
	addParamsLine("  [--significance <s=0.95>]    : The level of confidence for the hypothesis test.");
	addParamsLine("  [--md_resdir <file=\".\">]   : Metadata with mean resolution by direction.");
	addParamsLine("  [--doa_vol <vol_file=\"\">]  : Output filename with DoA volume");
}

void ProgResDir::produceSideInfo()
{
	std::cout << "Starting..." << std::endl;
	Image<double> V;
	if ((fnVol !="") && (fnVol2 !=""))
	{
		Image<double> V1, V2;
		V1.read(fnVol);
		V2.read(fnVol2);
		V()=0.5*(V1()+V2());
		V.write(fnMeanVol);
		halfMapsGiven = true;
	}
	else{
	    V.read(fnVol);
	    halfMapsGiven = false;
	}
	V().setXmippOrigin();

	//Sweeping the projection sphere
	std::cout << "Obtaining angular projections..." << std::endl;
	generateGridProjectionMatching(fnVol, ang_sampling, angles);

	FourierTransformer transformer;
	MultidimArray<double> &inputVol = V();
	VRiesz.resizeNoCopy(inputVol);

	transformer.FourierTransform(inputVol, fftV);
	iu.initZeros(fftV);

	// Frequency volume
	double uz, uy, ux, uz2, u2, uz2y2;
	long n=0;
//	for(size_t k=0; k<ZSIZE(fftV); ++k)
//	{
//		FFT_IDX2DIGFREQ(k,ZSIZE(inputVol),uz);
//		uz2=uz*uz;
//		for(size_t i=0; i<YSIZE(fftV); ++i)
//		{
//			FFT_IDX2DIGFREQ(i,YSIZE(inputVol),uy);
//			uz2y2=uz2+uy*uy;
//			for(size_t j=0; j<XSIZE(fftV); ++j)
//			{
//				FFT_IDX2DIGFREQ(j,XSIZE(inputVol),ux);
//				u2=uz2y2+ux*ux;
////				if ((fabs(uz) <= 0.1) || (fabs(uy) <= 0.1) )//|| (fabs(uz) <= 0.1))
////					DIRECT_MULTIDIM_ELEM(iu,n) = ux;
//				if ((k != 0) || (i != 0) || (j != 0))
//					DIRECT_MULTIDIM_ELEM(iu,n) = 1.0/sqrt(u2);
//				else
//					DIRECT_MULTIDIM_ELEM(iu,n) = 1e38;
//				++n;
//			}
//		}
//	}
	for(size_t k=0; k<ZSIZE(fftV); ++k)
	{
		FFT_IDX2DIGFREQ(k,ZSIZE(inputVol),uz);
		uz2=uz*uz;

		for(size_t i=0; i<YSIZE(fftV); ++i)
		{
			FFT_IDX2DIGFREQ(i,YSIZE(inputVol),uy);
			uz2y2=uz2+uy*uy;

			for(size_t j=0; j<XSIZE(fftV); ++j)
			{
				FFT_IDX2DIGFREQ(j,XSIZE(inputVol),ux);
				u2=uz2y2+ux*ux;
				if ((k != 0) || (i != 0) || (j != 0))
					DIRECT_MULTIDIM_ELEM(iu,n) = 1.0/sqrt(u2);
				else
					DIRECT_MULTIDIM_ELEM(iu,n) = 1e38;
				++n;
			}
		}
	}



	#ifdef DEBUG_DIR
	Image<double> freqimg;
	freqimg = iu;
	freqimg.write(formatString("frequencys.vol"));
	#endif



	// Prepare low pass filter
	lowPassFilter.FilterShape = RAISED_COSINE;
	lowPassFilter.raised_w = 0.01;
	lowPassFilter.do_generate_3dmask = false;
	lowPassFilter.FilterBand = LOWPASS;

	// Prepare mask
	MultidimArray<int> &pMask=mask();

	if (fnMask != "")
	{
		mask.read(fnMask);
		mask().setXmippOrigin();
	}
	else
	{
		std::cout << "Error: a mask ought to be provided" << std::endl;
		exit(0);
	}

	//use the mask for preparing resolution volumes
	Image<double> MaxResolution, MinResolution, AvgResoltion, VarianzeResolution;
	MaxResolution().resizeNoCopy(inputVol);
	MinResolution().resizeNoCopy(inputVol);
	AvgResoltion().resizeNoCopy(inputVol);
	VarianzeResolution().resizeNoCopy(inputVol);
	MinResolution().initConstant(maxRes);
	MaxResolution().initConstant(1);
	AvgResoltion().initZeros();
	VarianzeResolution().initZeros();

	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(MaxResolution())
		DIRECT_MULTIDIM_ELEM(MaxResolution(),n)*=DIRECT_MULTIDIM_ELEM(pMask,n);

	MaxResolution.write(fnMaxVol);
	MinResolution.write(fnMinVol);
	AvgResoltion.write(fnOut);
	VarianzeResolution.write(fnVar);

	MaxResolution.clear();
	MinResolution.clear();
	AvgResoltion.clear();
	VarianzeResolution.clear();

	NVoxelsOriginalMask = 0;
	FOR_ALL_ELEMENTS_IN_ARRAY3D(pMask)
	{
		if (A3D_ELEM(pMask, k, i, j) == 1)
			NVoxelsOriginalMask++;
		if (i*i+j*j+k*k > R*R)
			A3D_ELEM(pMask, k, i, j) = -1;
	}

	#ifdef DEBUG_MASK
	mask.write("mask.vol");
	#endif

	if (halfMapsGiven)
	{
		Image<double> V1, V2;
		V1.read(fnVol);
		V2.read(fnVol2);

		V1()-=V2();
		V1()/=2;
		fftN=new MultidimArray< std::complex<double> >;
		FourierTransformer transformer2;
		#ifdef DEBUG
		  V1.write("diff_volume.vol");
		#endif
		transformer2.FourierTransform(V1(), *fftN);
	}
	else
	{
		fftN=&fftV;
	}
	V.clear();
}


void ProgResDir::generateGridProjectionMatching(FileName fnVol_, double smprt,
												Matrix2D<double> &angles)
{
	FileName fnGallery;
	FileName fnanglesmd = "angles.doc";

	// Generate projections
	fnGallery=formatString("angles.stk");

	String args=formatString("-i %s -o %s --sampling_rate %f",
			fnVol_.c_str(), fnGallery.c_str(), smprt); //fnGallery.c_str(),smprt);

	String cmd=(String)"xmipp_angular_project_library " + args;
	std::cout << cmd << std::endl;
	system(cmd.c_str());

	MetaData md;
	md.read(fnanglesmd);

	size_t md_size = md.size();
	//				if ((fabs(uz) <= 0.1) || (fabs(uy) <= 0.1) )//|| (fabs(uz) <= 0.1))
	//					DIRECT_MULTIDIM_ELEM(iu,n) = ux;
	Matrix2D<double> aux_angles(2,md_size);
	size_t count = 0;
	double rot, tilt;
	FOR_ALL_OBJECTS_IN_METADATA(md)
	{
		md.getValue(MDL_ANGLE_ROT, rot, __iter.objId);
		md.getValue(MDL_ANGLE_TILT, tilt, __iter.objId);
		if ( (rot>=0) && (tilt>=0) )
		{
			MAT_ELEM(aux_angles,0, count) = rot;
			if (tilt>90)
				MAT_ELEM(aux_angles,1, count) = tilt-180;
			else
				MAT_ELEM(aux_angles,1, count) = tilt;
			count++;
		}
	}

	N_directions = count-1;
	angles.initZeros(4,N_directions);

	for (size_t k = 0; k<N_directions; k++)
	{
		MAT_ELEM(angles, 0, k) = MAT_ELEM(aux_angles,0, k);
		MAT_ELEM(angles, 1, k) = MAT_ELEM(aux_angles,1, k);
		std::cout << "k=" << k << "  rot = " << MAT_ELEM(angles, 0, k) << "  tilt = " << MAT_ELEM(angles, 1, k) << std::endl;
	}
}


void ProgResDir::amplitudeMonogenicSignal3D(MultidimArray< std::complex<double> > &myfftV,
		double w1, double w1h, double w1l, MultidimArray<double> &amplitude, int count, FileName fnDebug,
		double angle_cone, double rot, double tilt)
{
	fftVRiesz.initZeros(myfftV);
	fftVRiesz_aux.initZeros(myfftV);
	amplitude.resizeNoCopy(VRiesz);
	std::complex<double> J(0,1);

	// Filter the input volume and add it to amplitude
	long n=0;
	double iw=1.0/w1;
	double iwl=1.0/w1l;
	double ideltal=PI/(w1-w1l);

	#ifdef DEBUG_DIR
	MultidimArray<double> coneVol;
	coneVol.initZeros(iu);
	#endif

	//	if (tilt<0)
//	{
//		std::cout << "tilt < 0" << std::endl;
//		tilt= tilt + 180;
//	}

	double tilt_cone_plus = (tilt + 0.5*angle_cone)*PI/180;
	double tilt_cone_minus = (tilt - 0.5*angle_cone)*PI/180;
	double rot_cone_plus = (rot + 0.5*angle_cone)*PI/180;
	double rot_cone_minus = (rot - 0.5*angle_cone)*PI/180;

	std::cout << "------------------------" << std::endl;
	std::cout << "tilt_cone_plus = " << tilt_cone_plus << std::endl;
	std::cout << "tilt_cone_minus = " << tilt_cone_minus << std::endl;
	std::cout << "rot_cone_plus = " << rot_cone_plus << std::endl;
	std::cout << "rot_cone_minus = " << rot_cone_minus << std::endl;
	std::cout << "------------------------" << std::endl;

	bool tilt_extreme=false, rot_extreme = false;

	if (tilt_cone_plus>=PI/2)
	{
		tilt_cone_plus = tilt_cone_plus - PI;
		tilt_extreme = true;
	}
	if (tilt_cone_minus<=-PI/2)
	{
		tilt_cone_minus = tilt_cone_minus + PI;
		tilt_extreme = true;
	}
	if (rot_cone_plus>=PI)
	{
		rot_cone_plus = rot_cone_plus - PI;
		rot_extreme = true;
	}
	if (rot_cone_minus<=0)
	{
		rot_cone_minus = rot_cone_minus + PI;
		rot_extreme = true;
	}

	std::cout << "tilt_cone_minus= " << tilt_cone_minus*180/PI << "   tilt_cone_plus= "
			<< tilt_cone_plus*180/PI << "   rot_cone_minus="<< rot_cone_minus*180/PI <<
			"   rot_cone_plus=" << rot_cone_plus*180/PI << std::endl;

	double x_dir, y_dir, z_dir;

	x_dir = sin(tilt*PI/180)*cos(rot*PI/180);
	y_dir = sin(tilt*PI/180)*sin(rot*PI/180);
	z_dir = cos(tilt*PI/180);

	double cos_cone = cos(0.5*angle_cone*PI/180);
	double uz, uy, ux, uxxuyy;
	n=0;
	for(size_t k=0; k<ZSIZE(myfftV); ++k)
	{
		FFT_IDX2DIGFREQ(k,ZSIZE(amplitude),uz);
		if (uz ==0)
			uz = 1e-38;
		for(size_t i=0; i<YSIZE(myfftV); ++i)
		{
			FFT_IDX2DIGFREQ(i,YSIZE(amplitude),uy);
			if (uy ==0)
				uy = 1e-38;
			for(size_t j=0; j<XSIZE(myfftV); ++j)
			{
				FFT_IDX2DIGFREQ(j,XSIZE(amplitude),ux);
				if (ux ==0)
					ux = 1e-38;

				double iun=DIRECT_MULTIDIM_ELEM(iu,n);
				double tilt_freq = atan(sqrt(ux*ux + uy*uy)/uz);
				double rot_freq = acos(uy/sqrt(ux*ux + uy*uy));

				//directional vector
				double xx, yy, zz;
				xx = sin(tilt_freq)*cos(rot_freq);
				yy = sin(tilt_freq)*sin(rot_freq);
				zz = cos(tilt_freq);

				double dotproduct = fabs(xx*x_dir + yy*y_dir + zz*z_dir);

				if (dotproduct>=cos_cone)
					{
					double un=1.0/iun;
					DIRECT_MULTIDIM_ELEM(coneVol, n) = 1;
						if (w1l<=un && un<=w1)
						{
							DIRECT_MULTIDIM_ELEM(fftVRiesz, n) = DIRECT_MULTIDIM_ELEM(myfftV, n);
							DIRECT_MULTIDIM_ELEM(fftVRiesz, n) *= 0.5*(1+cos((un-w1)*ideltal));//H;
						} else if (un>w1)
						{
							DIRECT_MULTIDIM_ELEM(fftVRiesz, n) = DIRECT_MULTIDIM_ELEM(myfftV, n);
						}
					}
					DIRECT_MULTIDIM_ELEM(fftVRiesz_aux, n) = -J*iun*DIRECT_MULTIDIM_ELEM(fftVRiesz, n);
				++n;
			}
		}
	}

	#ifdef DEBUG_DIR
	if (count == 0)
	{
		Image<double> direction;
		direction = coneVol;
		direction.write(formatString("cone_%i.vol", count));
	}
	#endif

	transformer_inv.inverseFourierTransform(fftVRiesz, VRiesz);

	#ifdef DEBUG_DIR
	if (count == 0)
	{
		std::cout << "saving filtered volume" << std::endl;
		Image<double> filteredvolume;
		filteredvolume = VRiesz;
		filteredvolume.write(formatString("Volumen_filtrado_%i.vol", count));
	}
	#endif

	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitude)
		DIRECT_MULTIDIM_ELEM(amplitude,n)=DIRECT_MULTIDIM_ELEM(VRiesz,n)*DIRECT_MULTIDIM_ELEM(VRiesz,n);

	// Calculate first component of Riesz vector
	fftVRiesz.initZeros(myfftV);
	n=0;

	for(size_t k=0; k<ZSIZE(myfftV); ++k)
	{
		for(size_t i=0; i<YSIZE(myfftV); ++i)
		{
			for(size_t j=0; j<XSIZE(myfftV); ++j)
			{
				FFT_IDX2DIGFREQ(j,XSIZE(amplitude),ux);
				DIRECT_MULTIDIM_ELEM(fftVRiesz, n) *= ux*DIRECT_MULTIDIM_ELEM(fftVRiesz_aux, n);
				++n;
			}
		}
	}
	transformer_inv.inverseFourierTransform(fftVRiesz, VRiesz);
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitude)
	DIRECT_MULTIDIM_ELEM(amplitude,n)+=DIRECT_MULTIDIM_ELEM(VRiesz,n)*DIRECT_MULTIDIM_ELEM(VRiesz,n);

	// Calculate second component of Riesz vector
	fftVRiesz.initZeros(myfftV);
	n=0;

	for(size_t k=0; k<ZSIZE(myfftV); ++k)
	{
		for(size_t i=0; i<YSIZE(myfftV); ++i)
		{
			FFT_IDX2DIGFREQ(i,YSIZE(amplitude),uy);
			for(size_t j=0; j<XSIZE(myfftV); ++j)
			{
			DIRECT_MULTIDIM_ELEM(fftVRiesz, n) *= uy*DIRECT_MULTIDIM_ELEM(fftVRiesz_aux, n);
			++n;
			}
		}
	}
	transformer_inv.inverseFourierTransform(fftVRiesz, VRiesz);
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitude)
	DIRECT_MULTIDIM_ELEM(amplitude,n)+=DIRECT_MULTIDIM_ELEM(VRiesz,n)*DIRECT_MULTIDIM_ELEM(VRiesz,n);

	// Calculate third component of Riesz vector
	fftVRiesz.initZeros(myfftV);
	n=0;
	for(size_t k=0; k<ZSIZE(myfftV); ++k)
	{
		FFT_IDX2DIGFREQ(k,ZSIZE(amplitude),uz);
		for(size_t i=0; i<YSIZE(myfftV); ++i)
		{
			for(size_t j=0; j<XSIZE(myfftV); ++j)
			{
				DIRECT_MULTIDIM_ELEM(fftVRiesz, n) *= uz*DIRECT_MULTIDIM_ELEM(fftVRiesz_aux, n);
				++n;
			}
		}
	}
	transformer_inv.inverseFourierTransform(fftVRiesz, VRiesz);
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitude)
	{
		DIRECT_MULTIDIM_ELEM(amplitude,n)+=DIRECT_MULTIDIM_ELEM(VRiesz,n)*DIRECT_MULTIDIM_ELEM(VRiesz,n);
		DIRECT_MULTIDIM_ELEM(amplitude,n)=sqrt(DIRECT_MULTIDIM_ELEM(amplitude,n));
	}
//	#ifdef MONO_AMPLITUDE
//	if (fnDebug.c_str() != "")
//	{
//	Image<double> saveImg;
//	saveImg = amplitude;
//	iternumber = formatString("_Amplitude_%i.vol", count);
//	saveImg.write(fnDebug+iternumber);
//	saveImg.clear();
//	}
//	#endif // DEBUG
	// Low pass filter the monogenic amplitude
	lowPassFilter.w1 = w1;
	amplitude.setXmippOrigin();
	lowPassFilter.applyMaskSpace(amplitude);

	#ifdef MONO_AMPLITUDE
	saveImg2 = amplitude;
	FileName fnSaveImg2;
	if (fnDebug.c_str() != "")
	{
		iternumber = formatString("_Filtered_Amplitude_%i.vol", count);
		saveImg2.write(fnDebug+iternumber);
	}
	saveImg2.clear(); 
	#endif // DEBUG

}


void ProgResDir::postProcessingLocalResolutions(MultidimArray<double> &resolutionVol,
		std::vector<double> &list, MultidimArray<double> &resolutionChimera, double &cut_value, MultidimArray<int> &pMask)
{
	MultidimArray<double> resolutionVol_aux = resolutionVol;
	double last_resolution_2 = sampling/list[(list.size()-1)];

	// Count number of voxels with resolution
	size_t N=0;
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(resolutionVol)
		if (DIRECT_MULTIDIM_ELEM(resolutionVol, n)>(last_resolution_2-0.001)) //the value 0.001 is a tolerance
			++N;

	// Get all resolution values
	MultidimArray<double> resolutions(N);
	size_t N_iter=0;
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(resolutionVol)
		if (DIRECT_MULTIDIM_ELEM(resolutionVol, n)>(last_resolution_2-0.001))
			DIRECT_MULTIDIM_ELEM(resolutions,N_iter++)=DIRECT_MULTIDIM_ELEM(resolutionVol, n);
	// Sort value and get threshold
	std::sort(&A1D_ELEM(resolutions,0),&A1D_ELEM(resolutions,N));
	double filling_value = A1D_ELEM(resolutions, (int)(0.5*N)); //median value
	double trimming_value = A1D_ELEM(resolutions, (int)((1-cut_value)*N));

	double freq, res, init_res, last_res;

	init_res = sampling/list[0];
	last_res = sampling/list[(list.size()-1)];
	
	std::cout << "----------------------------------------------------------------------------------" << std::endl;

	resolutionChimera = resolutionVol;

	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(resolutionVol)
	{
		if (DIRECT_MULTIDIM_ELEM(resolutionVol, n) < last_res)
		{
			if (DIRECT_MULTIDIM_ELEM(pMask, n) >=1)
			{
				DIRECT_MULTIDIM_ELEM(resolutionVol, n) = filling_value;
			}
			else
			{
				DIRECT_MULTIDIM_ELEM(resolutionVol, n) = 0;
				DIRECT_MULTIDIM_ELEM(pMask,n) = 0;
			}

		}
		if (DIRECT_MULTIDIM_ELEM(resolutionVol, n) > trimming_value)
		{
		  DIRECT_MULTIDIM_ELEM(pMask,n) = 0;
		  DIRECT_MULTIDIM_ELEM(resolutionVol, n) = filling_value;
		}
	}
	//#ifdef DEBUG_MASK
//	Image<int> imgMask;
//	imgMask = pMask;
//	imgMask.write(fnMaskOut);
	//#endif
}


void ProgResDir::run()
{
	produceSideInfo();

	MetaData md;

	double criticalZ=icdf_gauss(significance);

	double range = maxRes-minRes;
	double R_ = range/N_freq;

	if (R_<0.1)
		R_=0.1;

		std::cout << "Analyzing directions" << std::endl;

	for (size_t dir=0; dir<N_directions; dir++)
	{
		Image<double> outputResolution;
		outputResolution().resizeNoCopy(VRiesz);
		outputResolution().initZeros();
		MultidimArray<double> &pOutputResolution = outputResolution();
		MultidimArray<double> amplitudeMS, amplitudeMN;
		MultidimArray<int> mask_aux = mask();
		MultidimArray<int> &pMask = mask_aux;
		std::vector<double> list;
		double resolution, last_resolution = 10000;  //A huge value for achieving last_resolution < resolution
		double freq, freqH, freqL, resVal, counter;
		double max_meanS = -1e38;
		double cut_value = 0.025;

		bool doNextIteration=true;
		bool lefttrimming = false;

		int iter = 0;
		int count_res = 0;
		double criticalW=-1;
		double angle_cone = ang_sampling;
		double rot = MAT_ELEM(angles, 0, dir);
		double tilt = MAT_ELEM(angles, 1, dir);
		std::cout << "Analyzing frequencies in direction = " << dir << "   rot = " << rot << "   tilt = " << tilt << std::endl;

		FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(pOutputResolution)
			if (DIRECT_MULTIDIM_ELEM(pMask, n) == 1)
				DIRECT_MULTIDIM_ELEM(pOutputResolution, n) = maxRes;

		std::vector<double> noiseValues;
		FileName fnDebug;
		do
		{
			resolution = maxRes - count_res*R_;
			freqL = sampling/(resolution+R_);
			freq = sampling/resolution;
			freqH = sampling/(resolution-R_);
			++count_res;



			//std::cout << "resolution =  " << resolution << std::endl;
			if (freq > 0.5)
			{
			  std::cout << "search stopped due to Nyquist limit has been reached" << std::endl;
			  break;
			}

			if (count_res==1)
				counter = 0; //maxRes/R_;
			else
				if (count_res==2)
					counter = 1;
				else
					counter = 2;//count_res-2;

			std::cout << "Iteration " << iter << " Freq = " << freq << " Resolution = " << resolution << " (A)" << std::endl;

			fnDebug = "Signal";

			amplitudeMonogenicSignal3D(fftV, freq, freqH, freqL, amplitudeMS, iter, fnDebug, angle_cone, rot, tilt);
			if (halfMapsGiven)
			{
				fnDebug = "Noise";
				amplitudeMonogenicSignal3D(*fftN, freq, freqH, freqL, amplitudeMN, iter, fnDebug, angle_cone, rot, tilt);
			}

			list.push_back(freq);

			double sumS=0, sumS2=0, sumN=0, sumN2=0, NN = 0, NS = 0;
			noiseValues.clear();

			if (halfMapsGiven)
			{
				if (noiseOnlyInHalves)
				{
					FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
					{
						double amplitudeValue=DIRECT_MULTIDIM_ELEM(amplitudeMS, n);
						double amplitudeValueN=DIRECT_MULTIDIM_ELEM(amplitudeMN, n);
						if (DIRECT_MULTIDIM_ELEM(pMask, n)>=1)
						{
							sumS  += amplitudeValue;
							sumS2 += amplitudeValue*amplitudeValue;
							++NS;
							sumN  += amplitudeValueN;
							sumN2 += amplitudeValueN*amplitudeValueN;
							++NN;
						}
					}
				}
				else
				{
					FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
					{
						double amplitudeValue=DIRECT_MULTIDIM_ELEM(amplitudeMS, n);
						double amplitudeValueN=DIRECT_MULTIDIM_ELEM(amplitudeMN, n);
						if (DIRECT_MULTIDIM_ELEM(pMask, n)>=1)
						{
							sumS  += amplitudeValue;
							sumS2 += amplitudeValue*amplitudeValue;
							++NS;
						}
						if (DIRECT_MULTIDIM_ELEM(pMask, n)>=0)
						{
							sumN  += amplitudeValueN;
							sumN2 += amplitudeValueN*amplitudeValueN;
							++NN;
						}
					}
				}
			}
			else
			{
				FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
				{
					double amplitudeValue=DIRECT_MULTIDIM_ELEM(amplitudeMS, n);
					if (DIRECT_MULTIDIM_ELEM(pMask, n)>=1)
					{
						sumS  += amplitudeValue;
						sumS2 += amplitudeValue*amplitudeValue;
						++NS;
					}
					else if (DIRECT_MULTIDIM_ELEM(pMask, n)==0)
					{
						sumN  += amplitudeValue;
						sumN2 += amplitudeValue*amplitudeValue;
						++NN;
					}
				}
			}


			if ( (NS/NVoxelsOriginalMask)<cut_value ) //when the 2.5% is reached then the iterative process stops
			{
				std::cout << "Search of resolutions stopped due to mask has been completed" << std::endl;
				doNextIteration =false;
				Nvoxels = 0;
				FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
				{
				  if (DIRECT_MULTIDIM_ELEM(pOutputResolution, n) == 0)
					DIRECT_MULTIDIM_ELEM(pMask, n) = 0;
				  else
				  {
					Nvoxels++;
					DIRECT_MULTIDIM_ELEM(pMask, n) = 1;
				  }
				}
			#ifdef DEBUG_MASK
			mask.write("partial_mask.vol");
			#endif
			lefttrimming = true;
			}
			else
			{
			if (NS == 0)
			{
				std::cout << "There are no points to compute inside the mask" << std::endl;
				std::cout << "If the number of computed frequencies is low, perhaps the provided"
						"mask is not enough tight to the volume, in that case please try another mask" << std::endl;
				break;
			}

			double meanS=sumS/NS;
			double sigma2S=sumS2/NS-meanS*meanS;
			double meanN=sumN/NN;
			double sigma2N=sumN2/NN-meanN*meanN;

			if (meanS>max_meanS)
				max_meanS = meanS;

			if (meanS<0.001*max_meanS)
			{
				std::cout << "Search of resolutions stopped due to too low signal" << std::endl;
				doNextIteration = false;
			}
			else
			{
				// Check local resolution
				double thresholdNoise;
				thresholdNoise = meanN+criticalZ*sqrt(sigma2N);

				#ifdef DEBUG
				  std::cout << "Iteration = " << iter << ",   Resolution= " << resolution << ",   Signal = " << meanS << ",   Noise = " << meanN << ",  Threshold = " << thresholdNoise <<std::endl;
				#endif

				FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
				{
					if (DIRECT_MULTIDIM_ELEM(pMask, n)>=1)
						if (DIRECT_MULTIDIM_ELEM(amplitudeMS, n)>thresholdNoise)
						{
							DIRECT_MULTIDIM_ELEM(pMask, n) = 1;
							DIRECT_MULTIDIM_ELEM(pOutputResolution, n) = resolution;//sampling/freq;
						}
						else
						{
							DIRECT_MULTIDIM_ELEM(pMask, n) += 1;
							if (DIRECT_MULTIDIM_ELEM(pMask, n) >2)
							{
								DIRECT_MULTIDIM_ELEM(pMask, n) = -1;
								DIRECT_MULTIDIM_ELEM(pOutputResolution, n) = resolution + counter*R_;//maxRes - counter*R_;
							}
						}
				}

				#ifdef DEBUG_MASK
				FileName fnmask_debug;
				fnmask_debug = formatString("maske_%i.vol", iter);
				mask.write(fnmask_debug);
				#endif

				// Is the mean inside the signal significantly different from the noise?
				double z=(meanS-meanN)/sqrt(sigma2S/NS+sigma2N/NN);
				#ifdef DEBUG
					std::cout << "thresholdNoise = " << thresholdNoise << std::endl;
					std::cout << "  meanS= " << meanS << " sigma2S= " << sigma2S << " NS= " << NS << std::endl;
					std::cout << "  meanN= " << meanN << " sigma2N= " << sigma2N << " NN= " << NN << std::endl;
					std::cout << "  z= " << z << " (" << criticalZ << ")" << std::endl;
				#endif
				if (z<criticalZ)
				{
					criticalW = freq;
					std::cout << "Search stopped due to z>Z (hypothesis test)" << std::endl;
					doNextIteration=false;
				}
				if (doNextIteration)
				{
					if (resolution <= (minRes-0.001))
						doNextIteration = false;
				}
				}
			}
			iter++;
		}while(doNextIteration);

		if (lefttrimming == false)
		{
		  Nvoxels = 0;
		  FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(amplitudeMS)
		  {
		    if (DIRECT_MULTIDIM_ELEM(pOutputResolution, n) == 0)
		    {
		      DIRECT_MULTIDIM_ELEM(pMask, n) = 0;
		    }
		    else
		    {
		      Nvoxels++;
		      DIRECT_MULTIDIM_ELEM(pMask, n) = 1;
		    }
		  }
		#ifdef DEBUG_MASK
		  //mask.write(fnMaskOut);
		#endif
		}
		amplitudeMN.clear();
		amplitudeMS.clear();
		fftVRiesz.clear();
		

		double last_resolution_2 = resolution;
		if (fnSym!="c1")
		{
			SymList SL;
			SL.readSymmetryFile(fnSym);
			MultidimArray<double> VSimetrized;
			symmetrizeVolume(SL, pOutputResolution, VSimetrized, LINEAR, DONT_WRAP);
			outputResolution() = VSimetrized;
			VSimetrized.clear();
		}

		#ifdef DEBUG_SYMMETRY
			outputResolution.write("resolution_simple_simmetrized.vol");

		Image<double> saveImg;
		saveImg = pOutputResolution;
		FileName fnres;
		fnres = formatString("resolution_simmetrized_%i.vol", dir);
		saveImg.write(fnres);
		saveImg.clear();
		#endif

		MultidimArray<double> resolutionFiltered, resolutionChimera;
		postProcessingLocalResolutions(pOutputResolution, list, resolutionChimera, cut_value, pMask);


		Image<double> VarianzeResolution, MaxResolution, MinResolution, AvgResolution;

		FileName fn_dires;
		fn_dires = formatString("resolution_direcion_%i.vol", dir);
		//dirResVol.write(fn_dires);

		//They were initialized in producesideinfo()
		MaxResolution.read(fnMaxVol);
		MinResolution.read(fnMinVol);
		AvgResolution.read(fnOut);
		VarianzeResolution.read(fnVar);

		MultidimArray<double> &pMaxResolution = MaxResolution();
		MultidimArray<double> &pMinResolution = MinResolution();
		MultidimArray<double> &pAvgResolution = AvgResolution();
		MultidimArray<double> &pVarianzeResolution = VarianzeResolution();

		// Calculating min and max resolution volumes
		FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(pOutputResolution)
		{
			if (DIRECT_MULTIDIM_ELEM(mask(), n) == 1)
			{
			double Resvoxel = DIRECT_MULTIDIM_ELEM(pOutputResolution, n);
			DIRECT_MULTIDIM_ELEM(pAvgResolution, n) += Resvoxel;
			DIRECT_MULTIDIM_ELEM(pVarianzeResolution, n) += Resvoxel*Resvoxel;
			if (DIRECT_MULTIDIM_ELEM(pOutputResolution, n)<DIRECT_MULTIDIM_ELEM(pMinResolution, n))
				DIRECT_MULTIDIM_ELEM(pMinResolution, n) = DIRECT_MULTIDIM_ELEM(pOutputResolution, n);
			if (DIRECT_MULTIDIM_ELEM(pOutputResolution, n)>DIRECT_MULTIDIM_ELEM(pMaxResolution, n))
				DIRECT_MULTIDIM_ELEM(pMaxResolution, n) = DIRECT_MULTIDIM_ELEM(pOutputResolution, n);
			}
			else
			{
				DIRECT_MULTIDIM_ELEM(pMinResolution, n) = 0;
				DIRECT_MULTIDIM_ELEM(pMaxResolution, n) = 0;
				DIRECT_MULTIDIM_ELEM(pAvgResolution, n) = 0;
				DIRECT_MULTIDIM_ELEM(pVarianzeResolution, n) = 0;
			}
		}

		MaxResolution.write(fnMaxVol);
		MinResolution.write(fnMinVol);
		AvgResolution.write(fnOut);
		VarianzeResolution.write(fnVar);

		double sumS=0, sumS2=0, N_elem=0;
		FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(pOutputResolution)
		{
			double value=DIRECT_MULTIDIM_ELEM(pOutputResolution, n);
			if (DIRECT_MULTIDIM_ELEM(pOutputResolution, n)>0)
			{
				sumS  += value;
				sumS2 += value*value;
				++N_elem;
			}
		}

		double avg = sumS/N_elem;
		MAT_ELEM(angles, 2, dir) = avg;
		MAT_ELEM(angles, 3, dir) = sumS2/N_elem-avg*avg;

		/////////////////////////
		size_t objId;
		objId = md.addObject();
		md.setValue(MDL_ANGLE_ROT, rot*180/PI, objId);
		md.setValue(MDL_ANGLE_TILT, tilt*180/PI, objId);
		md.setValue(MDL_RESOLUTION_FREQREAL, avg, objId);
		md.setValue(MDL_STDDEV, MAT_ELEM(angles, 3, dir), objId);

		md.write(fnMd);

		/////////////////////////
		#ifdef DEBUG_DIR
		Image<double> saveImg;
		saveImg = pOutputResolution;
		FileName fnres = formatString("resolution_dir_%i.vol", dir);
		saveImg.write(fnres);
		saveImg.clear();
		#endif

		pOutputResolution.clear();
		pMaxResolution.clear();
		pMinResolution.clear();
		pVarianzeResolution.clear();
		list.clear();
	}
	Image<double> VarianzeResolution, MaxResolution, MinResolution, AvgResolution;
	AvgResolution.read(fnOut);
	MultidimArray<double> &pAvgResolution = AvgResolution();

	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(pAvgResolution)
	{
		DIRECT_MULTIDIM_ELEM(pAvgResolution, n) /= N_directions;
		DIRECT_MULTIDIM_ELEM(pAvgResolution, n) *= DIRECT_MULTIDIM_ELEM(mask(), n);
	}
	AvgResolution.write(fnOut);
	#ifdef DEBUG
	Image<int> saveImg_int;
	FileName fnm;
	saveImg_int = mask();
	saveImg_int.write("maskfinalk.vol");
	saveImg_int.clear();
	#endif

	MaxResolution.read(fnMaxVol);
	MinResolution.read(fnMinVol);
	VarianzeResolution.read(fnVar);

	MultidimArray<double> &pMaxResolution = MaxResolution();
	MultidimArray<double> &pMinResolution = MinResolution();
	MultidimArray<double> &pVarianzeResolution = VarianzeResolution();

	double max_voxel, min_voxel;
	FOR_ALL_DIRECT_ELEMENTS_IN_MULTIDIMARRAY(pAvgResolution)
	{
		if (DIRECT_MULTIDIM_ELEM(mask(), n) == 1)
		{
			max_voxel = DIRECT_MULTIDIM_ELEM(pMaxResolution, n);
			min_voxel = DIRECT_MULTIDIM_ELEM(pMinResolution, n);
			double resmean = DIRECT_MULTIDIM_ELEM(pAvgResolution, n);
			DIRECT_MULTIDIM_ELEM(pVarianzeResolution, n) = (DIRECT_MULTIDIM_ELEM(pVarianzeResolution, n)/N_directions) - (resmean*resmean/(N_directions*N_directions));
			DIRECT_MULTIDIM_ELEM(pAvgResolution, n) = (max_voxel - min_voxel)/(max_voxel + min_voxel);
		}
		else
		{
			DIRECT_MULTIDIM_ELEM(pAvgResolution, n) = 0;
		}
	}
	AvgResolution.write(fnDoA);
	VarianzeResolution.write(fnVar);
}