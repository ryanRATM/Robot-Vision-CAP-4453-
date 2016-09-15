#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// Name: Lucas Ryan
// PID: LU469191
// Date: 9/14/2016
// File: canny.c

// the code for canny algorithm
void print(double** matrix, int rows, int cols);
double** getMemory(int rows, int cols);
void freeMemory(double** mem, int rows, int cols);
void printMatrix(char* title, double** matrix, int rows, int cols, int padding);
void printImageMatrix(char* title, int** matrix, int rows, int cols, int padding);
void writeFileInt(int** image, char* fileName, int numRows, int numCols);
void writeFileDouble(double** image, char* fileName, int numRows, int numCols);


/*
 * Expect: > canny.exe <image file name>.pgm <sigma> <percent>
 * argv[1] : stores image filename
 * argv[2] : stores sigma user wants for gGaussian filter
 * Process:
 *        (1) PART 1 : compute Gaussian filter
 *        (2) PART 2 : compute magnitude
 *        (3) PART 4 : compute HIGH & LOW thresholds
 *        (4) PART 3 : finish filtering out noise, and connecting/continuing edges
 */
int main(int argc, char** argv) {

    // dynamic sized images
    int**    image;                         // stores data read from file
    double** gaussian;                      // sized based on sigma
    double** mask_x;
    double** mask_y;
    double** image_x_gaus;                  // image (*) mask_x
    double** image_y_gaus;                  // image (*) mask_y
    double** image_mag_gaus;                // sqrt(image_x_gaus^2 + image_y_gaus^2)
    int**    filtered_peaks;                // peaks/edges in the filtered image
    int**    final_image;
    double   sigma;                         // input, used for calculating Gaussian mask
    double   maxMagVal = 0;
    double   percent;                       // input, used for finding thresholds
    int      hi, low;                       // thresholds used for final filtering
    int      numRows, numCols,              // dimensions of the image
             gaussianWidth,                 // width of Gaussian masks
             gaussianCenter,                // middle index for Gaussian mask
             imageMaxValue,                 // max possible value in image
             row, col,                      // will be used to iterate through matrix
             i;                             // used for when need to do single for loop
    char throwaway[80];                     // to read from file

    FILE* imgFile = fopen(argv[1], "rb");   //<file name>.pgm
    sigma = atof(argv[2]);                  // sigma
    percent = atof(argv[3]) / 100;

    // read image info from file
        fgets(throwaway, 80, imgFile);
        fgets(throwaway, 80, imgFile);
        fgets(throwaway, 80, imgFile);
if ( !( (throwaway[0]=='2') && (throwaway[1]=='5') && (throwaway[2]=='5')))
        fgets(throwaway, 80, imgFile);

    numRows = 256;
    numCols = 256;
    imageMaxValue = 255;

    // generate the size of matrices
    gaussianWidth = 1 + (6 * sigma);
    gaussianCenter = (gaussianWidth / 2);
//    numRows = numRows + (2 * gaussianWidth);    // do for padding
//    numCols = numCols + (2 * gaussianWidth);    // do for padding

    // borrow the memory need, clear out the memory
    image = calloc(numRows, sizeof(int*));
    final_image = calloc(numRows, sizeof(int*));
    filtered_peaks = calloc(numRows, sizeof(int*));
    for(i = 0; i < numRows; i++) {
        image[i] = calloc(numCols, sizeof(int));
        final_image[i] = calloc(numCols, sizeof(int));
        filtered_peaks[i] = calloc(numCols, sizeof(int));
    }
    gaussian = getMemory(gaussianWidth, gaussianWidth);
    mask_x = getMemory(gaussianWidth, gaussianWidth);
    mask_y = getMemory(gaussianWidth, gaussianWidth);
    image_x_gaus = getMemory(numRows, numCols);
    image_y_gaus = getMemory(numRows, numCols);
    image_mag_gaus = getMemory(numRows, numCols);



    // read in input from file
    for(i = 0; i < numRows; i++) {
        int j;
        for(j = 0; j < numCols; j++) {
            image[i][j] = getc(imgFile);
            image[i][j] = image[i][j] & 0377;
        }
    }
    fclose(imgFile);


// PART 1: GENERATE & APPLY GAUSSIAN MASK, VIA X & Y COMPONENT, & COMPUTE MAGNITUDE
    for(row = -gaussianCenter; row <= gaussianCenter; row++) {
        double val = 0;
        for(col = -gaussianCenter; col <= gaussianCenter; col++) {
            val = exp(-((col * col) + (row * row)) / (2 * sigma * sigma));
            gaussian[row + gaussianCenter][col + gaussianCenter] = val;
            mask_x[row + gaussianCenter][col + gaussianCenter] = col * val;
            mask_y[row + gaussianCenter][col + gaussianCenter] = row * val;
        }
    }
    // convolution between image and Gaussian components
    for(row = gaussianCenter; row < numRows - gaussianCenter; row++) {
        for(col = gaussianCenter; col < numCols - gaussianCenter; col++) {
            double verticalSum = 0, horizontalSum = 0;
            int row_c, col_c;
            for(row_c = -gaussianCenter; row_c <= gaussianCenter; row_c++) {
                for(col_c = -gaussianCenter; col_c <= gaussianCenter; col_c++) {
                    verticalSum += (((double)(image[row + row_c][col + col_c])) * mask_y[row_c + gaussianCenter] [col_c + gaussianCenter]);
                    horizontalSum += (((double)(image[row + row_c][col + col_c])) * mask_x[row_c + gaussianCenter] [col_c + gaussianCenter]);
                }
            }
            image_x_gaus[row][col] = horizontalSum;
            image_y_gaus[row][col] = verticalSum;
        }
    }

    // compute magnitude
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            image_mag_gaus[row][col] = sqrt((image_x_gaus[row][col] * image_x_gaus[row][col]) + (image_y_gaus[row][col] * image_y_gaus[row][col]));
            if(maxMagVal < image_mag_gaus[row][col]) {
                maxMagVal = image_mag_gaus[row][col];
            }
        }
    }

    // then scale out the magnitude image, so nothing is above, 255
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            image_mag_gaus[row][col] = (image_mag_gaus[row][col] / maxMagVal) * 255;
        }
    }



