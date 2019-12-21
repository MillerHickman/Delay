// Delay.c by Miller Hickman
// This program is adaoted from sf2.float.c
/* sf2float.c: 32-bit float PCM sound file converter
  by Ming-Lun Lee
   9/25/2019
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sndfile.h>
// Compile: gcc -o delay Delay.c -lsndfile

#define NFRAMES (1024)

enum {ARG_PROGNAME, ARG_INFILE,ARG_OUTFILE, ARG_NARGS}; // for command line arguments

int main (int argc, char * argv [])
{
    char * infilename;         // input file name
    char * outfilename;        // output file name
    SNDFILE * infile = NULL ;  // input sound file pointer
    SNDFILE * outfile = NULL;  // output sound file pointer
    SF_INFO   sfinfoIN;          // sound file info
    SF_INFO   sfinfoOUT;          // sound file info
    SF_FORMAT_INFO info;       // sound file format info
    long nsamples;             // number of samples per block
    long readcount;            // no. of samples read
    float * buffer = NULL;     // buffer pointer (a dynamic array of floats)
    int numRepeats = 0; 
    int delayTime;
    int milliTime = 0;
    int echo;
    double mix;
    
    if(argc != ARG_NARGS){
        printf("Delay Effect in C.\n");
        printf("insufficient arguments.\n");
        printf("Usage: %s infile outfile\n", argv[ARG_PROGNAME]);
            return 1;
    }
    
    infilename = argv[ARG_INFILE] ;   // infile name
    outfilename = argv[ARG_OUTFILE] ;  // outfile name
    
    // input validation
    if (strcmp (infilename, outfilename) == 0)
    {   printf ("Error : Input and output filenames are the same.\n") ;
        return 1 ;
    } ;
    
    memset(&sfinfoIN, 0, sizeof (sfinfoIN));  // clear sfinfo

    if ((infile = sf_open (infilename, SFM_READ, &sfinfoIN)) == NULL)
    {    printf ("Not able to open input file %s.\n", infilename) ;
        puts (sf_strerror (NULL)) ;
        return 1 ;
    } ;
    
    int preset;
    printf("You can use one of the following presets if yopu would like or you may enter your parameters manually: \n");
    printf("1. Double\n");
    printf("2. Slapback\n");
    printf("3. Straight\n");
    printf("4. Echo\n");
    printf("Enter the number of the preset you'd like or enter 0 to choose your own: ");
    while(scanf("%d", &preset) != 1 || preset < 0 || preset > 4) //Input validation for delay time
    {
        printf("Wrong input! Must be 0 and 5. Enter your choice again: ");
        while(getchar() != '\n'){}
        continue;
    }
    
    if(preset == 1){
        printf("You have chosen Double!\n");
        milliTime = 50;
        numRepeats = 1;
        mix = 0.6;
        echo = 0;
    }
    if(preset == 2){
        printf("You have chosen Slapback!\n");
        milliTime = 100;
        numRepeats = 2;
        mix = 1;
        echo = 0;
    }
    if(preset == 3){
        printf("You have chosen Straight!\n");
        milliTime = 400;
        numRepeats = 3;
        mix = 0.8;
        echo = 0;
    }
    if(preset == 4){
        printf("You have chosen Echo!\n");
        milliTime = 200;
        numRepeats = 4;
        mix = 0.8;
        echo = 1;
    }
    
    if(preset == 0){
        printf("Enter the length of the delay(milliseconds): ");
        while(scanf("%d", &milliTime) != 1 || milliTime <= 0 || milliTime > 2000) //Input validation for delay time
        {
            printf("Wrong input! Must be greater than 0! Max Input is 2000. Enter the delay length again: ");
            while(getchar() != '\n'){}
            continue;
        }
    
        printf("Enter the number of reapeats: ");
        while(scanf("%d", &numRepeats) != 1 || numRepeats <= 0 || numRepeats > 10) //Input validation for number of repeats
        {
            printf("Wrong input! Must be greater than 0! Max input is 10. Enter the number of repeats again: ");
            while(getchar() != '\n'){}
            continue;
        }
    
        printf("Enter the mix, 0 is fully dry, 1 is fully wet: ");
        while(scanf("%lf", &mix) != 1 || mix < 0 || mix > 1) //Input validation for number of repeats
        {
            printf("Wrong input! Must be between 0 and 1! Enter the number of mix again: ");
            while(getchar() != '\n'){}
            continue;
        }
    
        printf("If you would like echo enter 1, if not enter 0: ");
        while(scanf("%d", &echo) != 1 || (echo != 0 && echo != 1)) //Input validation for number of repeats
        {
            printf("Wrong input! Must be 0 or 1! Enter the echo again: ");
            while(getchar() != '\n'){}
            continue;
        }
    }
    
    sfinfoOUT.sections = sfinfoIN.sections;
    sfinfoOUT.frames = sfinfoIN.frames;
    sfinfoOUT.seekable = sfinfoIN.seekable;
    sfinfoOUT.samplerate = sfinfoIN.samplerate;
    sfinfoOUT.channels = 1; // set channel output to mono
    

    /* Return TRUE if fields of the SF_INFO struct are a valid combination of values.
    int  sf_format_check (const SF_INFO *info) ;*/
    if (!sf_format_check(&sfinfoIN))
    {
        sf_close(infile) ;
        printf ("Invalid encoding\n") ;
        return 1;
    }
    /* The SF_FORMAT_INFO struct is used to retrieve information about the sound file formats libsndfile supports using the sf_command () interface.
     typedef struct
     {    int            format ;
          const char    *name ;
          const char    *extension ;
     } SF_FORMAT_INFO ;
     */
     /* Return TRUE if fields of the SF_INFO struct are a valid combination of values.
      int sf_command (SNDFILE *sndfile, int command, void *data, int datasize) ;
      */
    // SFC_GET_FORMAT_MAJOR    Retrieve information about a major format type
    sf_command(NULL, SFC_GET_FORMAT_MAJOR, &info, sizeof(info)) ;
    /*
    printf("%s  (extension \"%s\")\n", info.name, info.extension);
    printf("sample rate = %d\n", sfinfo.samplerate);
    printf("number of channels = %d\n", sfinfo.channels);
    printf("infile format = %#x\n", sfinfo.format);
    printf("sections = %d\n", sfinfo.sections);
    printf("seekable = %d\n", sfinfo.seekable);
    */
    // dynamic memory allocation for a block of samples
    // no. of samples = (no. frames) * (no. channels)
    nsamples = sfinfoIN.channels * NFRAMES; // no. of samples per block
    buffer = (float *)malloc(nsamples * sizeof(float)); // used to save a block of samples
    
    //sfinfoOUT.format = SF_FORMAT_FLOAT | info.format; // output file format
    sfinfoOUT.format = sfinfoIN.format;
    /*
    printf("outfile format = %#x\n", sfinfo.format);
     */
    // open a sound file for writing
    if ((outfile = sf_open(outfilename, SFM_WRITE, &sfinfoOUT)) == NULL)
    {
        printf("Not able to open output file %s.\n", outfilename) ;
        puts(sf_strerror (NULL)) ;
        return 1 ;
    } ;
    
    /*
      typedef __int64  sf_count_t ;
      sf_count_t sf_read_float (SNDFILE *sndfile, float *ptr, sf_count_t items) ;
     
      sf_count_t  sf_write_float   (SNDFILE *sndfile, float *ptr, sf_count_t items) ;
       // items are number of samples = (number of frames) * (number of channels)
     */
    // processing here....
    
    delayTime = milliTime * sfinfoOUT.samplerate / 1000;
    
    float * circBuffs[numRepeats];
    int cbCounters[numRepeats];
    
    for(int i = 0; i < numRepeats; i++){
        circBuffs[i] = malloc(sizeof(float) * (i+1) * delayTime);
        cbCounters[i] = 0;
    }
    
    while ((readcount = sf_read_float(infile, buffer, nsamples)) > 0){
        float * newBuff = (float *)malloc(sizeof(float) * readcount);
        int newBuffCounter = 0;
        for(int i = 0; i < readcount; i++){
            newBuff[newBuffCounter] = 0.5 * buffer[i];
            for(int j = 0; j < numRepeats; j++){
                newBuff[newBuffCounter] += (mix / (j+2)) * circBuffs[j][cbCounters[j]];
                if(echo == 0){
                    circBuffs[j][cbCounters[j]] = buffer[i];
                }
                if(echo == 1){
                    circBuffs[j][cbCounters[j]] = newBuff[newBuffCounter];
                }
                cbCounters[j]++;
                if(cbCounters[j] == delayTime * (j+1)){
                    cbCounters[j] = 0;
                }
            }
            newBuffCounter++;
        }
        sf_write_float(outfile, newBuff, readcount);
    }
        
    float *finalBuff = (float*)malloc(sizeof(float) * numRepeats * delayTime);
    int finalBuffCounter = 0;
    while(cbCounters[numRepeats-1] < (numRepeats * delayTime - 1)){
        for(int j = 0; j < numRepeats; j++){
            finalBuff[finalBuffCounter] = (mix / (2*j)) * circBuffs[j][cbCounters[j]];
            circBuffs[j][cbCounters[j]] = 0;
            cbCounters[j]++;
            if(cbCounters[j] == delayTime * (j+1)){
                cbCounters[j] = 0;
            }
        }
        finalBuffCounter++;
    }
    sf_write_float(outfile, finalBuff, numRepeats * delayTime);
        
    sf_close (infile) ;   // close input sound file
    sf_close (outfile) ;  // close output sound file
    
    return 0 ;
}

