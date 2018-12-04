#include "image.h"
#include "mjpeg_streaming.h"
#include "DAI_push.h"
#include <iostream>
#include <vector>
#include <ctime>

static cv::VideoCapture *cap;
static image im;
static cv::Mat m, frame;
static image **alphabet;

int main()
{
    alphabet = load_alphabet();
    int red = 0, green = 0, blue = 255; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2

    srand(time(NULL));
    iot_init();

    char *filename = "time_counter.flv";

    cap = new cv::VideoCapture(filename);

    while(1)
    {
        *cap >> m;

        cv::resize(m, frame, cv::Size(800, 600), 0, 0);

        person_box b1 = {"person_name", 1, (rand()%100)+100, (rand()%100)+100, (rand()%300)+200, (rand()%300)+200};
        //person_box b2 = {"test_name", 2, 500, 100, 600, 500};
        std::vector<person_box> boxes;
        boxes.push_back(b1);
        //boxes.push_back(b2);

        //std::cout<<b.name<<std::endl;

        std::vector<uchar> outbuf;
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY); // default quality value is 95
        compression_params.push_back(30);

        cv::imencode(".jpg", frame, outbuf, compression_params); // encodes an image into a memory buffer

        iot_send(outbuf, boxes);

        im = mat_to_image(frame);

        width = im.h * .012;
        alphabet_size = im.h*.03;

        image label;

        for(int i = 0; i < boxes.size(); i++)
        {
            left = boxes[i].x1;
            top = boxes[i].y1;
            right = boxes[i].x2;
            bot = boxes[i].y2;
            draw_box_width(im, left, top, right, bot, width, red, green, blue);
            image label = get_label(alphabet, (char *)boxes[i].name.c_str(), alphabet_size);
            draw_label(im, top + width + alphabet_size, left, label, rgb);
        }

        //draw_box_width(im, left, top, right, bot, width, red, green, blue);
        
        //image label = get_label(alphabet, (char *)name.c_str(), alphabet_size);
        //draw_label(im, top + width + alphabet_size, left, label, rgb);

        frame = image_to_mat(im);

        cv::imshow("Sender", frame);
        cv::waitKey(1);

        //std::cout<<"jpg: "<<outbuf.size()<<std::endl;

        //send_frame("140.113.86.135", 8090, 95, outbuf);

        //im = resize_image(im, 800, 600);
        //show_image_cv(im, "Demo", 1);
    }
    
    //send_message("140.113.86.135", 8090, 95);

    /*char *filename = "predictions.jpg";

    cap = new VideoCapture(filename);

    Mat m;
    *cap >> m;

    std::vector<uchar> frame;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY); // default quality value is 95
    compression_params.push_back(30);

    cv::imencode(".jpg", m, frame, compression_params); // encodes an image into a memory buffer


    cout<<frame.size()<<endl;
    
    send_frame("140.113.86.135", 8090, 95, frame);*/






    //image im = load_image_color("predictions.jpg", 0, 0);

    //im = resize_image(im, 100, 100);

    //image_to_json(im, 8080);

    //int w = im.w, h = im.h, c = im.c;
    //int im_size = h*w*c;

    //printf("w=%d, h=%d, c=%d, im_size=%d\n", w, h, c, im_size);

    /*for(int i=0; i<im_size; i++)
    {
        if(i % 100 == 0) cout<<endl;

        cout<<(int)((unsigned char)(im.data[i]*(255)))<<"  ";
    }*/

    //show_image_cv(im, "predictions", 0);

    //int c =  show_image_cv(im, "predictions", 0);

    /*image in = load_image("predictions.jpg", 768, 576, 3);
    int c = show_image_cv(in, "Demo", 0);*/
    //cout<<c<<endl;

    //while(1);

    //alphabet = load_alphabet(); // read all alphabet picture

    //cap = open_video_stream("test.MTS", 0, 0, 0, 0); // open video
    //cap = open_video_stream("rtmp://140.113.86.135/demo", 0, 0, 0, 0);

    /*int red = 0, green = 0, blue = 255; // box color
    float rgb[3] = {red, green, blue};

    int width = 3; // box line width
    int alphabet_size = 3;

    int left = 50; // x1
    int top = 50; // y1
    int right = 200; // x2
    int bot = 300; // y2*/

    /*while(1)
    {
        im = get_image_from_stream(cap);
        //cout<<(*((VideoCapture*)cap)).get(CV_CAP_PROP_POS_MSEC)<<endl;
        //cout<<(*((VideoCapture*)cap)).get(CV_CAP_PROP_POS_FRAMES)<<endl;
        im = resize_image(im, 800, 600);

        std::vector<uchar> outbuf;
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY); // default quality value is 95
        compression_params.push_back(95);
        cv::imencode(".jpg", image_to_mat(im), outbuf, compression_params); // encodes an image into a memory buffer

        size_t outlen = outbuf.size();
        cout<<outlen<<endl;

        cv::Mat mat;
        imdecode(outbuf, CV_LOAD_IMAGE_COLOR, &mat);
        image im2 = mat_to_image(mat);

        show_image_cv(im2, "Demo", 1);*/


        //image_to_json(im, 8080);

        //int w = im.w, h = im.h, c = im.c;
        //int im_size = h*w*c;

        //printf("w=%d, h=%d, c=%d, im_size=%d\n", w, h, c, im_size);

        /*for(int i=0; i<im_size; i++)
        {
            if(i % 10 == 0) cout<<endl;

            cout<<im.data[i]<<"  "<<endl;
        }*/

        //printf("w=%d, h=%d, , c=%d, size=%d\n", im.w, im.h, im.c, (int)sizeof(im.data));

        /*width = im.h * .012;
        alphabet_size = im.h*.03;
        draw_box_width(im, 50, 50, 200, 300, width, red, green, blue);
        image label = get_label(alphabet, "person_name", alphabet_size);
        draw_label(im, top + width + alphabet_size, left, label, rgb);
        send_mjpeg(im, 8090, 200, 95);*/
        //show_image_cv(im, "Demo", 1);
        //int c = show_image_cv(im, "Demo", 1);

        //cout<<c<<endl;

        //free_image(im);
    //}

    return 0;
}