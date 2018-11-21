#include "socket_client.h"

#include "opencv2/opencv.hpp"

class ClientSocket
{
    SOCKET client_sock;

    PORT server_port;
    IP_ADDR server_ip;

    int quality; // jpeg compression [1..100]

    // @port: port number
    bool _connect(int server_port)
    {
        std::cout<<"_connect"<<std::endl;

        client_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN address; // server address information
        address.sin_addr.s_addr = inet_addr(SERVER_IP); // server address for connecting
        address.sin_family = AF_INET;
        address.sin_port = htons(server_port);

        if (::connect(client_sock, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
        {
            std::cerr << "error : couldn't connect on client sock " << client_sock << " on port " << server_port << " !" << std::endl;
            return _release();
        }

        return true;
    }

    // shutdown sock
    bool _release()
    {
        if (client_sock != INVALID_SOCKET)
            ::shutdown(client_sock, 2); // disable receive or send data, like close()
        client_sock = (INVALID_SOCKET);
        return false;
    }

    // @send_sock: client socket
    // @message: string of message
    // @message_len: length of s
    int _write(int send_sock, std::string message, int message_len)
    {
        int send_len = 0;

        /*const char *buffer = message.c_str();

        while(send_len < message_len)
        {
            send_len += send(send_sock, &buffer[send_len], message_len, 0);
        }*/

        send_len = send(send_sock, message.c_str(), message_len, 0);

        return send_len;
    }

    int _write_len(int send_sock, int messagee_len)
    {
        int send_len = 0;

        std::cout<<"_write_len"<<std::endl;

        send_len = send(send_sock, &messagee_len, sizeof(int), 0);

        return send_len;
    }

    // @receive_sock
    // @message_len
    std::string _read(int receive_sock, int message_len)
    {
        std::string message;
        message.clear(); // initial message

        int receive_len = 0;
        int rest_len = message_len;
        char buffer[BUFFER_MAX];
        memset(buffer, '\0', sizeof(char) * BUFFER_MAX);

        if(message_len <= BUFFER_MAX)
        {
            receive_len = recv(receive_sock, buffer, message_len, 0);
        }
        else
        {
            while(rest_len)
            {
                if(rest_len <= BUFFER_MAX)
                {
                    receive_len += recv(receive_sock, buffer, rest_len, 0);
                }
                else
                {
                    receive_len += recv(receive_sock, buffer, BUFFER_MAX, 0);
                    rest_len = message_len - receive_len;
                }

                message += buffer;
                memset(buffer, '\0', sizeof(char) * BUFFER_MAX); // clear buffer for new message
            }
        }

        std::cout<<message<<std::endl;

        return message;
    }

    int _read_len(int receive_sock)
    {
        int message_len = 0;
        recv(receive_sock, &message_len, sizeof(int), 0);
        return message_len;
    }

public:

    // constructor
    // _port: port number
    // _timeout: timeout for select() timeval
    // _quality: jpeg compression [1..100] for cv::imencode (the higher is the better)
    ClientSocket(std::string _ip = "", int _port = 0, int _quality = 30)
        : server_ip(_ip)
        , server_port(_port)
        , client_sock(INVALID_SOCKET)
        , quality(_quality)
    {
        // signal(SIGPIPE, SIG_IGN); // ignore ISGPIP to avoid client crash and server is forcde to stop

        std::cout<<"constructor"<<std::endl;

        if(server_ip.size() && server_port) _connect(server_port); // if port > 0, then create a server with port
    }

    // destructor
    ~ClientSocket()
    {
        _release();
    }

    // check sock is opened or not
    bool isOpened()
    {
        return client_sock != INVALID_SOCKET;
    }

    // @s: constant pointer to a constant char for message
    bool start()
    {
        int send_len = 0;
        send_len = _write_len(client_sock, 100);

        printf("send: %d\n", send_len);

        return true;
    }
};


int send_message(std::string ip, int port, int quality)
{
    static ClientSocket client_socket(ip, port, quality);

    client_socket.start();

    return 0;
}




static int *im_data = NULL;
static char *json_str = NULL;
static int init_check_create = 0;

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