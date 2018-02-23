// steg.cpp
// author: Cheri Walker-Owens
// Bug & error fixes written by Paul Talaga
// uses LodePNG version 20170917 Copyright (c) 2005-2017 Lode Vandevenne
// date: February 16, 2018
// TO COMPILE: g++ -o steg steg.cpp
// TO RUN: ./steg

#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>
#include "lodepng.cpp"

using namespace std;

void usage();
void help();
void getSize(char*[]);
void embedFile(char*[]);
void getFile(char*[]);

int main(int argc, char *argv[]){
    if(argc == 1){
        usage();  
    }
    
    else if(argc == 2 && string(argv[1]) == "-h"){
        help();
    }
    
    else if(argc == 3 && string(argv[1]) == "-s"){
        getSize(argv);
    }
    
    else if(argc == 3 && string(argv[1]) == "-g"){
        getFile(argv);
}
    
    else if(argc == 4 && string(argv[1]) == "-e"){
        embedFile(argv);
    }
    
    else{
        usage();
    }
    
    return 0;
}

void usage(){
    cout << "Steganography program using LSB insertion. All image files must be PNG." << endl;
    cout << "For a full list of commands, type ./steg -h." << endl;
    cout << "\nUSAGE: ./steg -e <filename.png> <filename.txt> OR ./steg -g <filename.png>" << endl;
}

void help(){
    cout << "\nMANUAL:" << endl;
    cout << "-s: -size  Calculates approx. number of bytes that can be stored in the file. ./steg -s <filename.png>" << endl;
    cout << "-e: -embed Embeds the message file in the PNG file. Saves a separate output file. ./steg -e <filename.png> <filename.txt>" << endl;
    cout << "-g: -get   Reads the message embedded in the file and saves to a separate output file. ./steg -g <filename.png>" << endl;
    cout << "\n Pulls LSB from every byte in the image, alternating RGB values." << endl;
}

void getSize(char *argv[]){
// Opening PNG file
    unsigned width, height;
    vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, argv[2]);
    if(error)cout << "Error! " << lodepng_error_text(error) << endl;
    
// Calculating approximate number of bytes that can be stored in the image
    int length = width * height / 8;
    
    cout << "Approximately " << length << " bytes can be stored in this image." << endl;
}

void embedFile(char *argv[]){

    string output_filename = "embedded-" + string(argv[2]);
   
// Opening and reading PNG file, uses lodepng.cpp & lodepng.h
    unsigned width, height;
    vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, argv[2]);
    if(error)cout << "Error! " << lodepng_error_text(error) << endl;
    
    
// Reading message file & storing file size in the beginning of the message
    // Modified version of code taken from http://www.cplusplus.com/doc/tutorial/files/
    string input_file = string(argv[3]);
    streampos size;
    char *memblock;
    ifstream file (input_file.c_str(), ios::in|ios::binary|ios::ate);
    if (file.is_open()){
        size = file.tellg();
        memblock = new char [int(size) + 4];
        file.seekg (0, ios::beg);
        file.read (memblock + 4, size);
        file.close();
    }
    
    int filesize = int(size);
    // PGT changed this
    char* message = memblock;
    memcpy((void*)memblock, &filesize, 4);
    filesize = filesize + 4; // To account for the int on the beginning.
    
    
// Embedding message file in PNG file
    int image_pos = 0;
    int length = width * height / 8;
    unsigned c = 0; // color, either 0,1,2
    
    if(filesize > length){
        cout << argv[3] << " is too large to be embedded into " << argv[2] << "." << endl;
        exit(1);
    }
    
    for(int i = 0; i < length && i < filesize; i++){
            for(int b = 0; b < 8; b++){
                unsigned x = image_pos % width;
                unsigned y = image_pos / width;
                image[(y * width + x) * 4 + (image_pos % 3)] = (image[(y * width + x) * 4 + (image_pos % 3)] & 0xFE) | (message[i] >> 7) & 0x1;
                image_pos++;
                message[i] = message[i] << 1;
            }
    }
    
    
// Writing new PNG file with embedded message file
    error = lodepng::encode(output_filename.c_str(), image, width, height);
    if(error)cout << "Error! " << lodepng_error_text(error) << endl;
    
    delete[] memblock;
}

void getFile(char *argv[]){
// Opening PNG file
    unsigned width, height;
    vector<unsigned char> image;
    unsigned error = lodepng::decode(image, width, height, argv[2]);
    if(error)cout << "Error! " << lodepng_error_text(error) << endl;
    

// Un-embedding message
    // Reads LSB from entire PNG
    int image_pos = 0;
    int length = width * height / 8;
    unsigned c = 0; // color, either 0,1,2
    char* out_text = new char[length];
    
    for(int i = 0; i < length; i++){
    	char o = 0;
    	for(int b = 0; b < 8; b++){
    		unsigned x = image_pos % width;
            unsigned y = image_pos / width;
    		o = o << 1;
    	    o = (o & 0xFE) | ((image[(y * width + x) * 4 + (image_pos % 3)]) & 1);
    		image_pos++;
    	}
    	out_text[i] = o;
    }
    
    // Reads length of message stored at the beginning of message and writes message_length # of bytes to file
    int message_length;
    memcpy(&message_length, (void*)out_text, 4);
    ofstream decryptedMessage;
    decryptedMessage.open("decryptedMessage.txt", ios::binary);
    //decryptedMessage << out_text;
    decryptedMessage.write(out_text + 4, message_length);
    decryptedMessage.close();
    
    delete[] out_text;
}
