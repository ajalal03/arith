#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "compress40.h"

static void (*compress_or_decompress)(FILE *input) = compress40;

int main(int argc, char *argv[])
{
        FILE *fp1;
        FILE *fp2;
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-c") == 0) {
                        compress_or_decompress = compress40;
                } else if (strcmp(argv[i], "-d") == 0) {
                        compress_or_decompress = decompress40;
                } else if (*argv[i] == '-') {
                    
                    fp1 = fopen(stdin,"r");
                    
                        // fprintf(stderr, "%s: unknown option '%s'\n",
                        //         argv[0], argv[i]);
                        // exit(1);
                } else if (argc - i > 2) {
                        fprintf(stderr, "Usage: %s -d [filename]\n"
                                "       %s -c [filename]\n",
                                argv[0], argv[0]);
                        exit(1);
                } else {
                        break;
                }
        }
        assert(argc - i <= 1);    /* at most one file on command line */
        if (i < argc) {
            if (fp1 == NULL) {
                FILE *fp1 = fopen(argv[i], "r");
                Pnm_ppm img = Pnm_ppmread(fp1, methods)
                --i;
            }
                FILE *fp2 = fopen(argv[++i],"r");
                Pnm_ppm img2 = Pnm_ppmread(fp2, methods);
                
                assert(fp1 != NULL);
                assert(fp2 != NULL);
                assert(img1->height - img2->height <= 1 && img1->height - img2->height >= -1);
                assert(img1->width - img2->width <= 1 && img1->width - img2->width >= -1);
                
                int total = 0;
                for (int i = 0; i < img->height; i++ ) {
                    for int j = 0; j < img->width; j++) {
                        
                        total += img1->methods-> at(img1->pixels,i,j)->red;
                    }
                }
                
        } 
        // else {
        //         compress_or_decompress(stdin);
        // }

        return EXIT_SUCCESS; 
}