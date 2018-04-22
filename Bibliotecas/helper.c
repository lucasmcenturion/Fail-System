#include "helper.h"


char* integer_to_string(char*string,int x) {
	string = malloc(10);
	if (string) {
		sprintf(string, "%d", x);
	}
	string=realloc(string,strlen(string)+1);
	return string; // caller is expected to invoke free() on this buffer to release memory
}

size_t getFileSize(const char* filename) {
    struct stat st;
    if(stat(filename, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

void escribir_log(char*nombre_log,char*proceso,char*mensaje,char*tipo){
	if(strcmp(tipo,"info")==0){
		t_log* log=log_create(nombre_log,proceso,false,LOG_LEVEL_INFO);
		log_info(log,mensaje);
		log_destroy(log);
	}else{
		if(strcmp(proceso,"dt")==0 || strcmp(proceso,"wk")==0){
			t_log* log=log_create(nombre_log,proceso,true,LOG_LEVEL_INFO);
			log_info(log,mensaje);
			log_destroy(log);
		}else{
			t_log* log=log_create(nombre_log,proceso,true,LOG_LEVEL_ERROR);
			log_error(log,mensaje);
			log_destroy(log);
		}
	}
}
