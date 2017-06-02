
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>          
#include <semaphore.h>     
#include <sys/mman.h>     
#include <sys/resource.h>
#include <sys/time.h>   
#include <signal.h>
#include <pthread.h>

#define OPTION "hi:I:o:O:"


static int float_32bit_to_signed_16bit(unsigned char *in_data, int in_size, 
	unsigned char **out_data)
{
	int input_samples = in_size/4;//32bit float
	int i = 0;
	float f_tmp = 0;
	void *f_ptmp = &f_tmp;
	short s_tmp = 0;
	unsigned char *out_tmp = NULL;

	out_tmp = (unsigned char *)malloc(input_samples * 2);//16bit short
	if(!out_tmp){
		return -1;
	}
	memset(out_tmp, 0, input_samples * 2);

	for(i = 0; i < input_samples; i++) {
		f_tmp = 0;

		*(((unsigned char *)f_ptmp) + 0) = in_data[i*4 + 0];
		*(((unsigned char *)f_ptmp) + 1) = in_data[i*4 + 1];
		*(((unsigned char *)f_ptmp) + 2) = in_data[i*4 + 2];
		*(((unsigned char *)f_ptmp) + 3) = in_data[i*4 + 3];
		f_tmp = (f_tmp * 32768.0f);
		if(f_tmp > 32767)
			f_tmp = 32767;
		if(f_tmp < -32767)
			f_tmp = -32767;

		s_tmp = (short)f_tmp;
		out_tmp[i*2 + 0] = (s_tmp & 0x00ff);
		out_tmp[i*2 + 1] = ((s_tmp>>8) & 0x00ff);
	}

	*out_data = out_tmp;

	return (input_samples * 2);
}

static long get_filesize(char *filename)
{
    FILE *fp = NULL;
    long filesize = 0;

    if (!filename) {
        printf("input param error\n");
        return -1;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        printf("open %s error\n", filename);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fclose(fp);
    fp = NULL;
    printf("file %s size %ld\n", filename, filesize);

    return filesize;
}

int main(int argc, char *argv[])
{
	char in_file[128] = {0};
	FILE *in_fp = NULL;
	unsigned char *data = NULL;
	int filesize = 0;

	char out_file[128] = {0};
	FILE *out_fp = NULL;
	unsigned char *out_data = NULL;

	int ret = 0;
	int ch = 0;

	while((ch = getopt(argc, argv, OPTION)) !=-1 ) {
        fprintf(stderr,"optarg:%s ",optarg);
        fprintf(stderr,"ch:%c \n",ch);
        switch(ch){
        case 'h':
            break;
        case 'i':
        case 'I':
            if(optarg){
                memset(in_file, 0, sizeof(in_file));
                strcpy(in_file, optarg);
            }
            break;
        case 'o':
        case 'O':
            if(optarg){
                memset(out_file, 0, sizeof(out_file));
                strcpy(out_file, optarg);
            }
            break;
        default:
            break;
        }
    }
    printf("in_file: %s, out_file:%s\n", in_file, out_file);

    filesize = get_filesize(in_file);

    in_fp = fopen(in_file, "rb");
    if(!in_fp){
    	printf("open in file %s error\n", in_file);
    	return -1;
    }

    out_fp = fopen(out_file, "wb+");
    if(!out_fp){
    	fclose(in_fp);
    	printf("open out file %s error\n", out_file);
    	return -1;
    }

    data = (unsigned char *)malloc(filesize);
    if(!data){
    	fclose(in_fp);
    	fclose(out_fp);
    	printf("malloc in file buffer error\n");
    	return -1;
    }
    memset(data, 0, filesize);

    ret = fread(data, 1, filesize, in_fp);
    fclose(in_fp);
    in_fp = NULL;

    ret = float_32bit_to_signed_16bit(data, filesize, &out_data);
    if(ret > 0){
    	fwrite(out_data, 1, ret, out_fp);
    	free(out_data);
    }

    fclose(in_fp);
    fclose(out_fp);

	return 0;
}



