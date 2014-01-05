/*
 * CurlUtils Source File
 *
 */

#include "CurlUtils.h"

namespace CurlUtils {

string data; //will hold the url's contents


/// Write callback function
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{ //callback must have this declaration
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer

    for (int c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb; //tell curl how many bytes we handled
}

Mat fetchImg(string url)
  {
    CURL* curl; //our curl object
    data.clear(); // Clear out old data (Very Important!)

    curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);

    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    vector<char> chardata (data.begin(), data.end());
    Mat img = imdecode(chardata, 1);

    return img;
   
   
  }

}
