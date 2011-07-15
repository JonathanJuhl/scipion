#!/usr/bin/env python
#------------------------------------------------------------------------------------------------
# Xmipp protocol for projection matching
#
# Example use:
# ./xmipp_protocol_projmatch.py
#
# Authors: Roberto Marabini,
#          Sjors Scheres,    March 2008
#        Rewritten by Roberto Marabini
#

# {begin_of_header}
#------------------------------------------------------------------------------------------
# {section}{has_question} Comment
#------------------------------------------------------------------------------------------
# Display comment
DisplayComment = False

# {text} Write a comment:
""" 
Describe your run here...
"""
#-----------------------------------------------------------------------------
# {section} Global parameters
#-----------------------------------------------------------------------------
# Run name:
""" This will identify your protocol run. It need to be unique for each protocol. You could have run1, run2 for protocol X, but not two
run1 for it. This name together with the protocol output folder will determine the working dir for this run.
"""
RunName = "run_001"

#Comment
Comment='Describe your project here...'

# {file} Selfile with the input images:
""" This selfile points to the spider single-file format images that make up 
your data set. The filenames can have relative or absolute paths, but it is 
strictly necessary that you put this selfile IN THE PROJECTDIR. 
"""
SelFileName ='new20.sel'

# {file} {expert} Docfile with the input angles:
""" Do not provide anything if there are no angles yet. 
    In that case, the starting angles will be read from the image headers
    This docfile should be in newXmipp-style format (with filenames as comments)
    Note that all filenames in this docfile should be with absolute paths!
"""
DocFileName =' '

# {file} Initial 3D reference map:
""" Write down the reference/es name. For example "Reference1.vol Reference2.vol"
    specifies two references
"""
ReferenceFileNames ='ico1.vol ico2.vol ico3.vol'


# Delete working subdirectory if it already exists?
""" Just be careful with this option...
"""
DoDeleteWorkingDir =True

# Number of iterations to perform
NumberOfIterations = 4

# {expert} Resume at Iter (vs Step)
"""This option control how to resume a previously performed run.
    Set to TRUE to restart at the beginning of iteration N
    or FALSE to continue at step N. (N is set in the next parameter).
    NOTE:If you do not know what are you doing make it equal to False
"""
IsIter =False

# Resume at iteration
""" Set to 1 to start a new run, set to -1 to continue the process (where you left it),
    set to a positive number N to restart at the begining of iteration N
    Note1: Do NOT delete working directory if this option is not set to 1
    Note2: Set this option to -1 if you want to perform extra iterations after
           successfully finish an execution
"""
ContinueAtIteration =1

# {expert} Save disc space by cleaning up intermediate files?
""" Be careful, many options of the visualization protocol will not work anymore, 
    since all class averages, selfiles etc will be deleted.
"""
CleanUpFiles =False


#-----------------------------------------------------------------------------
# {section}{has_question} CTF correction
#-----------------------------------------------------------------------------
# Perform CTF correction
""" If set to true, a CTF (amplitude and phase) corrected map will be refined,
    and the data will be processed in CTF groups.
    Note that you cannot combine CTF-correction with re-alignment of the classes.
"""
DoCtfCorrection = True

# {file} CTFDat file with CTF data:
""" The input selfile may be a subset of the images in the CTFDat file, but all 
    images in the input selfile must be present in the CTFDat file. This field is 
    obligatory if CTF correction is to be performed. 
    Note that this file should be positioned in the project directory, and that the
    image names and ctf parameter filenames should be in absolute paths.
"""
CTFDatName ='new_ctf.ctfdat'

# Make CTF groups automatically?
""" Make CTF groups based on a maximum differences at a given resolution limit.
    If this option is set to false, a docfile with the defocus values where to 
    split the images in distinct defocus group has to be provided (see expert option below)
"""
DoAutoCtfGroup =True

# Maximum difference in CTF-values in one group
""" If the difference between the CTF-values up to the resolution limit specified 
    below is larger than the value given here, two images will be placed in 
    distinct CTF groups.
"""
CtfGroupMaxDiff = 0.1

# Resolution limit (Ang) for CTF-differences in one group
""" Maximum resolution where to consider CTF-differences among different groups.
    One should use somewhat higher resolutions than those aimed for in the refinement.
"""
CtfGroupMaxResol = 5.6

# {file} {expert} Docfile with defocus values where to split into groups
""" This field is obligatory if you do not want to make the CTF groups automatically.
    Note that the requested docfile can be made initially with the xmipp_ctf_group program,
    and then it can be edited manually to suit your needs. 
"""
SplitDefocusDocFile =''

