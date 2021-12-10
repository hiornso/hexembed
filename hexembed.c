/*
 * hexembed - a simple utility to help embed files in C programs
 *
 * Copyright (c) 2010-2018 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgement in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */

/*
 * Modified by hiornso 2021-12-09 to add
 * - arguments to specify infile, outfile and variable name
 * - make it stream rather than cache in buffer
 * - have an `alignment` global var to allow easy setting of alignment of data
 */

#include <stdio.h>
#include <string.h>

const int alignment = 4;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n\thexembed <filename> [-o <outfile>] [-n <variable name>]\n"
               "By default, varname='file' and outfile is stdout.\n");
        return -1;
    }

    const char *infile = argv[1];
    const char *varname = "file";
    const char *outfile = NULL;
    for(int i = 2; i < argc; ++i){
        if (strcmp(argv[i], "-o") == 0) {
            i++;
            if (argc < i+1) {
                fprintf(stderr, "No outfile specified, assuming stdout.\n");
            } else {
                outfile = argv[i];
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            i++;
            if (argc < i+1) {
                fprintf(stderr, "No variable name specified, assuming 'file'.\n");
            } else {
                varname = argv[i];
            }
        } else {
            fprintf(stderr, "Unexpected argument '%s'\n", argv[i]);
        }
    }
    
    FILE *ofp;
    if (outfile == NULL) {
        ofp = stdout;
    } else {
        ofp = fopen(outfile, "wb");
        if (!ofp) {
            fprintf(stderr, "Error opening file: %s.\n", infile);
            return -1;
        }
    }
    
    FILE *fp = fopen(infile, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s.\n", infile);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    const int fsize = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    
    fprintf(ofp, "/* Embedded file: %s */\n", infile);
    fprintf(ofp, "const int %s_size = %i;\n", varname, fsize);
    fprintf(ofp, "__attribute__((aligned(%i))) const unsigned char %s[] = {\n\t", alignment, varname);

    for (int i = 0; i < fsize; ++i) {
        fprintf(ofp, "0x%02x%s",
                fgetc(fp),
                i == fsize-1 ? "" : ((i+1) % 16 == 0 ? ",\n\t" : ","));
    }

    fprintf(ofp, "\n};\n");

    fclose(fp);
    return 0;
}
