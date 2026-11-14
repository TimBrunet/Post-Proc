// Created by Timothee Brunet on 2025-10-30.
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
//#include "BMP.h"

using namespace std;

bool ReadInput(char *& memblock, char *& memblockA, char *& memblockB, char *& memblockF, streampos &size);
void WriteFile(char *& memblock, char *& memblockA, char *& memblockF, streampos size);
void ImageProc(int sets, char *& memblock, char *& memblockA, char *& memblockB, char *& memblockF, streampos size, int &thresh2);
double NormalRand(int input);

int main() {
    auto lang = "my post processing project";
    cout << "Hello and welcome to " << lang << "!\n";

    // ASK FOR LIKE 5-6 PostProc STEPS, IF A STEP ==0 THEN END PROGRAM
    /* Steps are chosen from numbered list:
    1) Blur
    2) BW threshold
    3) Posterize
    4) Sobel
    5) Bloom
    6) Dither
    [[
        7) Kuwahara
    ]]
    */
    int threshold = 0x80;
    char * memblock1;
    char * memblock2;
    char * memblock3;
    char * memblock4;
    streampos FileSize;
    if (ReadInput(memblock1,memblock2,memblock3,memblock4, FileSize)==true) {
        //Apply effects
        int effect = 0;
        int step = 0;
        for (step = 0; step < 10; step++) {
            cout<< "\nWhat is the desired effect?\n";
            cout << "Available effects are Blur (1), Threshold (2), Posterize (3), Sobel (4), Bloom (5), Dither (6), Kuwahara (7)\n";
            //cout << "Or enter 0 to end program and write file\n";
            cin >> effect;
            if (effect == 0) {
                cout<< "No effect selected, ending program.\n\n";
                break;
            }
            //cout<< "Processing desired effect...\n";
            ImageProc(effect,memblock1,memblock2,memblock3,memblock4, FileSize, threshold);
            cout<< "Processing done.\n";
        }
        //Write final image to file
        WriteFile(memblock1,memblock2,memblock4, FileSize);
    }
    //Close buffers
    delete[] memblock1;
    delete[] memblock2;
    delete[] memblock3;
    delete[] memblock4;

    return 0;
}

bool ReadInput(char *& memblock, char *& memblockA, char *& memblockB, char *& memblockF, streampos &size){
    cout << "enter file name:" << endl;
    string filename;
    cin >> filename;

    ifstream inFile ("../"+filename, ios::in|ios::binary|ios::ate);
    if (inFile.is_open()) {
        size = inFile.tellg();
        cout<< "File is of size: " << size << " bytes.\n";
        memblock = new char [size];
        memblockA = new char [size];
        memblockB = new char [size];
        memblockF = new char [size];
        inFile.seekg (0, ios::beg);
        inFile.read (memblock, size);
        inFile.close();

        int width, height;
        char W[4] = {memblock[18], memblock[19], memblock[20], memblock[21]};
        char H[4] = {memblock[22], memblock[23], memblock[24], memblock[25]};
        memcpy ( &width, W, sizeof(W));
        memcpy ( &height, H, sizeof(H));
        cout << "Width and height are: "<< width << ", and " << height << "\n";

        cout << "The entire file content is in memory.\n";
        return true;
    }
    cout << "Unable to open input image file";
    return false;
}

void WriteFile(char *& memblock, char *& memblockA, char *& memblockF, const streampos size) {
    cout << "Writing output image file" << endl;
    ofstream outFile ("../b.bmp", ios::binary);
    if (outFile.is_open()) {
        int hSize = 54;
        // TODO: add BMP image header reading to find info
        int width, height;
        char W[4] = {memblock[18], memblock[19],memblock[20], memblock[21]};
        char H[4] = {memblock[22], memblock[23],memblock[24], memblock[25]};
        memcpy ( &width, W, sizeof(W));
        memcpy ( &height, H, sizeof(H));

        for (int i = 0; i<hSize; i += 1) {memblockF[i] = memblock[i];}
        for (int i = hSize; i<size; i += 1) {memblockF[i] = memblockA[i];}
    }
    outFile.write(memblockF, size);
    cout << "Wrote to output image file" << endl;
    outFile.close();
}

