#include "socket_header.h"
#include "image.h"
#include "cJSON.h"
#include "base64.h"

#include <iostream>

#define PERSON_MAX_NUM 3

void image_to_json(image im, int port);

void json_to_image(const char * json_str);

//image json_to_image(const char * json_str);

int send_message(std::string ip, int port, int quality);
