#define EPI 0
#define MNI 1

#define MATLAB 0
#define PYTHON 1
#define BASH 2

#define CCA 0
#define GLM 1

#define SILENT 0
#define VERBOSE 1

#define NO 0
#define YES 1

#define RAW 0
#define NIFTI 1

#define DISCARD_DISPLACEMENT_FIELD 0
#define KEEP_DISPLACEMENT_FIELD 1

#define FLOAT 0
#define INT32 1
#define INT16 2
#define UINT32 3
#define UINT16 4
#define DOUBLE 5
#define UINT8 6

#define SLICE_TIMING_CORRECTION 0
#define MOTION_CORRECTION 1
#define SMOOTHING 2
#define DETRENDING 3
#define STATISTICAL_ANALYSIS 4
#define PERMUTATION_TEST 5
#define COPY 6
#define CONVOLVE 7
#define PHASEDC 8
#define PHASEG 9
#define AH2D 10
#define EQSYSTEM 11
#define INTERPOLATION 12

#define DONT_CALCULATE_VOXEL_LABELS 0
#define CALCULATE_VOXEL_LABELS 1

#define DONT_CALCULATE_CLUSTER_MASS 0
#define CALCULATE_CLUSTER_MASS 1

#define VOXEL 0
#define CLUSTER_EXTENT 1
#define CLUSTER_MASS 2
#define TFCE 3

#define VALID_FILTER_RESPONSES_X_SEPARABLE_CONVOLUTION_ROWS 32
#define VALID_FILTER_RESPONSES_Y_SEPARABLE_CONVOLUTION_ROWS 8
#define VALID_FILTER_RESPONSES_Z_SEPARABLE_CONVOLUTION_ROWS 8

#define VALID_FILTER_RESPONSES_X_SEPARABLE_CONVOLUTION_COLUMNS 24
#define VALID_FILTER_RESPONSES_Y_SEPARABLE_CONVOLUTION_COLUMNS 16
#define VALID_FILTER_RESPONSES_Z_SEPARABLE_CONVOLUTION_COLUMNS 8

#define VALID_FILTER_RESPONSES_X_SEPARABLE_CONVOLUTION_RODS 32
#define VALID_FILTER_RESPONSES_Y_SEPARABLE_CONVOLUTION_RODS 8
#define VALID_FILTER_RESPONSES_Z_SEPARABLE_CONVOLUTION_RODS 8

#define NOT_SKULL_STRIPPED 1
#define SKULL_STRIPPED 0

#define HALO 3
#define VALID_FILTER_RESPONSES_X_CONVOLUTION_2D_24KB 90
#define VALID_FILTER_RESPONSES_Y_CONVOLUTION_2D_24KB 58

#define VALID_FILTER_RESPONSES_X_CONVOLUTION_2D_32KB 122
#define VALID_FILTER_RESPONSES_Y_CONVOLUTION_2D_32KB 58

#define TTEST 0
#define FTEST 1
#define GROUP_MEAN 2

#define UP 0
#define DOWN 1
#define UP_INTERLEAVED 2
#define DOWN_INTERLEAVED 3
#define CUSTOM 4
#define UNDEFINED 5

#define TRANSLATION 0
#define RIGID 1
#define AFFINE 2

#define NEAREST 0
#define LINEAR 1
#define CUBIC 2

#define DO_OVERWRITE 0
#define NO_OVERWRITE 1

#define PI 3.14159265359

#define INITIAL_MM_T1_Z_CUT 15

#define SUCCESS 0
#define FAIL 1

#define LOWPASS 0
#define RANDOM 1

#define NVIDIA 0
#define INTEL 1
#define AMD 2
#define APPLE 3

#define NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION 6

#define CL_SUCCESS 0
#define CL_DEVICE_NOT_FOUND -1
#define CL_DEVICE_NOT_AVAILABLE -2
#define CL_COMPILER_NOT_AVAILABLE -3
#define CL_MEM_OBJECT_ALLOCATION_FAILURE -4
#define CL_OUT_OF_RESOURCES -5
#define CL_OUT_OF_HOST_MEMORY -6
#define CL_PROFILING_INFO_NOT_AVAILABLE -7
#define CL_MEM_COPY_OVERLAP -8
#define CL_IMAGE_FORMAT_MISMATCH -9
#define CL_IMAGE_FORMAT_NOT_SUPPORTED -10
#define CL_BUILD_PROGRAM_FAILURE -11
#define CL_MAP_FAILURE -12

#define CL_INVALID_VALUE -30
#define CL_INVALID_DEVICE_TYPE -31
#define CL_INVALID_PLATFORM -32
#define CL_INVALID_DEVICE -33
#define CL_INVALID_CONTEXT -34
#define CL_INVALID_QUEUE_PROPERTIES -35
#define CL_INVALID_COMMAND_QUEUE -36
#define CL_INVALID_HOST_PTR -37
#define CL_INVALID_MEM_OBJECT -38
#define CL_INVALID_IMAGE_FORMAT_DESCRIPTOR -39
#define CL_INVALID_IMAGE_SIZE -40
#define CL_INVALID_SAMPLER -41
#define CL_INVALID_BINARY -42
#define CL_INVALID_BUILD_OPTIONS -43
#define CL_INVALID_PROGRAM -44
#define CL_INVALID_PROGRAM_EXECUTABLE -45
#define CL_INVALID_KERNEL_NAME -46
#define CL_INVALID_KERNEL_DEFINITION -47
#define CL_INVALID_KERNEL -48
#define CL_INVALID_ARG_INDEX -49
#define CL_INVALID_ARG_VALUE -50
#define CL_INVALID_ARG_SIZE -51
#define CL_INVALID_KERNEL_ARGS -52
#define CL_INVALID_WORK_DIMENSION -53
#define CL_INVALID_WORK_GROUP_SIZE -54
#define CL_INVALID_WORK_ITEM_SIZE -55
#define CL_INVALID_GLOBAL_OFFSET -56
#define CL_INVALID_EVENT_WAIT_LIST -57
#define CL_INVALID_EVENT -58
#define CL_INVALID_OPERATION -59
#define CL_INVALID_GL_OBJECT -60
#define CL_INVALID_BUFFER_SIZE -61
#define CL_INVALID_MIP_LEVEL -62
#define CL_INVALID_GLOBAL_WORK_SIZE -63