void ImageProc(int sets, char *& memblock, char *& memblockA, char *& memblockB, char *& memblockF, const streampos size, int &thresh2) {
    //TODO: Turn into vars brought in from Main to save recalculation
    int hSize = 54;
    int width, height;
    char W[4] = {memblock[18], memblock[19], memblock[20], memblock[21]};
    char H[4] = {memblock[22], memblock[23], memblock[24], memblock[25]};
    memcpy ( &width, W, sizeof(W));
    memcpy ( &height, H, sizeof(H));
    int awidth = abs(width), aheight = abs(height);
    //cout << "Width and height are: "<< width << ", and " << height << "\n";

    //Blur
    if (sets == 1) {
        cout<< "Processing blur...\n";
        int blur = 0;
        cout<< "Please enter blur level:\n";
        cin >> blur;

        int Ravg = 0, Gavg = 0, Bavg = 0;
        int a = blur, b = int(pow(2*a+1,2));
        //cout << a << ", " << b << "\n";
        //cout << hSize << ", " << size << "\n";

        for (int i = hSize; i<int(size); i += 3) {
            Ravg = 0, Gavg = 0, Bavg = 0;
            //char noColor = 0x00;
            for (int c = -3*a; c<=3*a; c+=3) {
                for (int r = -3*a; r<=3*a; r+=3) {
                    Bavg += 0 + (unsigned char)memblock[i+c+r*awidth];
                    Gavg += 0 + (unsigned char)memblock[i+c+r*awidth+1];
                    Ravg += 0 + (unsigned char)memblock[i+c+r*awidth+2];
                    //if (i>0 && i<60) cout << 0+(unsigned char)memblock[i+c+r*awidth] << "\n";
                }
            }
            Bavg /= b;
            Gavg /= b;
            Ravg /= b;
            memblockA[i] = Bavg;//blue
            memblockA[i+1] = Gavg;//green
            memblockA[i+2] = Ravg;//red
            //if (i%10==0) cout << i << ": " << Bavg << ", " << Gavg << ", " << Ravg << "\n";
        }
        //cout << endl;

    }

    //Threshold
    if (sets == 2) {
        cout<< "Processing threshold...\n";
        char zero = 0x00, full = 0xFF, mid = 0xA0;
        int thresh = 255/2, type =0;
        cout << "What type of threshold is desired:" << endl;
        cout << "B&W (1), Red (2), Green (3), Blue (4)" << endl;
        cin >> type;
        cout << "What is the desired threshold?"<< endl;
        cin >> thresh;

        if (type == 1) {//BW
            for (int i = hSize; i<size; i += 3) {
                if ((unsigned char)memblock[i]+(unsigned char)memblock[i+1]+(unsigned char)memblock[i+2] > 3*thresh) {
                    memblockA[i] = full;//blue
                    memblockA[i+1] = full;//green
                    memblockA[i+2] = full;//red
                }
                else {
                    memblockA[i] = zero;//blue
                    memblockA[i+1] = zero;//green
                    memblockA[i+2] = zero;//red
                }
            }
        }

        if (type == 2) {//Red
            for (int i = hSize; i<size; i += 3) {
                if ((unsigned char)memblock[i+2] < thresh) {
                    memblockA[i] = 40;
                    memblockA[i+1] = 20;
                    memblockA[i+2] = mid;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }
        }

        if (type == 3) {//Green
            for (int i = hSize; i<size; i += 3) {
                if ((unsigned char)memblock[i+1] < thresh) {
                    memblockA[i] = zero;
                    memblockA[i+1] = 70;
                    memblockA[i+2] = 30;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }
        }

        if (type == 4) {//Blue
            for (int i = hSize; i<size; i += 3) {
                if ((unsigned char)memblock[i] < thresh) {
                    memblockA[i] = mid;
                    memblockA[i+1] = zero;
                    memblockA[i+2] = zero;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }
        }

        if (type == 5) {//Custom TODO: add custom thresholds (up/down, threshold RGB, result RGB)
            for (int i = hSize; i<size; i += 3) {
                if ((unsigned char)memblock[i] < thresh) {
                    memblockA[i] = mid;
                    memblockA[i+1] = zero;
                    memblockA[i+2] = zero;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }
        }
    }

    //Posterize
    if (sets == 3) {
        cout<< "Processing posterization...\n";
        int post = 0;
        cout << "What is the desired number of bits for each subpixel?"<< endl;
        cin >> post;
        int a = 0xFF/post;
        int B, G, R;
        for (int i = hSize; i<size; i += 3) {
            B = a*(((unsigned char)memblock[i])/a);//blue
            G = a*(((unsigned char)memblock[i+1])/a);//green
            R = a*(((unsigned char)memblock[i+2])/a);//red

            if (B>255) B=255;
            if (G>255) G=255;
            if (R>255) R=255;
            //std::clamp (B, 0, 255); CLAMP is not recognized?

            memblockA[i] = B;//blue
            memblockA[i+1] = G;//green
            memblockA[i+2] = R;//red

        }
    }

    //Sobel
    if (sets == 4) {
        cout<< "Processing Sobel operation...\n";
        int R = 0, G = 0, B = 0;
        int BW = 0;
        int V[9] = {
            -1, 0, 1,
            -2, 0, 2,
            -1, 0, 1
        };
        int H2[9] = {
            1, 2, 1,
            0, 0, 0,
            -1, -2, -1
        };

        cout << "Do you want to apply the Sobel (1), only Sobel (2), save for later (3), or apply saved Sobel (4)?" << endl;
        int choice;
        cin >> choice;

        if (choice != 4) {
            cout<< "What threshold should be used? (recommended value: 128 recommended)" << endl;
            cin >> thresh2;
        }
        else cout << "current threshold is: " << thresh2 << endl;

        for (int i = hSize; i<size; i += 3) {
            R = 0, G = 0, B = 0, BW = 0;
            for (int r = -3; r<=3; r+=3) {
                for (int c = -3; c<=3; c+=3) {
                    B += 0+(H2[(c+r/3+4)]+V[(c+r/3+4)])*(0+(unsigned char)memblock[i+c+r*awidth]);
                    G += 0+(H2[(c+r/3+4)]+V[(c+r/3+4)])*(0+(unsigned char)memblock[i+c+r*awidth+1]);
                    R += 0+(H2[(c+r/3+4)]+V[(c+r/3+4)])*(0+(unsigned char)memblock[i+c+r*awidth+2]);
                }
            }
            /*if (B<0x80) B=0x00;
            if (G<0x80) G=0x00;
            if (R<0x80) R=0x00;
            if (B>0x81) B=0xFF;
            if (G>0x81) G=0xFF;
            if (R>0x81) R=0xFF;*/

            BW = B+G+R;
            BW /= 3;
            if (BW > 255) BW = 255;
            if (BW < 0) BW = 0;

            if (choice == 1) {//apply sobel on top
                if (BW > thresh2) {
                    memblockA[i] = 0x18;
                    memblockA[i+1] = 0x18;
                    memblockA[i+2] = 0x18;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }

            else if (choice == 2) {//only sobel
                if (BW > 0 ) {//always true
                    memblockA[i] = BW;//blue
                    memblockA[i+1] = BW;//green
                    memblockA[i+2] = BW;//red
                }
                else {
                    memblockA[i] = 0x00;//blue
                    memblockA[i+1] = 0x00;//green
                    memblockA[i+2] = 0x00;//red
                }

            }

            else if (choice == 3) {//save for later
                memblockB[i] = BW;//blue
                memblockB[i+1] = BW;//green
                memblockB[i+2] = BW;//red

                memblockA[i] = memblock[i];// so A is not empty
                memblockA[i+1] = memblock[i+1];
                memblockA[i+2] = memblock[i+2];
            }

            else if (choice == 4) {//use saved sobel
                if ((unsigned char)memblockB[i] > thresh2) {
                    memblockA[i] = 0x18;
                    memblockA[i+1] = 0x18;
                    memblockA[i+2] = 0x18;
                }
                else {
                    memblockA[i] = memblock[i];
                    memblockA[i+1] = memblock[i+1];
                    memblockA[i+2] = memblock[i+2];
                }
            }
        }
    }

    //Bloom
    if (sets == 5) {
        cout<< "Processing bloom...\n";
        int bloom = 0;
        cout<< "Please enter bloom level:\n";
        cin >> bloom;

        int Ravg = 0, Gavg = 0, Bavg = 0;
        int Rf = 0, Gf = 0, Bf = 0;
        int a = bloom, b = int(pow(2*a+1,2));
        int* highlights = new int[size];
        int thresh = 220*3;

        for (int i = hSize; i<size; i += 3) {
            if ((unsigned char)memblock[i]+(unsigned char)memblock[i+1]+(unsigned char)memblock[i+2] > thresh) {
                memblockB[i] = 0 + (unsigned char)memblock[i];
                memblockB[i+1] = 0 + (unsigned char)memblock[i+1];
                memblockB[i+2] = 0 + (unsigned char)memblock[i+2];
                //cout << highlights[i] << " ";
            }
            else {
                memblockB[i] = 0x00;
                memblockB[i+1] = 0x00;
                memblockB[i+2] = 0x00;
            }
        }

        for (int i = hSize; i<int(size); i += 3) {
            Ravg = 0, Gavg = 0, Bavg = 0;
            for (int c = -3*a; c<=3*a; c+=3) {
                for (int r = -3*a; r<=3*a; r+=3) {
                    Bavg += 0 + (unsigned char)memblockB[i+c+r*awidth];
                    Gavg += 0 + (unsigned char)memblockB[i+c+r*awidth+1];
                    Ravg += 0 + (unsigned char)memblockB[i+c+r*awidth+2];
                }
            }
            Bavg /= b;
            Gavg /= b;
            Ravg /= b;

            Bf = Bavg + (unsigned char)memblock[i];
            Gf = Gavg + (unsigned char)memblock[i+1];
            Rf = Ravg + (unsigned char)memblock[i+2];
            if (Bf > 255) Bf = 255;
            if (Gf > 255) Gf = 255;
            if (Rf > 255) Rf = 255;
            if (Bf < 0) Bf = 0;
            if (Gf < 0) Gf = 0;
            if (Rf < 0) Rf = 0; //TEMP

            memblockA[i] = Bf;//blue
            memblockA[i+1] = Gf;//green
            memblockA[i+2] = Rf;//red

        }

        delete[] highlights;
        highlights = nullptr;
    }

    //Dither
    if (sets == 6) {
        cout<< "Processing dithering operation...\n";
        int R = 0, G = 0, B = 0;
        const int n=4;
        int Bayer[n*n] = {
            0, 8, 2, 10,
            12, 4, 14, 6,
            3, 11, 1, 9,
            15, 7, 13, 5
        };
        int b = n*n;

        cout << "How many bits per subpixel does your image have?" << endl;
        int bits = 32;
        cin >> bits;

        int loc=0;
        for (int r = 0; r < aheight; r++) {
            for (int c = 0; c < awidth; c++) {
                loc = 3*(c+r*awidth)+hSize;

                B = 0+(unsigned char)memblock[loc+0]+(float)(255.0/bits*Bayer[c%n+(r%n)*n]/b-1.0/2);
                G = 0+(unsigned char)memblock[loc+1]+(float)(255.0/bits*Bayer[c%n+(r%n)*n]/b-1.0/2);
                R = 0+(unsigned char)memblock[loc+2]+(float)(255.0/bits*Bayer[c%n+(r%n)*n]/b-1.0/2);

                //if (loc < 13000000 && loc > 13000000-50) cout << (255.0/bits*Bayer[c%n+(r%n)*n]/b) << ": " << B << " ";
                if (B>255) B = 255;
                if (G>255) G = 255;
                if (R>255) R = 255;
                /*if (B<0) B = 0;
                if (G<0) G = 0;
                if (R<0) R = 0;*/

                memblockA[loc+0] = B;
                memblockA[loc+1] = G;
                memblockA[loc+2] = R;

            }
        }



    }

    //Kuwahara
    if (sets == 7) {
        cout<< "Processing Kuwahara...\n";
        int kernel = 1;
        cout<< "Please enter kernel size:\n";
        cin >> kernel;

        int a = kernel, b = int(pow(2*a+1,2));

        for (int n = hSize; n<int(size); n += 3) {// Convert to HSV
            double cmax=0.0, cmin=0.0, diff=0.0, Hc=-1.0, Sc=-1.0, Vc=-1.0;
            double R= 0.0 + (unsigned char)memblock[n],
                G= 0.0 + (unsigned char)memblock[n+1],
                B= 0.0 + (unsigned char)memblock[n+2];
            R = R/255.0; G = G/255.0; B = B/255.0;
            cmax = max(R, max(G, B));
            cmin = min(R, min(G, B));
            diff = cmax-cmin;

            //Hue
            /*if (cmax == cmin) Hc = 0;
            else if (cmax == R) Hc = fmod(60.0 * ((G - B) / diff) + 360.0, 360.0);// True B?
            else if (cmax == G) Hc = fmod(60.0 * ((B - R) / diff) + 120.0, 360.0);// works
            else if (cmax == B) Hc = fmod(60.0 * ((R - G) / diff) + 240.0, 360.0);// True R?
            else cout << "      cmax error" << endl;
            if (Hc < 0) Hc = Hc + 360.0;
            else if (Hc > 360) Hc = Hc - 360.0;*/

            //Saturation
            //if (cmax == 0) Sc = 0;
            //else Sc = (diff / cmax) * 100.0;

            //Value
            Vc = cmax * 100.0;

            //memblockB[n] = Hc;//blue
            //memblockB[n+1] = (Sc);//green
            memblockB[n+2] = (Vc);//red
        }
        cout << "Done converting to HSV"<<endl;

        for (int n = hSize; n<int(size); n += 3) {// Find avr and std dev per pixel to find pixel color
            double Vavg[4]={0}, Vdev[4]={0}, Vx=0, Vdf=255;//TODO: add progress % / bar
            int If = 0;

            for (int i = 0; i<4; i++ ) {//Calculate each quadrant
                int u,v;
                if (i==0) {u = -3*a; v = 3*a;}
                else if (i==1) {u = 3*a; v = 3*a;}
                else if (i==2) {u = 3*a; v = -3*a;}
                else if (i==3) {u = -3*a; v = -3*a;}
                Vavg[i] = 0;
                Vdev[i] = 0;

                for (int c = -3*a; c<=3*a; c+=3) { //Find the mean per quadrant
                    for (int r = -3*a; r<=3*a; r+=3) {
                        Vavg[i] += 0.0 + (unsigned char)memblockB[n+(c+u)+(r+v)*awidth+2];
                    }
                }
                Vavg[i] = Vavg[i]/b;
                for (int c = -3*a; c<=3*a; c+=3) { //Find the std dev per quadrant
                    for (int r = -3*a; r<=3*a; r+=3) {
                        Vx = 0.0 + (unsigned char)memblockB[n+(c+u)+(r+v)*awidth+2];
                        Vdev[i] += pow(Vx-Vavg[i],2);
                    }
                }
                Vdev[i] = sqrt(Vdev[i]/(b-1));// TODO: I dont think I need this part as I only use Vdev to be compared against each other, as such proportinnal editing is useless
                if (Vdev[i] < Vdf) {Vdf = Vdev[i]; If = i;}
            }

            memblockB[n+1] = int(If);
            if ( (fmod(n*100/double(size),10.0)) < 300/double(size) ) {// TODO: probs uses more compute than worth? but id rather slow than no progress indicator
                cout << "Progress is : " << int(n*100/double(size)) << "%, at n =" << n <<endl;
            }
        }
        cout << "Done calculating Kuwahara quadrant values"<<endl;

        for (int n = hSize; n<int(size); n += 3) {// TODO: I think I could combine this with the n for loop above
            double Ravg[4]={0}, Gavg[4]={0}, Bavg[4]={0};

            int i = memblockB[n+1];
            int u,v;
            if (i==0) {u = -3*a; v = 3*a;}
            else if (i==1) {u = 3*a; v = 3*a;}
            else if (i==2) {u = 3*a; v = -3*a;}
            else if (i==3) {u = -3*a; v = -3*a;}

            for (int c = -3*a; c<=3*a; c+=3) { //Find the mean per quadrant
                for (int r = -3*a; r<=3*a; r+=3) {
                    Ravg[i] += 0.0 + (unsigned char)memblock[n+(c+u)+(r+v)*awidth+0];
                    Gavg[i] += 0.0 + (unsigned char)memblock[n+(c+u)+(r+v)*awidth+1];
                    Bavg[i] += 0.0 + (unsigned char)memblock[n+(c+u)+(r+v)*awidth+2];
                }
            }
            Ravg[i] /= b;
            Gavg[i] /= b;
            Bavg[i] /= b;

            memblockA[n+0] = int(Ravg[i]);//Using chosen quadrant value stored in memblockB[n+1]
            memblockA[n+1] = int(Gavg[i]);
            memblockA[n+2] = int(Bavg[i]);

        }
        cout << "Done calculating RGB averages and finding final values"<<endl;

    }

    //Pixel Sorting // TODO: implement sorting
    if (sets == 8) {
        //input stuff
        cout << "sorting pixels... " << endl;

        for (int n = hSize; n<int(size); n += 3) {// Convert to HSV
            double cmax=0.0, cmin=0.0, diff=0.0, Hc=-1.0, Sc=-1.0, Vc=-1.0;
            double R= 0.0 + (unsigned char)memblock[n+2],
                G= 0.0 + (unsigned char)memblock[n+1],
                B= 0.0 + (unsigned char)memblock[n+0];
            R = R/255.0; G = G/255.0; B = B/255.0;
            cmax = max(R, max(G, B));
            cmin = min(R, min(G, B));
            diff = cmax-cmin;

            //Hue
            if (cmax == cmin) Hc = 0;
            else if (cmax == R) {Hc = 60.0 * fmod( (G-B)/diff + 6.0, 6.0);}
            else if (cmax == G) Hc = 60.0 * ( (B-R)/diff + 2.0 );
            else if (cmax == B) Hc = abs(60.0 * ( (R-G)/diff + 4.0 ));
            //else if (cmax == R) Hc = fmod(60.0 * ((G - B) / diff) + 360.0, 360.0);
            //else if (cmax == G) Hc = fmod(60.0 * ((B - R) / diff) + 120.0, 360.0);
            //else if (cmax == B) Hc = fmod(60.0 * ((R - G) / diff) + 240.0, 360.0);
            else cout << "      cmax error" << endl;
            //if ( (R+G+B)*255.0 < 600) cout << " Hc = " << Hc;// << endl;
            if (Hc < 0) Hc = Hc + 360.0;
            else if (Hc > 360) Hc = Hc - 360.0;

            //Saturation
            if (cmax == 0) Sc = 0;
            else Sc = (diff / cmax) * 100.0;

            //Value
            Vc = cmax * 100.0;

            memblockA[n+0] = Hc;//blue
            memblockA[n+1] = Sc;//green
            memblockA[n+2] = Vc;//red
        }
        cout << "Done converting to HSV"<<endl;

        char * harray;
        char * trackray;
        harray  = new char [aheight];
        trackray = new char [aheight];
        bool sorted = false;
        int swaph, swaptrack, count;
        int Ht, St, Vt;

        for (int w = 0; w < awidth; w++) {//Horizontal
            for (int h = 0; h<aheight; h++) {//Vertical
                harray[h] = memblockA[hSize + 3*(w + h*awidth)+2];//get values
                trackray[h] = h;
            }
            while (sorted == false) {
                count = 0;
                for (int h = 0; h<aheight-1; h++) {
                    if (harray[h] > harray[h+1]) {
                        swaph = 0 + harray[h];
                        harray[h] = harray[h+1];
                        harray[h+1] = swaph;

                        swaptrack = 0 + trackray[h];
                        trackray[h] = trackray[h+1];
                        trackray[h+1] = swaptrack;

                    }
                    else count++;
                }
                if (count == aheight-1) sorted = true;
            }

            for (int h = 0; h<aheight; h++) {
                memblockB[hSize + 3*(w + h*awidth)+0] = memblockA[hSize + 3*(w + (0+trackray[h])*awidth)+0];
                memblockB[hSize + 3*(w + h*awidth)+1] = memblockA[hSize + 3*(w + (0+trackray[h])*awidth)+1];
                memblockB[hSize + 3*(w + h*awidth)+2] = memblockA[hSize + 3*(w + (0+trackray[h])*awidth)+2];

            }

        }

        delete[] harray;
        delete[] trackray;

        for (int n = hSize; n<int(size); n += 3) {// Convert to back to RGB
            int R, G, B;
            double R1=0.0, G1=0.0, B1=0.0;
            double Ht, St, Vt, C, X, m;
            Ht  = memblockA[n+0];
            St = memblockA[n+1]/100.0;
            Vt = memblockA[n+2]/100.0;

            C = Vt * St;
            X = C * (1.0 - abs(fmod(Ht/60.0,2.0)-1.0) );
            m = Vt - C;

            if (0<= Ht && Ht < 60) {
                R1 = C;
                G1 = X;
                B1 = 0;
            }
            else if (60 <= Ht && Ht < 120) {
                R1 = X;
                G1 = C;
                B1 = 0;
            }
            else if (120 <= Ht && Ht < 180) {
                R1 = 0;
                G1 = C;
                B1 = X;
            }
            else if (180 <= Ht && Ht < 240) {
                R1 = 0;
                G1 = X;
                B1 = C;
            }
            else if (240 <= Ht && Ht < 300) {
                R1 = X;
                G1 = 0;
                B1 = C;
            }
            else if (300 <= Ht && Ht <= 360) {
                R1 = C;
                G1 = 0;
                B1 = X;
            }

            R = (R1+m)*255.0;
            G = (G1+m)*255.0;
            B = (B1+m)*255.0;

            memblockA[n+2] = R;
            memblockA[n+1] = G;
            memblockA[n+0] = B;
        }

    }

    //Noise
    if (sets == 9) {
        cout<< "Adding Noise...\n";
        int seed = 0;
        cout<< "Please enter Noise seed:\n";
        cin >> seed;
        srand(seed);
        // RAND_MAX = 2 147 483 647;
        int factor = 50;
        cout<< "Please enter Noise Amplitude:\n";
        cin >> factor;
        int R = 0, G = 0, B = 0;

        for (int i = hSize; i<int(size); i += 3) {
            B = 0 + NormalRand(rand()) * factor + (unsigned char)memblock[i+0];
            G = 0 + NormalRand(rand())*factor + (unsigned char)memblock[i+1];
            R = 0 + NormalRand(rand())*factor + (unsigned char)memblock[i+2];
            if (B>255) B = 255;
            else if (B<0) B = 0;
            if (G>255) G = 255;
            else if (G<0) G = 0;
            if (R>255) R = 255;
            else if (R<0) R = 0;

            memblockA[i+0] = B;//blue
            memblockA[i+1] = G;//green
            memblockA[i+2] = R;//red
            //cout << NormalRand(rand())*factor << endl;
        }
    }

    //End program
    for (int i = hSize; i<size; i += 1) {memblock[i] = memblockA[i];} // so next operation is built on top of previous
}

double NormalRand(int input) {
    double i = double(input) /  RAND_MAX;
    i -= 0.5;
    return i;
}