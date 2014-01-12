
#include "VisionServer.h"
size_t writeCallback(char *buf, size_t size, size_t nmemb, void *up);

// Get Image Thread
void *GetImagesThread(void *arg)
{
    Mat img_in;
    CURL *curl;                 // our curl object
#ifdef SIM_CAM
    string url = "file://image2.bmp";

#else                           /*  */
    string url = "http://10.1.77.11/axis-cgi/jpg/image.cgi";

#endif                          /*  */
    string data;                // will hold the url's contents
    char error_buf[CURL_ERROR_SIZE];
    CURLcode result;

#ifdef DEBUG
    int clk_tck = sysconf(_SC_CLK_TCK);
    struct tms t;

#endif                          /*  */
    clock_t begin, end;

    cout << "Starting Get Image Thread" << endl;
    //curl_global_init(CURL_GLOBAL_ALL);  // pretty obvious
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT);   // Timeout after a second 
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
    while (1) {

#ifdef DEBUG
        begin = times(&t);

#endif                          /*  */
        data.clear();           // Clear out old data (Very Important!)
        result = curl_easy_perform(curl);
        if (result == CURLE_OK) {
            vector < char >chardata(data.begin(), data.end());

            // Get Mutex
            pthread_mutex_lock(&img_mutex);
            img_buf = imdecode(chardata, 1);
            image_ready = 1;

            // Signal the other thread that there is data available
            pthread_cond_broadcast(&image_ready_cond);

            // Release Mutex
            pthread_mutex_unlock(&img_mutex);
        } else {

            // todo - do we need to reset the curl options in this case?
            cout << "libCurl error: " << result << " - " << error_buf << endl;
        }

#ifdef SIM_CAM
        //Delay to simulate camera timing
        usleep(100000);

#else                           /*  */
#ifdef DEBUG
        end = times(&t);
        //cout << "GetImage Time: " << double (end - begin) / clk_tck << endl;

#endif                          /*  */
#endif                          /*  */
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

/// Write callback function
size_t writeCallback(char *buf, size_t size, size_t nmemb, void *up)
{                               //callback must have this declaration
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer
    string *data = (string *) up;
    for (int c = 0; c < size * nmemb; c++) {
        data->push_back(buf[c]);
    } return size * nmemb;      //tell curl how many bytes we handled
}
