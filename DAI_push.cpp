#include "DAI_push.h"

void iot_init()
{
    Py_Initialize();

    //PySys_SetPath(PYTHON_IMPORT_PATH);

    //setenv("PYTHONPATH", PYTHON_IMPORT_PATH, 1);

    pModule = PyImport_ImportModule("DAI_push");
    CHECK_PYTHON_NULL(pModule)

    pDict = PyModule_GetDict(pModule);
    CHECK_PYTHON_NULL(pDict)

    pFunc = PyDict_GetItemString(pDict, "send_frame_to_iottalk");
    CHECK_PYTHON_NULL(pFunc)
}

void iot_send(std::vector<uchar> &outbuf, std::vector<person_box> &boxes)
{
    PyObject *pFrameList  = PyList_New(outbuf.size()); // store frame undigned char int

    int boxes_size = boxes.size();
    PyObject *pBoxDict[boxes_size]; // store all boxes information
    for(int i = 0; i < boxes_size; i++) pBoxDict[i] = PyDict_New();
    PyObject *pBoxList  = PyList_New(boxes_size); // store all box dictionaries
    
    PyObject *pArgList = PyTuple_New(2); // arguments for function

    //std::cout<<"pFrameList"<<std::endl;

    // create python list with frame data
    for(int i = 0; i < PyList_Size(pFrameList); i++)
    {
        PyList_SetItem(pFrameList, i, PyInt_FromLong((int)outbuf[i]));
    }

    PyTuple_SetItem(pArgList, 0, pFrameList); // put in argument

    //std::cout<<"pBoxDict"<<std::endl;

    // create python dictionary with box information
    for(int i = 0; i < boxes_size; i++)
    {
        PyDict_SetItem(pBoxDict[i], PyString_FromString("name"), PyString_FromString((boxes[i].name).c_str()));
        PyDict_SetItem(pBoxDict[i], PyString_FromString("id"), PyInt_FromLong(boxes[i].id));
        PyDict_SetItem(pBoxDict[i], PyString_FromString("x1"), PyInt_FromLong(boxes[i].x1));
        PyDict_SetItem(pBoxDict[i], PyString_FromString("y1"), PyInt_FromLong(boxes[i].y1));
        PyDict_SetItem(pBoxDict[i], PyString_FromString("x2"), PyInt_FromLong(boxes[i].x2));
        PyDict_SetItem(pBoxDict[i], PyString_FromString("y2"), PyInt_FromLong(boxes[i].y2));
    }

    //std::cout<<"pBoxList"<<std::endl;

    // put all dictionaries into list
    for(int i = 0; i < boxes_size; i++)
    {
        PyList_SetItem(pBoxList, i, pBoxDict[i]);
    }

    PyTuple_SetItem(pArgList, 1, pBoxList); // put in argument

    if(PyCallable_Check(pFunc))
    {
        PyObject_CallObject(pFunc, pArgList);
    }
    else
    {
        PyErr_Print();
    }
}