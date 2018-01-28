#include <iostream>
#include <string>
#include <curl/curl.h>

size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size*nmemb;
    size_t oldLength = s->size();
    try
    {
        s->resize(oldLength + newLength);
    }
    catch(std::bad_alloc &e)
    {
        //handle memory problem
        return 0;
    }

    std::copy((char*)contents,(char*)contents+newLength,s->begin()+oldLength);
    return size*nmemb;
}

int main (void)
{
	CURL *curl;
	CURLcode res;
	
	// init
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl) {
		std::string s;
		// Setup the url and set it to check for items with tags dummyItems
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/rest/items?tags=dummyItems&recursive=false");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output
		// Do the call
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		
		 std::cout<<std::endl<<s<<std::endl;
	}
	curl_global_cleanup();

	return 0;
}