// PART 2: RUN MAX TEST, mark pixel as a peak, if max between neighbors
    for(row = gaussianCenter; row < numRows - gaussianCenter; row++) {
        for(col = gaussianCenter; col < numCols - gaussianCenter; col++) {

            // don't want to divide by zero
            if(image_x_gaus[row][col] == 0.0) {
               image_x_gaus[row][col] = .00001;
            }

            double slope = image_y_gaus[row][col] / image_x_gaus[row][col];

            if((-.4142 < slope) && (slope <= .4142)) {
                if((image_mag_gaus[row][col - 1] < image_mag_gaus[row][col]) && (image_mag_gaus[row][col + 1] < image_mag_gaus[row][col])) {
                    filtered_peaks[row][col] = 255;
                }
            } else if((.4142 < slope) && (slope <= 2.4142)) {
                if((image_mag_gaus[row - 1][col - 1] < image_mag_gaus[row][col]) && (image_mag_gaus[row + 1][col + 1] < image_mag_gaus[row][col])) {
                    filtered_peaks[row][col] = 255;
                }
            } else if((-2.4142 < slope) && (slope <= -.4142)) {
                if((image_mag_gaus[row + 1][col - 1] < image_mag_gaus[row][col]) && (image_mag_gaus[row - 1][col + 1] < image_mag_gaus[row][col])) {
                    filtered_peaks[row][col] = 255;
                }
            } else {
                if((image_mag_gaus[row - 1][col] < image_mag_gaus[row][col]) && (image_mag_gaus[row + 1][col] < image_mag_gaus[row][col])) {
                    filtered_peaks[row][col] = 255;
                }
            }

        }
    }
writeFileInt(filtered_peaks, "filtered_peaks.pgm", numRows, numCols);


// PART 4: CALCULATE LOW AND HIGH THESHOLD VALUES
    // create histogram
    int* histogram = calloc(256, sizeof(int)); // represent values 0 -> 255, zeros out indexes
    // first generate the histogram
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            histogram[(int)(image_mag_gaus[row][col])]++;
        }
    }


    // start count, continue until we have marked enough highest edges
    int cutOff = percent * (numRows) * (numCols);
    int areaOfTops = 0;
    for(row = 255; (0 < row) && (areaOfTops <= cutOff); row--) {
        areaOfTops += histogram[row];
    }

    // calculate hi and low to use
    hi = row; // represents high bound for edges
    low = (int)(.35 * hi);
    printf("high threshold: %d; low threshold: %d\n", hi, low);
