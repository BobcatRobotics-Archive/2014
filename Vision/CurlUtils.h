/*
 * CurlUtils Header File
 */

#include <curl/curl.h>
#include "CVHeader.h"

namespace CurlUtils {

/// Function declarations
size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);
Mat fetchImg(string url);

}

