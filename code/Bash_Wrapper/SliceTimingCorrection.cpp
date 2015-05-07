/*
 * BROCCOLI: An open source multi-platform software for parallel analysis of fMRI data on many core CPUs and GPUS
 * Copyright (C) <2013>  Anders Eklund, andek034@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "broccoli_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include "nifti1_io.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include <limits.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#define ADD_FILENAME true
#define DONT_ADD_FILENAME true

#define CHECK_EXISTING_FILE true
#define DONT_CHECK_EXISTING_FILE false

std::string Getexepath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return std::string( result, (count > 0) ? count : 0 );
}

void FreeAllMemory(void **pointers, int N)
{
    for (int i = 0; i < N; i++)
    {
        if (pointers[i] != NULL)
        {
            free(pointers[i]);
        }
    }
}

void FreeAllNiftiImages(nifti_image **niftiImages, int N)
{
    for (int i = 0; i < N; i++)
    {
		if (niftiImages[i] != NULL)
		{
			nifti_image_free(niftiImages[i]);
		}
    }
}

void ReadBinaryFile(float* pointer, int size, const char* filename, void** pointers, int& Npointers, nifti_image** niftiImages, int Nimages)
{
	if (pointer == NULL)
    {
        printf("The provided pointer for file %s is NULL, aborting! \n",filename);
        FreeAllMemory(pointers,Npointers);
		FreeAllNiftiImages(niftiImages,Nimages);
        exit(EXIT_FAILURE);
	}	

	FILE *fp = NULL; 
	fp = fopen(filename,"rb");

    if (fp != NULL)
    {
        fread(pointer,sizeof(float),size,fp);
        fclose(fp);
    }
    else
    {
        printf("Could not open %s , aborting! \n",filename);
        FreeAllMemory(pointers,Npointers);
		FreeAllNiftiImages(niftiImages,Nimages);
        exit(EXIT_FAILURE);
    }
}

void AllocateMemory(float *& pointer, size_t size, void** pointers, int& Npointers, nifti_image** niftiImages, int Nimages, const char* variable)
{
    pointer = (float*)malloc(size);
    if (pointer != NULL)
    {
        pointers[Npointers] = (void*)pointer;
        Npointers++;
    }
    else
    {
        printf("Could not allocate host memory for variable %s ! \n",variable);        
		FreeAllMemory(pointers, Npointers);
		FreeAllNiftiImages(niftiImages, Nimages);
		exit(EXIT_FAILURE);        
    }
}


bool WriteNifti(nifti_image* inputNifti, float* data, const char* filename, bool addFilename, bool checkFilename)
{       
	if (data == NULL)
    {
        printf("The provided data pointer for file %s is NULL, aborting writing nifti file! \n",filename);
		return false;
	}	
	if (inputNifti == NULL)
    {
        printf("The provided nifti pointer for file %s is NULL, aborting writing nifti file! \n",filename);
		return false;
	}	


    char* filenameWithExtension;
    
    // Add the provided filename to the original filename, before the dot
    if (addFilename)
    {
        // Find the dot in the original filename
        const char* p = inputNifti->fname;
        int dotPosition = 0;
        while ( (p != NULL) && ((*p) != '.') )
        {
            p++;
            dotPosition++;
        }
    
        // Allocate temporary array
        filenameWithExtension = (char*)malloc(strlen(inputNifti->fname) + strlen(filename) + 1);
        if (filenameWithExtension == NULL)
        {
            printf("Could not allocate temporary host memory! \n");      
            return false;
        }
    
        // Copy filename to the dot
        strncpy(filenameWithExtension,inputNifti->fname,dotPosition);
        filenameWithExtension[dotPosition] = '\0';
        // Add the extension
        strcat(filenameWithExtension,filename);
        // Add the rest of the original filename
        strcat(filenameWithExtension,inputNifti->fname+dotPosition);    
    }
        
    // Copy information from input data
    nifti_image *outputNifti = nifti_copy_nim_info(inputNifti);    
    // Set data pointer 
    outputNifti->data = (void*)data;        
    // Set data type to float
    outputNifti->datatype = DT_FLOAT;
    outputNifti->nbyper = 4;    
    
    // Change filename and write
    bool written = false;
    if (addFilename)
    {
        if ( nifti_set_filenames(outputNifti, filenameWithExtension, checkFilename, 1) == 0)
        {
            nifti_image_write(outputNifti);
            written = true;
        }
    }
    else if (!addFilename)
    {
        if ( nifti_set_filenames(outputNifti, filename, checkFilename, 1) == 0)
        {
            nifti_image_write(outputNifti);
            written = true;
        }                
    }    
    
    outputNifti->data = NULL;
    nifti_image_free(outputNifti);

    if (addFilename)
    {
        free(filenameWithExtension);
    } 
        
    if (written)
    {      
        return true;
    }
    else
    {
        return false;
    }                        
}    

double GetWallTime()
{
    struct timeval time;
    if (gettimeofday(&time,NULL))
    {
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


int main(int argc, char ** argv)
{
    //-----------------------
    // Input pointers
    
    float           *h_fMRI_Volumes = NULL;
    
    void*			allMemoryPointers[500];
    int             numberOfMemoryPointers = 0;

	nifti_image*	allNiftiImages[500];
    int             numberOfNiftiImages = 0;

	float			h_Custom_Slice_Times[1000];

    // Default parameters
    int             OPENCL_PLATFORM = 0;
    int             OPENCL_DEVICE = 0;
    bool            DEBUG = false;
    const char*     FILENAME_EXTENSION = "_stc";
    bool            PRINT = true;
	bool			VERBOS = false;
    
    int             DATA_W, DATA_H, DATA_D, DATA_T;
    float           EPI_VOXEL_SIZE_X, EPI_VOXEL_SIZE_Y, EPI_VOXEL_SIZE_Z;
	float			TR;
	int				SLICE_ORDER = UNDEFINED;
	bool			DEFINED_SLICE_PATTERN = false;
	bool			DEFINED_SLICE_CUSTOM_REF = false;
	int				SLICE_CUSTOM_REF = 0;
	const char*		SLICE_TIMINGS_FILE;


    //-----------------------
    // Output parameters
    
    const char      *outputFilename;
    
    float           *h_Slice_Timing_Corrected_fMRI_Volumes = NULL;
    
    //---------------------
    
    /* Input arguments */
    FILE *fp = NULL; 
    
    // No inputs, so print help text
    if (argc == 1)
    {        
        printf("Usage:\n\n");
        printf("SliceTimingCorrection input.nii [options]\n\n");
        printf("Options:\n\n");
        printf(" -platform        The OpenCL platform to use (default 0) \n");
        printf(" -device          The OpenCL device to use for the specificed platform (default 0) \n");
        printf(" -slicepattern    The sampling pattern used during scanning (overrides pattern in NIFTI file) \n");
        printf("                  0 = sequential 1-N (bottom-up), 1 = sequential N-1 (top-down), 2 = alternating 1-N, 3 = alternating N-1 \n");        
        printf("                  (no slice timing correction is performed if pattern in NIFTI file is unknown and no pattern is provided) \n");        
		printf(" -slicecustom     Provide a text file with the slice times, one value per slice, in milli seconds (0 - TR) (overrides pattern provided in NIFTI file)\n");
		printf(" -slicecustomref  Reference slice for the custom slice times (0 - (#slices-1)) (default #slices/2)\n");
        printf(" -output          Set output filename (default input_stc.nii) \n");
        printf(" -quiet           Don't print anything to the terminal (default false) \n");
        printf(" -verbose         Print extra stuff (default false) \n");
        printf("\n\n");
        
        return EXIT_SUCCESS;
    }
    // Try to open file
    else if (argc > 1)
    {        
        fp = fopen(argv[1],"r");
        if (fp == NULL)
        {            
            printf("Could not open file %s !\n",argv[1]);
            return EXIT_FAILURE;
        }
        fclose(fp);        
    }
    
    // Loop over additional inputs
    int i = 2;
    while (i < argc)
    {
        char *input = argv[i];
        char *p;
        if (strcmp(input,"-platform") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -platform !\n");
                return EXIT_FAILURE;
			}

            OPENCL_PLATFORM = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("OpenCL platform must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (OPENCL_PLATFORM < 0)
            {
                printf("OpenCL platform must be >= 0!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-device") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -device !\n");
                return EXIT_FAILURE;
			}

            OPENCL_DEVICE = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("OpenCL device must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (OPENCL_DEVICE < 0)
            {
                printf("OpenCL device must be >= 0!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-slicepattern") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicepattern !\n");
                return EXIT_FAILURE;
			}

            SLICE_ORDER = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Slice pattern must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (SLICE_ORDER < 0)
            {
                printf("Slice pattern must be a positive number!\n");
                return EXIT_FAILURE;
            }
            else if ( (SLICE_ORDER != 0) && (SLICE_ORDER != 1) && (SLICE_ORDER != 2) && (SLICE_ORDER != 3) )
            {
                printf("Slice pattern must be 0, 1, 2 or 3!\n");
                return EXIT_FAILURE;
            }
            i += 2;
			DEFINED_SLICE_PATTERN = true;
        }
		else if (strcmp(input,"-slicecustom") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicecustom !\n");
                return EXIT_FAILURE;
			}

			SLICE_ORDER = CUSTOM;
			SLICE_TIMINGS_FILE = argv[i+1];

            i += 2;
			DEFINED_SLICE_PATTERN = true;
        }
        else if (strcmp(input,"-slicecustomref") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicecustomref !\n");
                return EXIT_FAILURE;
			}

            SLICE_CUSTOM_REF = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Reference slice must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (SLICE_CUSTOM_REF < 0)
            {
                printf("Reference slice must be >= 0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
			DEFINED_SLICE_CUSTOM_REF = true;
        }
        else if (strcmp(input,"-debug") == 0)
        {
            DEBUG = true;
            i += 1;
        }
        else if (strcmp(input,"-quiet") == 0)
        {
            PRINT = false;
            i += 1;
        }
        else if (strcmp(input,"-verbose") == 0)
        {
            VERBOS = true;
            i += 1;
        }
        else if (strcmp(input,"-output") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read name after -output !\n");
                return EXIT_FAILURE;
			}

            outputFilename = argv[i+1];
            i += 2;
        }
        else
        {
            printf("Unrecognized option! %s \n",argv[i]);
            return EXIT_FAILURE;
        }                
    }
    
	// Check if BROCCOLI_DIR variable is set
	if (getenv("BROCCOLI_DIR") == NULL)
	{
        printf("The environment variable BROCCOLI_DIR is not set!\n");
        return EXIT_FAILURE;
	}

    double startTime = GetWallTime();

    // Read data
    nifti_image *inputData = nifti_image_read(argv[1],1);
    
    if (inputData == NULL)
    {
        printf("Could not open nifti file!\n");
        return EXIT_FAILURE;
    }
    allNiftiImages[numberOfNiftiImages] = inputData;
	numberOfNiftiImages++;

	double endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to read the nifti file\n",(float)(endTime - startTime));
	}

    // Get data dimensions
    DATA_W = inputData->nx;
    DATA_H = inputData->ny;
    DATA_D = inputData->nz;
    DATA_T = inputData->nt;

    // Get voxel sizes
    EPI_VOXEL_SIZE_X = inputData->dx;
    EPI_VOXEL_SIZE_Y = inputData->dy;
    EPI_VOXEL_SIZE_Z = inputData->dz;
    
    // Get repetition time
    TR = inputData->dt;                           

	if (DEFINED_SLICE_CUSTOM_REF)
	{
	    if (SLICE_CUSTOM_REF >= DATA_D) 
	    {
	    	printf("Reference slice must be < number of slices!\n");
			FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	        return EXIT_FAILURE;
	    }
	}
	else
	{
		SLICE_CUSTOM_REF = (int)round((float)DATA_D/2.0f);
	}

	//---------------------------------------------	
	// Read slice timing information from text file
	
	if (SLICE_ORDER == CUSTOM)
	{		
		std::ifstream slicetimes;
		slicetimes.open(SLICE_TIMINGS_FILE);

		if (!slicetimes.good())    
		{
			slicetimes.close();
	        printf("Unable to open slice timing file %s. Aborting! \n",SLICE_TIMINGS_FILE);
			FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	        return EXIT_FAILURE;
		}

		// Loop over slices
	    for (int slice = 0; slice < DATA_D; slice++)
	    {
	        float time;
    	        
	        // Read onset, duration and value for current event
			if (! (slicetimes >> time) )
			{
	            slicetimes.close();
	            printf("Unable to read the slice time for slice %i in slice timing file %s, aborting! Check the slice timing file. \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}

			if (time > (TR*1000.0f))
			{
	            slicetimes.close();
	            printf("Slice time cannot be larger than the TR! Check the time for slice %i in slice timing file %s ! \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}

			if (time < 0.0f)
			{
	            slicetimes.close();
	            printf("Slice time cannot be negative! Check the time for slice %i in slice timing file %s ! \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}
	
			h_Custom_Slice_Times[slice] = time/1000.0f;		

			if (DEBUG)
			{
				printf("Slice time for slice %i is %f \n",slice,time);
			}
		}
		slicetimes.close();
	}

	// Get fMRI slice order
	int SLICE_ORDER_NIFTI = (int)inputData->slice_code;

	std::string SLICE_ORDER_STRING;
	
	// No slice pattern given by user, so use the one from the nifti file (if not unknown)
	if (!DEFINED_SLICE_PATTERN)
	{
		if (SLICE_ORDER_NIFTI == NIFTI_SLICE_SEQ_INC)
		{
			SLICE_ORDER_STRING = std::string("Seqential increasing");
			SLICE_ORDER = UP;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_SEQ_DEC)
		{
			SLICE_ORDER_STRING = std::string("Seqential decreasing");
			SLICE_ORDER = DOWN;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_INC)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing");
			SLICE_ORDER = UP_INTERLEAVED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_DEC)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing");
			SLICE_ORDER = DOWN_INTERLEAVED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_INC2)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing 2, not yet supported. Use -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_DEC2)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing 2, not yet supported. Use -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
		else
		{
			SLICE_ORDER_STRING = std::string("Unknown, need to specify with option -slicepattern or -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
	}
	// Slice pattern defined by user
	else
	{
		if (SLICE_ORDER == UP)
		{
			SLICE_ORDER_STRING = std::string("Seqential increasing");
		}
		else if (SLICE_ORDER == DOWN)
		{
			SLICE_ORDER_STRING = std::string("Seqential decreasing");
		}
		else if (SLICE_ORDER == UP_INTERLEAVED)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing");
		}
		else if (SLICE_ORDER == DOWN_INTERLEAVED)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing");
		}
		else if (SLICE_ORDER == CUSTOM)
		{
			SLICE_ORDER_STRING = std::string("Custom slice order defined by file");
		}
	}
	
    // Calculate size, in bytes
    size_t DATA_SIZE = DATA_W * DATA_H * DATA_D * DATA_T * sizeof(float);
    size_t VOLUME_SIZE = DATA_W * DATA_H * DATA_D * sizeof(float);
    
    // Print some info
    if (PRINT)
    {
        printf("Authored by K.A. Eklund \n");
        printf("Data size: %i x %i x %i x %i \n",  DATA_W, DATA_H, DATA_D, DATA_T);
        printf("Voxel size: %f x %f x %f mm \n", EPI_VOXEL_SIZE_X, EPI_VOXEL_SIZE_Y, EPI_VOXEL_SIZE_Z);    
        printf("TR: %f s \n", TR);
		printf("Slice order: %s \n",SLICE_ORDER_STRING.c_str());
    } 

	if (SLICE_ORDER == UNDEFINED)
	{
        printf("Slice order unknown in NIFTI file and no slice pattern provided, aborting!\n");
        return EXIT_FAILURE;
	}
    
    // ------------------------------------------------
    
    // Allocate memory on the host
    
	startTime = GetWallTime();

	// If the data is in float format, we can just copy the pointer
	if ( inputData->datatype != DT_FLOAT )
	{
		AllocateMemory(h_fMRI_Volumes, DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, "INPUT_DATA");
	}
    
	endTime = GetWallTime();
    
	if (VERBOS)
 	{
		printf("It took %f seconds to allocate memory\n",(float)(endTime - startTime));
	}

	startTime = GetWallTime();

    // Convert data to floats
    if ( inputData->datatype == DT_SIGNED_SHORT )
    {
        short int *p = (short int*)inputData->data;
    
        for (int i = 0; i < DATA_W * DATA_H * DATA_D * DATA_T; i++)
        {
            h_fMRI_Volumes[i] = (float)p[i];
        }
    }
    else if ( inputData->datatype == DT_UINT8 )
    {
        unsigned char *p = (unsigned char*)inputData->data;
    
        for (int i = 0; i < DATA_W * DATA_H * DATA_D * DATA_T; i++)
        {
            h_fMRI_Volumes[i] = (float)p[i];
        }
    }
    else if ( inputData->datatype == DT_UINT16 )
    {
        unsigned short int *p = (unsigned short int*)inputData->data;
    
        for (int i = 0; i < DATA_W * DATA_H * DATA_D * DATA_T; i++)
        {
            h_fMRI_Volumes[i] = (float)p[i];
        }
    }
	// Correct data type, just copy the pointer
	else if ( inputData->datatype == DT_FLOAT )
    {
		h_fMRI_Volumes = (float*)inputData->data;

		// Save the pointer in the pointer list
		allMemoryPointers[numberOfMemoryPointers] = (void*)h_fMRI_Volumes;
        numberOfMemoryPointers++;		

        //float *p = (float*)inputData->data;
    
        //for (int i = 0; i < DATA_W * DATA_H * DATA_D * DATA_T; i++)
        //{
        //    h_fMRI_Volumes[i] = p[i];
        //}
    }
    else
    {
        printf("Unknown data type in input data, aborting!\n");
        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }

	// Free input fMRI data, it has been converted to floats
	if ( inputData->datatype != DT_FLOAT )
	{		
		free(inputData->data);
		inputData->data = NULL;
	}
	// Pointer has been copied to h_fMRI_Volumes and pointer list, so set the input data pointer to NULL
	else
	{		
		inputData->data = NULL;
	}
    
	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to convert data to floats\n",(float)(endTime - startTime));
	}

    //------------------------
    
	startTime = GetWallTime();

	// Initialize BROCCOLI
    BROCCOLI_LIB BROCCOLI(OPENCL_PLATFORM,OPENCL_DEVICE,2,VERBOS); // 2 = Bash wrapper

	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to initiate BROCCOLI\n",(float)(endTime - startTime));
	}
    
    // Print build info to file (always)
	std::vector<std::string> buildInfo = BROCCOLI.GetOpenCLBuildInfo();
	std::vector<std::string> kernelFileNames = BROCCOLI.GetKernelFileNames();

	for (int k = 0; k < BROCCOLI.GetNumberOfKernelFiles(); k++)
	{
		std::string temp = "buildInfo";
		std::string name = kernelFileNames[k];
		// Remove "kernel" and ".cpp" from kernel filename
		name = name.substr(0,name.size()-4);
		name = name.substr(6,name.size());
		temp.append(name);
		temp.append(".txt");
		fp = fopen(temp.c_str(),"w");
		if (fp == NULL)
		{     
		    printf("Could not open %s for writing ! \n",temp.c_str());
		}
		else
		{	
			if (buildInfo[k].c_str() != NULL)
			{
			    int error = fputs(buildInfo[k].c_str(),fp);
			    if (error == EOF)
			    {
			        printf("Could not write to %s ! \n",temp.c_str());
			    }
			}
			fclose(fp);
		}
	}

    // Something went wrong...
    if (!BROCCOLI.GetOpenCLInitiated())
    {              
        printf("Initialization error is \"%s\" \n",BROCCOLI.GetOpenCLInitializationError().c_str());
		printf("OpenCL error is \"%s\" \n",BROCCOLI.GetOpenCLError());

        // Print create kernel errors
        int* createKernelErrors = BROCCOLI.GetOpenCLCreateKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createKernelErrors[i] != 0)
            {
                printf("Create kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(createKernelErrors[i]));
            }
        }                        
                
        printf("OpenCL initialization failed, aborting! \nSee buildInfo* for output of OpenCL compilation!\n");      
        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }
    // Initialization OK
    else
    {
        // Set all necessary pointers and values
        BROCCOLI.SetInputfMRIVolumes(h_fMRI_Volumes);
        
        BROCCOLI.SetEPIWidth(DATA_W);
        BROCCOLI.SetEPIHeight(DATA_H);
        BROCCOLI.SetEPIDepth(DATA_D);
        BROCCOLI.SetEPITimepoints(DATA_T);  
        BROCCOLI.SetEPITR(TR);  
        BROCCOLI.SetEPISliceOrder(SLICE_ORDER);  
		BROCCOLI.SetCustomSliceTimes(h_Custom_Slice_Times);
		BROCCOLI.SetCustomReferenceSlice(SLICE_CUSTOM_REF);
                                
        // Run the actual slice timing correction
		startTime = GetWallTime();        
		BROCCOLI.PerformSliceTimingCorrectionWrapper();        
		endTime = GetWallTime();

		if (VERBOS)
	 	{
			printf("\nIt took %f seconds to run the slice timing correction\n",(float)(endTime - startTime));
		}    

        // Print create buffer errors
        int* createBufferErrors = BROCCOLI.GetOpenCLCreateBufferErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createBufferErrors[i] != 0)
            {
                printf("Create buffer error %i is %s \n",i,BROCCOLI.GetOpenCLErrorMessage(createBufferErrors[i]));
            }
        }
        
        // Print create kernel errors
        int* createKernelErrors = BROCCOLI.GetOpenCLCreateKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createKernelErrors[i] != 0)
            {
                printf("Create kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(createKernelErrors[i]));
            }
        } 

        // Print run kernel errors
        int* runKernelErrors = BROCCOLI.GetOpenCLRunKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (runKernelErrors[i] != 0)
            {
                printf("Run kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(runKernelErrors[i]));
            }
        } 
    }
        
    // Write slice timing corrected data to file            
    startTime = GetWallTime();

	/*
    // Create new nifti image
	nifti_image *outputNifti = nifti_copy_nim_info(inputData);      
	allNiftiImages[numberOfNiftiImages] = outputNifti;
	numberOfNiftiImages++;   
     
    // Copy information from input data    	
	if (!CHANGE_OUTPUT_NAME)
	{
    	nifti_set_filenames(outputNifti, inputData->fname, 0, 1);    
	}
	else
	{
		nifti_set_filenames(outputNifti, outputFilename, 0, 1);    
	}
	*/

    WriteNifti(inputData,h_fMRI_Volumes,FILENAME_EXTENSION,ADD_FILENAME,DONT_CHECK_EXISTING_FILE);


	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to write the nifti file\n",(float)(endTime - startTime));
	}
    
    // Free all memory
    FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);            
    FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    
    return EXIT_SUCCESS;
}