# {expert} Padding factor
""" Application of CTFs to reference projections and of Wiener filter to class averages will be done using padded images.
    Use values larger than one to pad the images. Suggestion, use 1 for large image and 2 for small
"""
PaddingFactor =2

# {expert} Wiener constant
""" Term that will be added to the denominator of the Wiener filter.
    In theory, this value is the inverse of the signal-to-noise ratio
    If a negative value is taken, the program will use a default value as in FREALIGN 
    (i.e. 10% of average sum terms over entire space) 
    see Grigorieff JSB 157 (2006) pp117-125
"""
WienerConstant = -1

# Images have been phase flipped?
DataArePhaseFlipped =True

# Is the initial reference map CTF (amplitude) corrected?
"""
    You may specify this option for each iteration. 
    This can be done by a sequence of 0 or 1 numbers (for instance, "1 1 0 0" 
    specifies 4 iterations, the first two applied alig2d while the last 2
    dont. an alternative compact notation is 
    is ("2x1 2x0", i.e.,
    2 iterations with value 1, and 2 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    IMPORTANT: if you set this variable to 0 the output  of the projection
    muching step will be copied as output of align2d
"""
ReferenceIsCtfCorrected ='1'

#-----------------------------------------------------------------------------
# {section} {has_question} Mask
#-----------------------------------------------------------------------------
# Mask reference volume
""" Masking the reference volume will increase the signal to noise ratio.
    Do not provide a very tight mask.
    See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Mask for details
"""
DoMask =True

# Use a spherical mask?
""" If set to true, provide the radius of the mask in the next input field
    if set to false, provide a binary mask file in the second next input field
"""
DoSphericalMask =True

# {condition}(DoSphericalMask=True) Radius of spherical mask
""" This is the radius (in pixels) of the spherical mask 
"""
MaskRadius = 64

# {file} {condition}(DoSphericalMask=False)  Binary mask file
""" This should be a binary (only 0/1-valued) Xmipp volume of equal dimension as your reference
    The protein region should be white (1) and the solvent should be black (0).
    Note that this entry is only relevant if no spherical mask is used.
"""
MaskFileName ='mask.vol'

#-----------------------------------------------------------------------------
# {section} Projection Matching
#-----------------------------------------------------------------------------
# Inner radius for rotational correlation:
""" In pixels from the image center
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "8 8 2 2 " 
    specifies 4 iterations, the first two set the value to 8 
    and the last two to 2. An alternative compact notation 
    is ("2x8 2x0", i.e.,
    2 iterations with value 8, and 2 with value 2).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
InnerRadius = '0'

# Outer radius for rotational correlation
""" In pixels from the image center. Use a negative number to use the entire image.
    WARNING: this radius will be use for masking before computing resolution
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "8 8 2 2 " 
    specifies 4 iterations, the first two set the value to 8 
    and the last two to 2. An alternative compact notation 
    is ("2x8 2x0", i.e.,
    2 iterations with value 8, and 2 with value 2).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
OuterRadius = '64'

# {expert} Available memory to store all references (Gb)
""" This is only for the storage of the references. If your projections do not fit in memory, 
    the projection matching program will run MUCH slower. But, keep in mind that probably 
    some additional memory is needed for the operating system etc.
    Note that the memory per computing node needs to be given. That is, when using threads, 
    this value will be multiplied automatically by the number of (shared-memory) threads.
"""
AvailableMemory = 2

# Angular sampling rate
"""Angular distance (in degrees) between neighboring projection  points
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "8 8 2 2 " 
    specifies 4 iterations, the first two set the value to 8 
    and the last two to 2. An alternative compact notation 
    is ("2x8 2x0", i.e.,
    2 iterations with value 8, and 2 with value 2).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
AngSamplingRateDeg='1 3 2 1'

# Angular search range 
"""Maximum change in rot & tilt  (in +/- degrees)
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1000 1000 10 10 " 
    specifies 4 iterations, the first two set the value to 1000 (no restriction)
    and the last two to 10degrees. An alternative compact notation 
    is ("2x1000 2x10", i.e.,
    2 iterations with value 1000, and 2 with value 10).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
MaxChangeInAngles='1000 10 4 2'

