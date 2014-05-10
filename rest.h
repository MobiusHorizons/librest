#ifndef __LIBREST_H__
#define __LIBREST_H__
#ifdef WIN32
#	include "curl/include/curl/curl.h"	
#else
#	include <curl/curl.h>
#endif
#include <json-c/json.h>
#include <stdbool.h>
#include "buffer.h"

typedef struct {
	CURL * curl;
	FILE * file;
	char * url;
} run_curl_args;

int rest_build_param(char** param, const char * name, const char* value);
char * rest_build_url(char ** params, char* base);
char * rest_escape(char * url);
void * run_curl(void* ptr);
size_t ReadFileCB( void *contents, size_t size, size_t nmemb, void *userp);
static size_t WriteFileCB(void * contents, size_t size, size_t nmemb, void * userp);
static size_t WriteBufferCB(void *contents, size_t size, size_t nmemb, void *userp);
FILE * rest_get	(char ** params, char * url);
buffer rest_get_buffer (char ** params, char * url);
buffer rest_post (char ** params, char * url);
buffer rest_put_file (char** params, char* url, FILE * in);
#endif
