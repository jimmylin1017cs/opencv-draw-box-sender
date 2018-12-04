#ifndef DAI_PUSH
#define DAI_PUSH

#include "image.h"
#include "cJSON.h"

#include <Python.h>
#include <iostream>
#include <vector>

#define CHECK_PYTHON_NULL(p) \
    if (NULL == (p)) {\
        PyErr_Print();\
        exit(EXIT_FAILURE);\
    }


static PyObject *pModule, *pDict, *pFunc, *pInstance;

#endif

void iot_init();
void iot_send(std::vector<uchar> &outbuf, std::vector<person_box> &boxes);