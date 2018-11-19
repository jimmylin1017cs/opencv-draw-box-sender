#include "json_client.h"

#include <iostream>
#include "opencv2/opencv.hpp"

class JSONClient
{
    SOCKET sock;
    SOCKET maxfd; // record the max value of sock for select() loop
    PORT port;

    fd_set masterfds;
    int timeout; // readfds sock timeout, shutdown after timeout millis.
    int quality; // jpeg compression [1..100]

    // sock: client socket
    // s: constant pointer to a constant char
    // len: length of s
    int _write(int sock, char const * const s, int len)
    {
        if (len < 1)
        {
            len = strlen(s);
        }
        return ::send(sock, s, len, 0);
    }

    void _read(int sock, char * s, int len)
    {
        recv(sock, s, len, 0);
        printf("%s\n", s);
    }

public:

    // constructor
    // _port: port number
    // _timeout: timeout for select() timeval
    // _quality: jpeg compression [1..100] for cv::imencode (the higher is the better)
    JSONClient(int _port = 0, int _timeout = 200000, int _quality = 30)
        : port(_port)
        , sock(INVALID_SOCKET)
        , timeout(_timeout)
        , quality(_quality)
    {
        // signal(SIGPIPE, SIG_IGN); // ignore ISGPIP to avoid client crash and server is forcde to stop

        if(port) open(port); // if port > 0, then create a server with port
    }

    // destructor
    ~JSONClient()
    {
        release();
    }

    bool release()
    {
        if (sock != INVALID_SOCKET)
            ::shutdown(sock, 2); // disable receive or send data, like close()
        sock = (INVALID_SOCKET);
        return false;
    }


    bool open(int port)
    {
        sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN address; // server address information
        address.sin_addr.s_addr = inet_addr("140.113.86.137"); // server address for connecting
        address.sin_family = AF_INET;
        address.sin_port = htons(port);

        if (::connect(sock, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't connect on sock " << sock << " on port " << port << " !" << std::endl;
            return release();
        }

        return true;
    }

    bool isOpened()
    {
        return sock != INVALID_SOCKET;
    }

    bool write(const char * json_str)
    {

        _write(sock, json_str, strlen(json_str));

        return true;
    }
};



void image_to_json(image im, int port)
{
    // create only one MJPGWriter object
    //static JSONClient json_client(port);

    IplImage *disp = image_to_ipl(im);

    int h = disp->height;
    int w = disp->width;
    int c = disp->nChannels;
    int im_size = h*w*c;

    int step = disp->widthStep;

    int *im_data = (int *)calloc(im_size, sizeof(int));

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

    char *json_str = (char *)malloc(sizeof(char) * strlen(str));

    strcpy(json_str, str);

    cJSON_Delete(cjson_obj);

    json_to_image(json_str);

    //printf("%s (%d)\n",json_str, strlen(json_str));

    //return json_str;
}

image json_to_image(const char * json_str)
{
    //printf("%s (%d)\n",json_str, strlen(json_str));

    int id, h, w, c, step;

    h = w = c = step = 0;

    unsigned char *data;

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

        data = (unsigned char *)calloc(h*w*c, sizeof(unsigned char));
        
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

    show_image_cv(im, "predictions", 0);

    return im;
}

/*IplImage *image_to_ipl(image im)
{
    int x,y,c;
    IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    int step = disp->widthStep;
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    return disp;
}

cv::Mat image_to_mat(image im)
{
    image copy = copy_image(im);
    constrain_image(copy);
    if(im.c == 3) rgbgr_image(copy);

    IplImage *ipl = image_to_ipl(copy);
    cv::Mat m = cv::cvarrToMat(ipl, true);
    cvReleaseImage(&ipl);
    free_image(copy);
    return m;
}*/

/*
void send_mjpeg(image im, int port, int timeout, int quality) {
    
    // create only one MJPGWriter object
    static JSONServer mjpeg_writer(port, timeout, quality);

    cv::Mat mat;

    cv::resize(image_to_mat(im), mat, cv::Size(1352, 1013));

    mjpeg_writer.write(mat);
}
*/