#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "rest.h"


#ifdef WIN32
	#include <io.h>
	#include <fcntl.h>
	#define pipe(fds) _pipe(fds,4096, _O_BINARY) 
	#define SSL_CERT char SSL_CA_PATH[PATH_MAX]; getcwd(SSL_CA_PATH,PATH_MAX); strcat(SSL_CA_PATH,"\\ca-bundle.crt"); curl_easy_setopt(curl, CURLOPT_CAINFO, SSL_CA_PATH); //printf("SSL_CA_PATH = '%s'\n",SSL_CA_PATH);
#else
	#define SSL_CERT
//	#define SSL_CERT curl_easy_setopt(curl, CURLOPT_CAPATH, "./ca-bundle.crt");
#endif
pthread_t threads[256];

int rest_build_param(char** param, const char * name, const char* value){
	*param = (char*)malloc(strlen(name)+strlen(value) + 2);
	return sprintf(*param, "%s=%s", name, value);
}

char * rest_escape(char * url){
	size_t len = strlen(url);
	size_t diff = 0;
	size_t current = len;
	int i;
	char * escaped = (char*) malloc(len + 1);
	escaped[0] = '\0';
	for ( i = 0; i < len; i ++){
		if (len+diff +3> current){
			current += 4;
			escaped = (char*)realloc(escaped,current+1);
		}
		char c = url[i];
		switch (c){
		case ' ':
			diff += 2;
			strcat(escaped,"%20");
			break;
		default:
			escaped[i+diff]=c;
			escaped[i+diff+1] = '\0';
			break;
		}
	}
	return escaped;
}


void * run_curl(void* ptr){
	if (ptr != NULL){
		run_curl_args * args = (run_curl_args*) ptr;
		curl_easy_perform(args->curl);
		curl_easy_cleanup(args->curl);
		fclose(args->file);
		free(args->url);
	}
}
size_t ReadFileCB( void *contents, size_t size, size_t nmemb, void *userp){
	FILE * file = (FILE*) userp;
	if (file == NULL) file == stdin;
	return fread(contents,size,nmemb,file);
}

static size_t WriteFileCB(void * contents, size_t size, size_t nmemb, void * userp){
	FILE * file = (FILE*) userp;
	if (file == NULL) file == stdout;
	return fwrite(contents, size, nmemb, file);
}

static size_t WriteBufferCB(void *contents, size_t size, size_t nmemb, void *userp)
{ 
	if (userp!= NULL){
		size_t realsize = size * nmemb;
		buffer * data = (buffer*) userp;
		*data = buffer_append(*data,contents, realsize);
		return realsize;
	}
}

char * rest_build_url(char ** params, char* base){
	char * url; 
	int num_params;
	int i;
	int len = 0;
	for (i = 0; params[i] != NULL; i++){
		len += strlen(params[i]) + 1;
	}
	num_params = i;
	url = (char*) malloc(strlen(base) + len + 2);
	strcpy(url, base);
	strcat(url, "?");
	for ( i = 0; i < num_params; i ++){
		strcat(url, params[i]);
		if ( i< num_params-1)
			strcat(url, "&");
	}
	char *escaped = rest_escape(url);
	free(url);
	return escaped;
}

FILE * rest_get	(char ** params, char * url){
	char * full_url = rest_build_url(params, url);

	/* set up pipes for reading/writing*/
	int fd[2];
	if (pipe(fd)!=0){ // uh oh, we have something wrong
		printf("could not create pipe\n");
		exit(1);
	}
	FILE * read = fdopen(fd[0], "r");
	FILE * write = fdopen(fd[1], "w");
	CURL * curl = curl_easy_init();
	SSL_CERT
//	printf("url = %s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)write);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteFileCB);
	run_curl_args *args = malloc(sizeof(run_curl_args));
	args->curl = curl;
	args->file = write;
	args->url = full_url;
	pthread_create(&threads[fd[1]],NULL, run_curl, args);
	return read;
}

buffer rest_get_buffer (char ** params, char * url){
	char * full_url = rest_build_url(params, url);
	CURL * curl = curl_easy_init();
	SSL_CERT
	buffer data = buffer_init(data,0);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
//	printf("url = %s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK){
		printf("code = %d, data= %s\n",res,data.data);
	}
	curl_easy_cleanup(curl);
	free(full_url);
	return data;
}

buffer  rest_post (char ** params, char * url){
	CURL * curl = curl_easy_init();
	char * post = rest_build_url(params,""); post++;
	char * escaped_url = rest_escape(url);
	
	SSL_CERT
	buffer data = buffer_init(data,0);
	curl_easy_setopt(curl,CURLOPT_URL,escaped_url);
	int i = 0;
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		printf("res = &d, data = %s\n",res,data.data);
	curl_easy_cleanup(curl);
	free(--post);
	free(escaped_url);
	return data;
}

buffer rest_put_file (char** params, char* url, FILE * in){
	CURL * curl = curl_easy_init();
	buffer data = buffer_init(data,0);
	char * full_url = rest_build_url(params,url);
	free(full_url);
//	printf("%s\n",full_url);
	curl_easy_setopt(curl,CURLOPT_URL,full_url);
	curl_easy_setopt(curl,CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl,CURLOPT_READDATA,in);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&data);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,WriteBufferCB);
	curl_easy_setopt(curl,CURLOPT_READFUNCTION,ReadFileCB); // for windows
	CURLcode res = curl_easy_perform(curl);
	free(full_url);
	curl_easy_cleanup(curl);
	if (res != CURLE_OK){
		printf("res = %d, data = %.*s\n",res,data.size,data.data);
		data = buffer_free(data);
	}
	return data;
}
