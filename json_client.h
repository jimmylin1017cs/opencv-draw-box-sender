#include "socket_header.h"
#include "image.h"
#include "cJSON.h"

#define PERSON_MAX_NUM 3

void image_to_json(image im, int port);

image json_to_image(const char * json_str);