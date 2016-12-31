/*
 * File:   Redirect.cpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * The MIT License
 *
 * Copyright 2016.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Redirect.hpp"

// Create Redirect specifying output stream

Redirect::Redirect(std::ostream& outStream) {
    this->outStream = &outStream;
}

// Create Redirect specifying output stream, output file and start the redirect

Redirect::Redirect(std::ostream& outStream, std::string outfileName, std::ios_base::openmode mode) {
    this->outStream = &outStream;
    this->change(outfileName, mode);
}

// Restore old output stream

Redirect::~Redirect() {
    restore();
}

// Change output for stream to file 

void Redirect::change(std::string outfileName, std::ios_base::openmode mode) {
    this->fileStream.reset(new std::ofstream{outfileName, mode});
    this->outBuffer = outStream->rdbuf();
    outStream->rdbuf((this->fileStream)->rdbuf());
}


// Restore old output stream

void Redirect::restore() {

    if (this->outBuffer) {
        outStream->rdbuf(this->outBuffer);
    }
    if (this->fileStream) {
        this->fileStream->close();
    }
    
}