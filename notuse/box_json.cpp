#include "box_json.h"

static int *im_data = NULL;
static char *json_str = NULL;
static int init_check_create = 0;

void image_to_json()
{
    // create only one MJPGWriter object
    //static JSONClient json_client(port);

    IplImage *disp = image_to_ipl(im);

    int h = disp->height;
    int w = disp->width;
    int c = disp->nChannels;
    int im_size = h*w*c;

    int step = disp->widthStep;

    if(!init_check_create)
    {
        im_data = (int *)calloc(im_size, sizeof(int));
    }

    for(int i=0; i<im_size; i++)
    {
        im_data[i] = (int)((unsigned char)(disp->imageData)[i]);
    }

    /*for(int i=0; i<im_size; i++)
    {
        printf("%d", (int)((disp->imageData)[i]));
    }*/

    cJSON *cjson_obj;
    cJSON *cjson_tmp[PERSON_MAX_NUM];
    cJSON *cjson_image;

    cjson_obj = cJSON_CreateArray();
    cjson_image = cJSON_CreateIntArray(im_data, im_size);

    cjson_tmp[0] = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjson_tmp[0],"id",1);
    cJSON_AddNumberToObject(cjson_tmp[0],"height",h);
    cJSON_AddNumberToObject(cjson_tmp[0],"width",w);
    cJSON_AddNumberToObject(cjson_tmp[0],"nChannels",c);
    cJSON_AddNumberToObject(cjson_tmp[0],"step",step);
    cJSON_AddItemToObject(cjson_tmp[0],"data",cjson_image);
    cJSON_AddItemToArray(cjson_obj, cjson_tmp[0]);

    char *str = cJSON_PrintUnformatted(cjson_obj);

    //printf("%s (%d)\n",str, strlen(str));

    //printf("%d\n", cJSON_GetArraySize(cjson_image));

    if(!init_check_create)
    {
        init_check_create = 1;
        json_str = (char *)calloc(strlen(str), sizeof(char));
    }

    //char *json_str = (char *)calloc(strlen(str), sizeof(char));

    strcpy(json_str, str);

    cJSON_Delete(cjson_obj);

    /*int json_str_length = strlen(json_str); // data length
    int base64_str_length = Base64encode_len(json_str_length);
    char *base64_str = (char *)calloc(base64_str_length, sizeof(char));
    Base64encode(base64_str, json_str, json_str_length);

    printf("%s\n", base64_str);*/

    json_to_image(json_str);

    cvReleaseImage(&disp);

    //json_to_image(json_str);

    //printf("(%d) (%d)\n", strlen(json_str), strlen(base64_str));

    //printf("%s (%d)\n",json_str, strlen(json_str));

    //return json_str;
}

static int init_check_parse = 0;
static unsigned char *data = NULL;

void json_to_image(const char * json_str)
{
    //printf("%s (%d)\n",json_str, strlen(json_str));

    /*char* data = NULL;
    int data_length = 0;
    int alloc_length = Base64decode_len(base64_string);
    some_random_data = malloc(alloc_length);
    data_length = Base64decode(data, base64_string);*/

    static int id = 0, h = 0, w = 0, c = 0, step = 0;

    // Parsing json
    cJSON *cjson_obj = cJSON_Parse(json_str);

    cJSON *cjson_tmp;
    cJSON *cjson_data;
    cJSON *cjson_int;

    cJSON_ArrayForEach(cjson_tmp, cjson_obj)
    {
        //printf("%s\n", cJSON_PrintUnformatted(cjson_tmp));

        id = (cJSON_GetObjectItem(cjson_tmp, "id"))->valueint;
        h = (cJSON_GetObjectItem(cjson_tmp, "height"))->valueint;
        w = (cJSON_GetObjectItem(cjson_tmp, "width"))->valueint;
        c = (cJSON_GetObjectItem(cjson_tmp, "nChannels"))->valueint;
        step = (cJSON_GetObjectItem(cjson_tmp, "step"))->valueint;

        if(!init_check_parse)
        {
            init_check_parse = 1;
            data = (unsigned char *)calloc(h*w*c, sizeof(unsigned char));
        }
        //data = (unsigned char *)calloc(h*w*c, sizeof(unsigned char));
        
        cjson_data = cJSON_GetObjectItem(cjson_tmp, "data");

        //printf("%d\n", cJSON_GetArraySize(cJSON_GetObjectItem(cjson_tmp, "data")));

        int count = 0;
        cJSON_ArrayForEach(cjson_int, cjson_data)
        {
            data[count] = (unsigned char)cjson_int->valueint;
            //printf("%d ", cjson_int->valueint);
            count++;
        }

        printf("%d\n", count);
    }

    cJSON_Delete(cjson_obj);
    //cJSON_Delete(cjson_tmp);
    //cJSON_Delete(cjson_int);


    // Convert to image
    image im = make_image(w, h, c);

    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                // ##### to get image information
                //printf("%d ", (int)data[i*step + j*c + k]);
                //printf("%u ", (unsigned char)data[i*step + j*c + k]);

                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }

    show_image_cv(im, "predictions", 1);

    free_image(im);

    //return im;
}