//PART 3: DOUBLE THRESHOLDING, FINISH JOINING EDGES
    // first we do a run through, checking which pixels we are sure are and aren't edges
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            // only check if considered an edge
            if(filtered_peaks[row][col] == 255) {
                if(hi <= image_mag_gaus[row][col]) { // guaranteed an edge
                    filtered_peaks[row][col] = 0;   // OFF
                    final_image[row][col] = 255;    // ON
                } else if(image_mag_gaus[row][col] < low) { // guaranteed NOT an edge
                    filtered_peaks[row][col] = 0;   // OFF
                    final_image[row][col] = 0;      // OFF
                }
            }
        }
    }

    // now time to search through all the pixels, that were not guaranteed an EDGE
    int moreToDo = 1;
    while(moreToDo == 1) {
        moreToDo = 0;   // assume done checking, to prevent infinite loop
        for(row = 0; row < numRows; row++) {
            for(col = 0; col < numCols; col++) {
                // if hit an edge
                if(filtered_peaks[row][col] == 255) {
                    // check if neighbors are an edge
                    for(i = -1; i <= 1; i++) {
                        int j;
                        for(j = -1; j <= 1; j++) {
                            // if neighbor is high, then have to check it's neighbors
                            if(filtered_peaks[row + i][col + j] == 255) {
                                filtered_peaks[row][col] = 0;   // OFF, so stop looking here
                                final_image[row][col] = 255;    // ON, we were an edge!!!
                                moreToDo = 1;            // continue the hunt
                            }
                        }
                    }
                }
            }
        }
    }

writeFileInt(final_image, "final_image.pgm", numRows, numCols);
writeFileDouble(image_mag_gaus, "image_mag_gaussian.pgm", numRows, numCols);
writeFileDouble(image_x_gaus, "image_x_gaussian.pgm", numRows, numCols);
writeFileDouble(image_y_gaus, "image_y_gaussian.pgm", numRows, numCols);

    // free up memory borrowed
    for(i = 0; i < numRows; i++) {
        free(image[i]);
        free(final_image[i]);
        free(filtered_peaks[i]);
    }
    free(image);
    free(histogram);
    free(final_image);
    free(filtered_peaks);
    freeMemory(gaussian, gaussianWidth, gaussianWidth);
    freeMemory(mask_x, gaussianWidth, gaussianWidth);
    freeMemory(mask_y, gaussianWidth, gaussianWidth);
    freeMemory(image_x_gaus, numRows, numCols);
    freeMemory(image_y_gaus, numRows, numCols);
    freeMemory(image_mag_gaus, numRows, numCols);

    return 0;
}

double** getMemory(int rows, int cols) {
    double** tempMem = calloc(rows, sizeof(double*));
    int i;
    for(i = 0; i < rows; i++) {
        tempMem[i] = calloc(cols, sizeof(double));
    }
    return tempMem;
}

void freeMemory(double** mem, int rows, int cols) {
    int i;
    for(i = 0; i < rows; i++) {
        free(mem[i]);
    }
    free(mem);
}

void printMatrix(char* title, double** matrix, int rows, int cols, int padding) {
    int i, j;
    printf("\n\n*************| %s |*************\n\n", title);
    for(i = padding; i < rows - padding; i++) {
        for(j = padding; j < cols - padding; j++) {
            printf("| %10lf ", matrix[i][j]);
        }
        printf("\n");
    }
}

void printImageMatrix(char* title, int** matrix, int rows, int cols, int padding) {
    int i, j;
    printf("\n\n*************| %s |*************\n\n", title);
    for(i = padding; i < rows - padding; i++) {
        for(j = padding; j < cols - padding; j++) {
            printf("| %10d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void writeFileInt(int** image, char* fileName, int numRows, int numCols) {
    FILE* imgFile = fopen(fileName, "wb");
    int row, col;

    fprintf(imgFile, "%s\n%s\n","P5", "# CREATOR: XV Version 3.10a  Rev: 12/29/94");
    fprintf(imgFile, "%d %d\n%d\n", numRows, numCols, 255);
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            fprintf(imgFile, "%c", (char) (image[row][col]));
        }
    }
    fclose(imgFile);
}

void writeFileDouble(double** image, char* fileName, int numRows, int numCols) {
    FILE* imgFile = fopen(fileName, "wb");
    int row, col;

    fprintf(imgFile, "%s\n%s\n","P5", "# CREATOR: XV Version 3.10a  Rev: 12/29/94");
    fprintf(imgFile, "%d %d\n%d\n", numRows, numCols, 255);
    for(row = 0; row < numRows; row++) {
        for(col = 0; col < numCols; col++) {
            fprintf(imgFile, "%c", (char)((int) (image[row][col])));
        }
    }
    fclose(imgFile);
}
