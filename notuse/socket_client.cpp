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
        //std::cout<<"_connect"<<std::endl;

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

        //const char *buffer = message.c_str();

        //while(send_len < message_len)
        //{
        //    send_len += send(send_sock, &buffer[send_len], message_len, 0);
        //}

        send_len = send(send_sock, message.c_str(), message_len, 0);

        return send_len;
    }

    int _write_frame(int &send_sock, std::vector<uchar> frame, int frame_len)
    {
        int send_len = 0;

        send_len = send(send_sock, frame.data(), frame_len, 0);

        return send_len;
    }

    int _write_len(int send_sock, int messagee_len)
    {
        //std::cout<<"_write_len"<<std::endl;

        int send_len = 0;  

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
    /*bool start()
    {
        std::string message = "test";
        
        int send_len = 0;
        send_len = _write_len(client_sock, message.size());
        printf("send: %d\n", send_len);

        send_len = _write(client_sock, message.c_str(), message.size());
        printf("send: %d\n", send_len);

        return true;
    }*/

    bool start(std::vector<uchar> frame)
    {
        printf("frame size: %d\n", frame.size());
        int send_len = 0;
        send_len = _write_len(client_sock, frame.size());
        printf("send: %d\n", send_len);

        if(send_len < 0) // server is cracked and send_len = -1
        {
            return false;
        }

        send_len = _write_frame(client_sock, frame, frame.size());
        printf("send: %d\n", send_len);

        if(send_len < 0) // server is cracked and send_len = -1
        {
            return false;
        }

        return true;
    }
};


/*int send_message(std::string ip, int port, int quality)
{
    static ClientSocket client_socket(ip, port, quality);

    client_socket.start();

    return 0;
}*/

int send_frame(std::string ip, int port, int quality, std::vector<uchar> frame)
{
    static ClientSocket client_socket(ip, port, quality);

    if(!client_socket.start(frame)) // check whether server is cracked
    {
        exit(EXIT_FAILURE);
    }

    return 0;
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