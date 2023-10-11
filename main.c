#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

static char *help_message = "Image Processing in C. \
\nFile format supported : PNG.\n\
Arguments:\n\
\t - k : Method to use.\
\n\t - f : filename.\n";

int main(int argc, char *argv[])
{

  // png image header size.
  // const int PNG_HEADER_SIZE = 8;

  // Max image size : 16mb.
  const int MAX_SIZE = 16 * 1024 * 1024;

  // for profiling.
  clock_t startTime, endTime;
  startTime = clock();

  int option;
  char *filename = NULL;

  // parsing command line arguments.
  while ((option = getopt(argc, argv, "hkf:")) != -1)
  {
    switch (option)
    {
    case 'h':
      printf(help_message);
      return 0;

    case 'k':
      printf("k was entered.\n");
      break;

    case 'f':
      filename = optarg;
      printf("FILENAME : %s\n", filename);
      break;

    default:
      fprintf(stderr, "Could not parse these command line arguments.\n");
      break;
    }
  }

  char *imageBuffer = (char *)malloc(MAX_SIZE);
  if (!imageBuffer)
  {
    fprintf(stderr, "Couldn't allocate memory.\n");
    return 1;
  }

  // opening png file.
  FILE *fileptr = fopen(filename, "rb");

  // checking if file opened or not.
  if (!fileptr)
  {
    fprintf(stderr, "Could not open this file, make sure the image is in the same directory as this executable.");
    free(imageBuffer);
    return 1;
  }
  // int size = fread(imageBuffer, 1, MAX_SIZE, fileptr);

  // unsigned char header[PNG_HEADER_SIZE];
  // for (size_t i = 0; i < PNG_HEADER_SIZE; i++)
  // {
  //   printf("%02X ", header[i]);
  //   printf("\n");
  // }

  // closes file.
  fclose(fileptr);

  // end time.
  endTime = clock();
  printf("\nTotal duration it took: %lfms\n", ((double)(endTime - startTime) * 1000.0) / CLOCKS_PER_SEC);

  return 0;
}