#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 * Name: Lucas Ryan
 * PID: LU469191
 * Date: 9/14/2016
 * File: sobel.c
 */

# define MASK_SIZE 3

// given image will save results in outImage arrays
void convolution(int** image, int numRows, int numCols);
void gradient(int** gradientX, int** gradientY, int numRows, int numCols);
void fileWriteDouble(double** gradient, char* fileName, int numRows, int numCols, int threshold);
void fileWriteInt(int** gradient, char* fileName, int numRows, int numCols, int threshold);


const int MASK_X [MASK_SIZE][MASK_SIZE] = {{-1,  0,  1},
                                           {-2,  0,  2},
                                           {-1,  0,  1}};

const int MASK_Y [MASK_SIZE][MASK_SIZE] = {{ 1,  2,  1},
                                           { 0,  0,  0},
                                           {-1, -2, -1}};

// original image read#include <math.h> in
int** image;
// filter on X-axis
int** outImageX;
// filter on Y-axis
int** outImageY;
// gradient values = sqrt(x^2 + y^2)
double** gradientValueImage;
double maxGradientValue;

struct pgmInfo{
    int imageWidth;
    int imageHeight;
    int imageMaxValue;
};

// will store the image data info
struct pgmInfo imageData;
/*
 * Sobel
 * : sobel.exe <image file_name>.pgm <low threshold> <high threshold>
*/
int main(int argc, char** argv) {
    int i;
    char throwaway [80];
    FILE* imageFile;

    imageFile = fopen(argv[1], "rb");
    int lo_threshold = atoi(argv[2]); // save low threshold
    int hi_threshold = atoi(argv[3]); // save hi threshold


    fgets(throwaway, 80, imageFile);
    fgets(throwaway, 80, imageFile);
    fgets(throwaway, 80, imageFile);
    if ( !( (throwaway[0]=='2') && (throwaway[1]=='5') && (throwaway[2]=='5')))
        fgets(throwaway, 80, imageFile);

    imageData.imageHeight = 256;
    imageData.imageWidth = 256;
    imageData.imageMaxValue = 255;

    // setup array to store images, zero out memory and do zero padding
    image = calloc(imageData.imageHeight + (2 * MASK_SIZE), sizeof(int*));
    outImageX = calloc(imageData.imageHeight + (2 * MASK_SIZE), sizeof(int*));
    outImageY = calloc(imageData.imageHeight + (2 * MASK_SIZE), sizeof(int*));
    gradientValueImage = calloc(imageData.imageHeight + (2 * MASK_SIZE), sizeof(double*));

    for(i = 0; i < imageData.imageHeight + (2 * MASK_SIZE); i++) {
        image[i] = calloc(imageData.imageWidth + (2 * MASK_SIZE), sizeof(int));
        outImageX[i] = calloc(imageData.imageWidth + (2 * MASK_SIZE), sizeof(int));
        outImageY[i] = calloc(imageData.imageWidth + (2 * MASK_SIZE), sizeof(int));
        gradientValueImage[i] = calloc(imageData.imageWidth + (2 * MASK_SIZE), sizeof(double));
    }

    // read in the file
    for(i = MASK_SIZE; i < imageData.imageHeight + MASK_SIZE; i++) {
        int j;
        for(j = MASK_SIZE; j < imageData.imageWidth + MASK_SIZE; j++) {
            image[i][j] = getc(imageFile) & 0x0377;
        }
    }

    fclose(imageFile);

    // do convolution, does horizontal and vertical simultaneously
    convolution(image, imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE));

    // go gradient matrix using x and y axis gradient matrices
    gradient(outImageX, outImageY, imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE));

    // write to file
    fileWriteInt(outImageX, "result-x-filter.pgm", imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE), 0);
    fileWriteInt(outImageY, "result-y-filter.pgm", imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE), 0);
    fileWriteDouble(gradientValueImage, "filter-final-noThreshold.pgm", imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE), 0);
    fileWriteDouble(gradientValueImage, "filter-final-lowThreshold.pgm", imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE), lo_threshold);
    fileWriteDouble(gradientValueImage, "filter-final-hiThreshold.pgm", imageData.imageHeight + (2 * MASK_SIZE), imageData.imageWidth + (2 * MASK_SIZE), hi_threshold);



    //free up memory
    for(i = 0; i < imageData.imageHeight + (2 * MASK_SIZE); i++) {
        free(image[i]);
        free(outImageX[i]);
        free(outImageY[i]);
        free(gradientValueImage[i]);
    }

    free(image);
    free(outImageY);
    free(outImageX);
    free(gradientValueImage);

    return 0;
}

void convolution(int** image, int numRows, int numCols) {
    // look at for loop in sobel in notepade++ for code
    int row, col;
    int horizontalSum;
    int verticalSum;
    int conv_row;
    int conv_col;
    int mr = 1;

    // iterates through image array
    for(row = mr; row < numRows - mr; row++) {
        for(col = mr; col < numCols - mr; col++) {
            horizontalSum = 0;
            verticalSum = 0;

            // iterate through MASK arrays to do convolution
            for(conv_row = -mr; conv_row <= mr; conv_row++) {
                for(conv_col = -mr; conv_col <= mr; conv_col++) {
                    horizontalSum += image[row + conv_row][col + conv_col] * MASK_X[conv_row + mr][conv_col + mr]; // get weighted sum
                    verticalSum += image[row + conv_row][col + conv_col] * MASK_Y[conv_row + mr][conv_col + mr];
                }
            }

            // save weighted sums
            outImageX[row][col] = horizontalSum;
            outImageY[row][col] = verticalSum;
        }
    }
}

void gradient(int** gradientX, int** gradientY, int numRows, int numCols) {
    int row, col;
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            gradientValueImage[row][col] = sqrt((double)((gradientX[row][col]*gradientX[row][col])
                                                + (gradientY[row][col]*gradientY[row][col])));
            maxGradientValue = (maxGradientValue < gradientValueImage[row][col]) ?
                                    gradientValueImage[row][col] : maxGradientValue;
        }
    }

}

void fileWriteDouble(double** gradient, char* fileName, int numRows, int numCols, int threshold) {
    // file to store the image in
    FILE* imgFile;
    imgFile = fopen(fileName, "wb");

    // first need to write the 4 lines
    fprintf(imgFile,"%s", "P5\n");
    fprintf(imgFile,"%s", "# CREATOR: XV Version 3.10a  Rev: 12/29/94\n");
    fprintf(imgFile,"%d %d\n%d\n", numRows, numCols, 255);

    // write to file
    int row, col;
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            int temp = (threshold < gradient[row][col]) ? 255 : 0;;
            if(threshold == 0) {
                temp = (int) ((gradient[row][col] / maxGradientValue) * 255);
            }
            fprintf(imgFile,"%c",(char)(temp));
        }
    }

    // close file
    fclose(imgFile);
}

void fileWriteInt(int** gradient, char* fileName, int numRows, int numCols, int threshold) {
    // file to store the image in
    FILE* imgFile;
    imgFile = fopen(fileName, "wb");

    // first need to write the 4 lines
    fprintf(imgFile,"%s", "P5\n");
    fprintf(imgFile,"%s", "# CREATOR: XV Version 3.10a  Rev: 12/29/94\n");
    fprintf(imgFile,"%d %d\n%d\n", numRows, numCols, 255);

    // write to file
    int row, col;
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            //gradient[row][col] = (gradient[row][col] / maxGradientValue) * 255;
            int temp = (threshold <= gradient[row][col]) ? 255 : 0;
            fprintf(imgFile,"%c",(char)((temp)));
        }
    }

    // close file
    fclose(imgFile);
}
