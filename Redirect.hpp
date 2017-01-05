/*
 * File:   Redirect.hpp
 * 
 * Author: Robert Tizzard
 *
 * Created on October 24, 2016, 2:34 PM
 *
 * Copyright 2016.
 *
 */

#ifndef REDIRECT_HPP
#define REDIRECT_HPP

// STL definitions

#include <iostream>
#include <memory>
#include <fstream>

class Redirect {
    
public:

    Redirect(std::ostream& outStream);
    Redirect(std::ostream& outStream, std::string outfileName, std::ios_base::openmode mode = std::ios_base::out);
    ~Redirect();

    void change(std::string outfileName, std::ios_base::openmode mode = std::ios_base::out);
    void restore(void);

private:

    Redirect() = delete;
    Redirect(const Redirect & orig) = delete;
    Redirect(const Redirect && orig) = delete;

    std::unique_ptr<std::ofstream> fileStream = nullptr;
    std::ostream *outStream;
    std::streambuf *outBuffer;

};
#endif /* REDIRECT_HPP */