# {expert} Perturb projection directions?
""" If set to 1, this option will result to a Gaussian perturbation to the 
    evenly sampled projection directions of the reference library. 
    This may serve to decrease the effects of model bias.
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1 1 0" 
    specifies 3 iterations, the first two set the value to 1 
    and the last to 0. An alternative compact notation 
    is ("2x1 0", i.e.,
    2 iterations with value 1, and 1 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
PerturbProjectionDirections ='0'

# Maximum change in origin offset
""" Maximum allowed change in shift in the 3D+2D searches (in +/- pixels).
    Shifts larger than this value will be reset to (0,0)
    You must specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1000 1000 10 10 " 
    specifies 4 iterations, the first two set the value to 1000 (no restriction)
    and the last two to 10degrees. An alternative compact notation 
    is ("2x1000 2x10", i.e.,
    2 iterations with value 1000, and 2 with value 10).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
MaxChangeOffset='1000 10 5'

# Search range for 5D translational search 
""" Give search range from the image center for 5D searches (in +/- pixels).
    Values larger than 0 will results in 5D searches (which may be CPU-intensive)
    Give 0 for conventional 3D+2D searches. 
    Note that after the 5D search, for the optimal angles always 
    a 2D exhaustive search is performed anyway (making it ~5D+2D)
    Provide a sequence of numbers (for instance, "5 5 3 0" specifies 4 iterations,
    the first two set the value to 5, then one with 3, resp 0 pixels.
    An alternative compact notation is ("3x5 2x3 0", i.e.,
    3 iterations with value 5, and 2 with value 3 and the rest with 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    
"""
Search5DShift ='4x5 0'

# {expert} Step size for 5D translational search
""" Provide a sequence of numbers (for instance, "2 2 1 1" specifies 4 iterations,
    the first two set the value to 2, then two with 1 pixel.
    An alternative compact notation is ("2x2 2x1", i.e.,
    2 iterations with value 2, and 2 with value 1).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    
"""
Search5DStep ='2'

# {expert} Restrict tilt angle search?
DoRestricSearchbyTiltAngle = False

# {expert} {condition}(DoRestricSearchbyTiltAngle=True) Lower-value for restricted tilt angle search
Tilt0 = -91

# {expert} {condition}(DoRestricSearchbyTiltAngle=True) Higher-value for restricted tilt angle search
TiltF = 91

# Symmetry group
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Symmetry
    for a description of the symmetry groups format
    If no symmetry is present, give c1
"""
SymmetryGroup ='i3'

# {expert} Symmetry group for Neighbourhood computations
""" If you do not know what this is leave it blank.
    This symmetry will be using for compute neighboring points,
    but not for sampling or reconstruction
    See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Symmetry
    for a description of the symmetry groups format
    If no symmetry is present, give c1
