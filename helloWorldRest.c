#include <iostream>
#include <string>
#include <time.h>
#include <stdlib.h>     // atof
#include <curl/curl.h>
#include "../jsmn/jsmn.h"

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

struct openhabItem {
	std::string name;
	float state;  // assumes the item is a number
	openhabItem *next;
}; 

int main (void)
{
	CURL *curl;
	CURLcode res;

	
	
	// init
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl) {
		int i;
		int r;
		int itemEnd;
		struct openhabItem *itemList, *currentItem;
		struct curl_slist *list = NULL;
		char dataBuffer[256];
		std::string s;
		jsmn_parser p;
		jsmntok_t t[128]; /* We expect no more than 128 tokens */
		jsmn_init(&p);
	
		
		// Read out items with tag dummyItems by calling the REST api
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/rest/items?tags=dummyItems&recursive=false");
		list = curl_slist_append(list, "Accept: application/json");		// we're expecting json
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output
		// Do the call
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		/* always cleanup */ 
		curl_easy_cleanup(curl);
		
		// Tokenize
		s = s.substr(1, s.length()-2); // curl (or openhab?) adds [] to start and end of string, remove those
		std::cout << s << std::endl;
		r = jsmn_parse(&p, s.c_str(), s.length(), t, sizeof(t)/sizeof(t[0]));
 		if (r < 0) {
			printf("Failed to parse JSON: %d\n", r);
		return 1;
		}
		/* Assume the top-level element is an object */
		if (r < 1 || t[0].type != JSMN_OBJECT) {
			printf("Object expected\n");
			return 1;
		}
		itemList = new struct openhabItem;
		currentItem = itemList;
		currentItem->next = NULL;
		itemEnd = t[0].end;
		for (i=0; i < r; ++i) {
			if (t[i].start > itemEnd) { // is there a new item?
				currentItem->next = new struct openhabItem;
				currentItem = currentItem->next;
				currentItem->next = NULL;
				itemEnd = t[i].end;
			}
			if (s.substr(t[i].start, t[i].end-t[i].start).compare("state") == 0) {
				if (s.substr(t[i+1].start, t[i+1].end-t[i+1].start).compare("NULL") == 0)
					currentItem->state = 0;  // set to 0 if NULL
				else
					currentItem->state =  atof(s.substr(t[i+1].start, t[i+1].end-t[i+1].start).c_str()) + clock(); // Add number clicks since program start to the current state
			} else if (s.substr(t[i].start, t[i].end-t[i].start).compare("name") == 0) {
				currentItem->name.assign(s.substr(t[i+1].start, t[i+1].end-t[i+1].start));
			}
		}
		std::cout << "Found these items:" << std::endl;
		currentItem = itemList;
		while (currentItem) {
			std::cout << currentItem->name << ": " << currentItem->state << std::endl;
			currentItem = currentItem->next;				
		}
		

		// Print all the items and state values and update it on openhab via REST	
		curl_slist_free_all(list);
		list = NULL;
		curl = curl_easy_init();
		if (curl) {			
			list = curl_slist_append(list, "Content-Type: text/plain");
			list = curl_slist_append(list, "Accept: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); /* !!! */
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); //remove this to disable verbose output
			
		std::cout << "Update items:" << std::endl;
		currentItem = itemList;
		while (currentItem) {
			sprintf(dataBuffer,"http://localhost:8080/rest/items/%s/state", currentItem->name.c_str());
			curl_easy_setopt(curl, CURLOPT_URL, dataBuffer);  
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, sprintf(dataBuffer,"%f", currentItem->state));
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, dataBuffer); /* data goes here */			
			res = curl_easy_perform(curl);
			std::cout << "res: " << res << std::endl;
			currentItem = currentItem->next;
		}
		
		
		// item cleanup
		currentItem = itemList;
		while (currentItem) {
			itemList = currentItem->next;
			delete currentItem;
			currentItem = itemList;
		}
		
		/* always cleanup */ 
		curl_easy_cleanup(curl);	
		}
	}
	curl_global_cleanup();

	return 0;
}