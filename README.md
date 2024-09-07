# Babel Engine library

A C++ static library for assigning an address to an *arbitrary sequence of bytes* and being able to retrieve the original sequence from the address.  This library mimics the behavior of the original [*Library of Babel*](https://libraryofbabel.info/) website.

Instead of only text, this program can address and lookup any image, recording, text, or executable that is within the size of the data space.

## Usage

This library can be included within a project by either including the source files directly or by linking against the static library that this project generates.  Its API is defined in the `babel_engine.h` header file and exposes the `Babel` namespace with the following functions:

```cpp
// Get the decomposed components of an address
Babel::LibraryCoordinate Babel::getAddressComponents(const std::string &address);

// Assigns an address to a sequence of bytes
std::string Babel::computeAddress(const std::vector<unsigned char>& data, bool padRandom);

// Retrieve the original sequence of bytes from an address
std::vector<unsigned char> Babel::search(const std::string &address);
```

There are also streaming versions of the `computeAddress` and `search` functions that allow for the processing of continuous data streams.  These functions are defined as follows:

```cpp
// Assigns an address to a sequence of streamed bytes
std::string Babel::computeStreamAddress(std::istream& stream, bool padRandom);

// Retrieve the original sequence of streamed bytes from an address
void Babel::searchStream(const std::string &address, std::ostream &stream);
```

## Address Space

All addresses are encoded in standard base64.  Their length is fixed, but depends on the size of the input space (the maximum number of bytes in the input sequence that this library is compiled with).  These address can easily be store within strings and displayed.  Even very short addresses can reference a large byte sequence.

Addresses are composed similarly to those of the original *Library of Babel* website.  They are composed of the following components, delimited by colons:

* Hexagon room address
* Wall number
* Shelf number
* Volume number
* Page number

## Data Space

The data space is the set of all byte combinations that can exist within the size defined by `Babel::MAX_PAGE_LEN`.  Each address references a sequence of bytes that is exactly this length.

If an address references a sequence of bytes that is shorter than `Babel::MAX_PAGE_LEN`, the remaining bytes are filled with random data or zeroes, depending on the value of the `padRandom` parameter.