"""
SymmetryGroupNeighbourhood =''

# {expert} compute only closest neighbor 
""" This option is only relevant if SymmetryGroupNeighbourhood !=''
    If set to 1 only one neighbor will be computed per sampling point
    You may specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1 1 0" 
    specifies 3 iterations, the first two set the value to 1 
    and the last to 0. An alternative compact notation 
    is ("2x1 0", i.e.,
    2 iterations with value 1, and 1 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
OnlyWinner ='0'

# Discard images with ccf below
""" Provide a sequence of numbers (for instance, "0.3 0.3 0.5 0.5" specifies 4 iterations,
    the first two set the value to 0.3, then two with 0.5.
    An alternative compact notation would be ("2x0.3 2x0.5").
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    Set to -1 to prevent discarding any images
"""
MinimumCrossCorrelation ='-1'

# Discard percentage of images with ccf below
""" Provide a sequence of numbers (for instance, "20 20 10 10" specifies 4 iterations,
    the first two set the value to 20%, then two with 10%
    An alternative compact notation would be ("2x20 2x10").
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    Set to zero to prevent discarding any images
"""
DiscardPercentage ='10'

# Perform scale search?
""" If true perform scale refinement
"""
DoScale = False

# {condition}(DoScale=True) Step scale factors size
""" Scale step factor size (1 means 0.01 in/de-crements arround 1)
"""
ScaleStep ='1'

# {condition}(DoScale=True) Number of scale steps
""" Number of scale steps.
    With default values (ScaleStep='1' and ScaleNumberOfSteps='3'): 1 +/-0.01 | +/-0.02 | +/-0.03.    
    With values ScaleStep='2' and ScaleNumberOfSteps='4' it performs a scale search over:
     1 +/-0.02 | +/-0.04 | +/-0.06 | +/-0.08.    
"""
ScaleNumberOfSteps ='3'


# {expert} Additional options for Projection_Matching
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Projection_matching and
        http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Mpi_projection_matching for details
    try -Ri xx -Ro yy for restricting angular search (xx and yy are
    the particle inner and outter radius)
    
"""
ProjMatchingExtra =''

#-----------------------------------------------------------------------------
# {section}{expert}{has_question} 2D re-alignment of classes
#-----------------------------------------------------------------------------
# Perform 2D re-alignment
PerformAlign2D = False

# Perform 2D re-alignment of classes?
""" After performing a 3D projection matching iteration, each of the
    subsets of images assigned to one of the library projections is
    re-aligned using a 2D-alignment protocol.
    This may serve to remove model bias.
    See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Align2d for details
    Note that you cannot combine this option with CTF-correction!
    You may specify this option for each iteration. 
    This can be done by a sequence of 0 or 1 numbers (for instance, "1 1 0 0" 
    specifies 4 iterations, the first two applied alig2d while the last 2
    dont. an alternative compact notation is 
    is ("2x1 2x0", i.e.,
    2 iterations with value 1, and 2 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
    IMPORTANT: if you set this variable to 0 the output  of the projection
    muching step will be copied as output of align2d
"""
DoAlign2D ='0'

# Number of align2d iterations:
""" Use at least 3 iterations
    The number of align iteration may change in each projection matching iteration
    Ffor instance, "4 4 3 3 " 
    specifies 4 alig2d iterations in the first projection matching iteration 
    and  two 3 alig2d iteration in the last 2 projection matching iterations.
     An alternative compact notation 
    is ("2x4 2x3", i.e.,
    2 iterations with value 4, and 2 with value 3).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
Align2DIterNr ='4'

# Maximum change in origin offset (+/- pixels)
"""Maximum change in shift  (+/- pixels)
    You must specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1000 1000 10 10 " 
    specifies 4 iterations, the first two set the value to 1000 (no restriction)
    and the last two to 10degrees. An alternative compact notation 
    is ("2x1000 2x10", i.e.,
    2 iterations with value 1000, and 2 with value 10).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
Align2dMaxChangeOffset ='2x1000 2x10'

# Maximum change in rotation (+/- degrees)
"""Maximum change in shift  (+/- pixels)
    You must specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, "1000 1000 10 10 " 
    specifies 4 iterations, the first two set the value to 1000 (no restriction)
    and the last two to 10degrees. An alternative compact notation 
    is ("2x1000 2x10", i.e.,
    2 iterations with value 1000, and 2 with value 10).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
Align2dMaxChangeRot ='2x1000 2x20'

#-----------------------------------------------------------------------------
# {section} 3D Reconstruction
#-----------------------------------------------------------------------------

# {list}(fourier, art, wbp) Reconstruction method
""" Choose between wbp, art or fourier
"""
ReconstructionMethod ='fourier'

# {expert}{condition}(ReconstructionMethod=art) Values of lambda for art
""" IMPORTANT: ou must specify a value of lambda for each iteration even
    if art has not been selected.
    IMPORTANT: NOte that we are using the WLS version of ART that 
    uses geater lambdas than the plain art.
    See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Art
        for details
    You must specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, ".1 .1 .3 .3" 
    specifies 4 iterations, the first two set the value to 0.1 
    (no restriction)
    and the last  two to .3. An alternative compact notation 
    is ("2x.1 2x.3").
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
ARTLambda ='0.2'

# {expert}{condition}(ReconstructionMethod=art) Additional reconstruction parameters for ART
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Art
        for details
"""
ARTReconstructionExtraCommand ='-k 0.5 -n 10 '

# {condition}(ReconstructionMethod=fourier) Initial maximum frequency
""" This number os only used in the first iteration. 
    From then on, it will be set to resolution computed in the resolution section
"""
FourierMaxFrequencyOfInterest =0.25

# {expert}{condition}(ReconstructionMethod=wbp) Additional reconstruction parameters for WBP
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Wbp and
        http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Mpi_wbp and
        for details
"""
WBPReconstructionExtraCommand =''

# {expert} {condition}(ReconstructionMethod=fourier)Additional reconstruction parameters for Fourier
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Fourier and
        http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Mpi_Fourier and
        for details
    -thr_width 
"""
FourierReconstructionExtraCommand =''

#-----------------------------------------------------------------------------
# {section} Compute Resolution
#-----------------------------------------------------------------------------
# Compute resolution?
""" See http://xmipp.cnb.uam.es/twiki/bin/view/Xmipp/Resolution for details
    You may specify this option for each iteration. 
    This can be done by a sequence of 0 or 1 numbers (for instance, "1 1 0 0" 
    specifies 4 iterations, the first two applied alig2d while the last 2
    dont. an alternative compact notation is 
    is ("2x1 2x0", i.e.,
    2 iterations with value 1, and 2 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
DoComputeResolution ='1'

# {expert} Split references averages
"""In theory each reference average should be splited
   in two when computing the resolution. In this way each
   projection direction will be represented in each of the
   subvolumes used to compute the resolution. A much faster
   but less accurate approach is to split the 
   proyection directions in two but not the averages. We
   recomend the first approach for small volumes and the second for
   large volumes (especially when using small angular
   sampling rates.
   IMPORTANT: the second option has ONLY been implemented for FOURIER
   reconstruction method. Other reconstruction methods require this
   flag to be set to True
    You may specify this option for each iteration. 
    This can be done by a sequence of 0 or 1 numbers (for instance, "1 1 0 0" 
    specifies 4 iterations, the first two split the images   while the last 2
    don't. an alternative compact notation is 
    is ("2x1 2x0", i.e.,
    2 iterations with value 1, and 2 with value 0).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more vapplications/scripts/protocols/new_protocol_projmatch.pyalues than iterations the extra value are ignored
"""
DoSplitReferenceImages ="1"


# Pixel size (in Ang.)
""" This will make that the X-axis in the resolution plots has units 1/Angstrom
"""
ResolSam=5.6

#-----------------------------------------------------------------------------
# {section} Postprocessing
#-----------------------------------------------------------------------------
# Low-pass filter the reference?
DoLowPassFilter = True

# {condition}(DoLowPassFilter=True) Use estimated resolution for low-pass filtering?
"""If set to true, the volume will be filtered at a frecuency equal to
   the  resolution computed with a FSC=0.5 threshold, possibly 
   plus a constant provided by the user in the next input box. 

   If set to false, then the filtration will be made at the constant 
   value provided by the user in the next box (in digital frequency, 
   i.e. pixel-1: minimum 0, maximum 0.5) 
"""
UseFscForFilter = True

# {condition}(DoLowPassFilter=True) Constant to by add to the estimated resolution
""" The meaning of this field depends on the previous flag.
    If set to true, then the volume will be filtered at a frecuency equal to
    the  resolution computed with resolution_fsc (FSC=0.5) plus the value 
    provided in this field 
    If set to false, the volume will be filtered at the resolution
    provided in this field 
    This value is in digital frequency, or pixel^-1: minimum 0, maximum 0.5

    You can specify this option for each iteration. 
    This can be done by a sequence of numbers (for instance, ".15 .15 .1 .1" 
    specifies 4 iterations, the first two set the constant to .15
    and the last two to 0.1. An alternative compact notation 
    is ("2x.15 2x0.1", i.e.,
    4 iterations with value 0.15, and three with value .1).
    Note: if there are less values than iterations the last value is reused
    Note: if there are more values than iterations the extra value are ignored
"""
ConstantToAddToFiltration ='0.1'

# {expert} Center volume
""" Center volume after each iteration """
DoCenterVolume = False

#------------------------------------------------------------------------------------------------
# {section} Parallelization issues
#------------------------------------------------------------------------------------------------
# Number of (shared-memory) threads?
""" This option provides shared-memory parallelization on multi-core machines. 
    It does not require any additional software, other than xmipp
"""
NumberOfThreads = 1

# Number of MPI processes to use
NumberOfMpiProcesses = 3

#MPI job size 
"""Minimum size of jobs in mpi processes. Set to 1 for large images (e.g. 500x500) and to 10 for small images (e.g. 100x100)
"""
MpiJobSize ='1'

#------------------------------------------------------------------------------------------
# {section}{has_question} Queue
#------------------------------------------------------------------------------------------
# Submmit to queue
"""Submmit to queue
"""
SubmmitToQueue = True

# Queue name
"""Name of the queue to submit the job
"""
QueueName = "default"

# Queue hours
"""This establish a maximum number of hours the job will
be running, after that time it will be killed by the
queue system
"""
QueueHours = 72

#------------------------------------------------------------------------------------------------
# {expert} Analysis of results
""" This script serves only for GUI-assisted visualization of the results
"""
AnalysisScript ='visualize_projmatch.py'

#-----------------------------------------------------------------------------
# {section}{has_question} Debug
#-----------------------------------------------------------------------------
# Do debug
"""Check that some output files are created. 
"""
Verify=True

# {expert} print wrapper name
PrintWrapperCommand=True

# {expert} print wrapper parameters
PrintWrapperParameters=True

# {expert} show file verification
ViewVerifyedFiles=True 

# {hidden} Show expert options
"""If True, expert options will be displayed
"""
ShowExpertOptions = False

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# {end_of_header} USUALLY YOU DO NOT NEED TO MODIFY ANYTHING BELOW THIS LINE ...
#-----------------------------------------------------------------------------
       
from protocol_projmatch import *

if __name__ == '__main__':
    protocolMain(ProtProjMatch)