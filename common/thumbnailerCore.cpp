#include "thumbnailerCore.h"
#include <vector>

unsigned long findXMPSignature(std::ifstream &stream,
                               const unsigned long bufsize = 128) {

  // Allocate 3 buffers
  // There are 3 buffers used to prevent the rare edge case of the signature
  // being
  // only halfway in the buffer because it is placed at for example '0x126'
  // causing it
  // to be not completely inside
  // The first two buffers get read to and the searchbuffer is the one witht the
  // two combined
  //
  // Example:
  //    Searching the signature 'World'
  //    The signature gets read halfway into the end of buffer2
  //    This here is a representation of the searchbuffer
  //
  //  ['Hello Wo']
  //        ^ Beginning of buffer2
  // 'World' can't be recognized because only 'Wo' are inside the buffer
  //
  //  ['o World!']
  //        ^ Beginning of buffer1
  // After the second read the whole word 'World' is in the buffer and can be
  // recognized
  // It is done with 3 buffers instead of one to make the code less complex
  //
  // IMPORTANT: If the buffersize is smaller than the length of the signature it
  // can still fail
  std::string buffer1(bufsize, '\0');
  std::string buffer2(bufsize, '\0');
  std::string searchbuffer((bufsize + 1) * 2, '\0');

  bool flip = true;
  // Read until end of file or return
  while (!stream.eof()) {

    // Read into a buffer every other iteration
    if (flip) {
      // Read into buffer2
      stream.read(&buffer2[0], bufsize);
      // Set the searchbuffer to [b1, b2]
      searchbuffer = buffer1 + buffer2;
    } else {
      // Read into buffer 1
      stream.read(&buffer1[0], bufsize);
      // Set the searchbuffer to [b2, b1]
      searchbuffer = buffer2 + buffer1;
    }
    flip = !flip;

    // Check if the signature is somewhere in the searchbuffer
    long pos = (long)searchbuffer.find(XMP_SIG_MAGIC);
    if (pos != (long)std::string::npos) {
      // Calculate the signature's position in the file
      // Twice the bufsize is used because the searchbuffer is twice as large as
      // the other ones
      pos = stream.tellg() - (long)bufsize * 2 + pos;
      // Return to the position and return it
      stream.seekg(pos);
      return pos;
    }
  }
  // Return -1 if it isn't found
  return -1;
}

std::string removeXMPHeaders(const char *data) {
  std::regex pattern(XMP_HEADER_FOOTER_REGEX);
  return std::regex_replace(data, pattern, "");
}

// Find the imageData in an XMLDocument
std::string findImageData(tinyxml2::XMLDocument &xmp) {
  // Get The first Element in the /x:xmpmeta/rdf:RDF tag
  // This is done with the safe assumption that the XMP file has that hirarchy
  tinyxml2::XMLElement *currentElement =
      xmp.RootElement()->FirstChildElement()->FirstChildElement();
  do {
    // Try to find the tag containing the image-data
    if (!currentElement->NoChildren()) {
      // If it has a tag with this name it should take the first element of the
      // list it contains
      if (currentElement->FirstChildElement("xap:Thumbnails")) {
        tinyxml2::XMLElement *thumbnailsTag =
            currentElement->FirstChildElement("xap:Thumbnails");

        if (!thumbnailsTag->NoChildren()) {
          tinyxml2::XMLElement *thumbnailList =
              thumbnailsTag->FirstChildElement();

          if (!thumbnailList->NoChildren()) {
            tinyxml2::XMLElement *thumbnailTag =
                thumbnailList->FirstChildElement();

            if (thumbnailTag->FirstChildElement("xapGImg:image")) {
              // Decode the base64 image-data
              const char *base64Data =
                  thumbnailTag->FirstChildElement("xapGImg:image")->GetText();
              return base64_decode(base64Data);
            }
          }
        }
      }
    }
    // Go to the next element
    currentElement = currentElement->NextSiblingElement();
  } while (currentElement);
  return "";
}

// Get the thumbnail out of an XMP block at the current stream position
// with a size of 'size'
std::string readXMPBySize(std::ifstream &stream, long size) {
  std::vector<char> buffer(size + 1);
  stream.read(buffer.data(), size);
  stream.close();

  std::string xml = removeXMPHeaders(buffer.data());

  tinyxml2::XMLDocument xmp;

  xmp.Parse(xml.c_str());

  if (xmp.Error()) {
    std::cerr << "Error: " << xmp.ErrorName() << std::endl;
    return "";
  }
  return findImageData(xmp);
}

// Read an xmp sidecar file
std::string readXmpFromSidecar(std::string path) {

  // Check if the file ends with '.hdf5'
  if (path.rfind(HDF_FILE_ENDING) == path.length() - HDF_FILE_ENDING_LENGTH) {
    // If yes. Remove the file ending
    path = path.substr(0, path.length() - HDF_FILE_ENDING_LENGTH);
  }
  path += ".xmp";

  std::ifstream xmpFile(path, std::ios::in | std::ios::ate);
  if (!xmpFile.good()) {
    return "";
  }
  std::streamsize size = xmpFile.tellg();
  xmpFile.seekg(0);

  return readXMPBySize(xmpFile, size);
}

std::string readFromHdfFile(std::string path) {

  std::ifstream file(path, std::ifstream::binary);
  file.seekg(0, std::ios::beg);

  // Check if the file has xmp data
  uint32_t header;
  file.read(reinterpret_cast<char *>(&header), 4);
#ifndef WIN32
  header = ntohl(header);
#else
  header = _byteswap_ulong(header);
#endif

  if (header == MAGIC_HDF) {
    file.close();
    return "";
  }

  file.seekg(0, std::ios::beg);

  // Find the start signature
  unsigned long start = findXMPSignature(file);

  // Start can't be 0 because the other signature also needs to be there
  if (start == (unsigned long)-1 || start == (unsigned long)0) {
    return "";
  }

  // Move past the signature
  start += XMP_SIG_MAGIC_LENGTH;
  file.seekg(start);

  unsigned long end = findXMPSignature(file);

  if (start == (unsigned long)-1) {
    return "";
  }

  file.seekg(start);

  long size = end - start;

  return readXMPBySize(file, size);
}

std::string getThumbnail(std::string path) {
  std::string binaryData = readFromHdfFile(path);

  if (binaryData.length() == 0) {
    binaryData = readXmpFromSidecar(path);
  }
  return binaryData;
}
