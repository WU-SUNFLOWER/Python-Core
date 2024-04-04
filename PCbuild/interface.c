#include "Python.h"

PyAPI_FUNC(void) ConvertCodeStringToByteCode(const char* pyFileName, const char* pycFileName) {
    // ��ʼ��Python������
    Py_Initialize();

    // Python�����ַ���
    size_t bufferSize = 100;
    char* code_str = (char*)malloc(bufferSize);
    if (code_str == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(-1);
    }
    memset(code_str, 0, bufferSize);

    char line[2500] = { 0 };
    size_t totalLength = 0;
    
    FILE* sourceCodeFile = fopen(pyFileName, "r");
    if (!sourceCodeFile) {
        fprintf(stderr, "Can't open %s\n", pyFileName);
        exit(-1);
    }

    while (fgets(line, sizeof(line), sourceCodeFile)) {
        size_t lineLength = strlen(line);
        // ������ǰ�Ļ������ܷ�װ�£�����װ��������
        size_t minBufferLength = totalLength + lineLength + 1;
        if (minBufferLength > bufferSize) {
            size_t expectBufferLength = bufferSize << 1;
            bufferSize =
                expectBufferLength > minBufferLength ? expectBufferLength : minBufferLength;
            char* newBuffer = realloc(code_str, bufferSize);
            if (newBuffer == NULL) {
                free(code_str);
                fprintf(stderr, "Memory allocation failed.\n");
                exit(-1);
            }
            code_str = newBuffer;
        }
        totalLength += lineLength;
        strcat(code_str, line);
    }
    fclose(sourceCodeFile);

    // ����Python����
    PyObject* codeObject = Py_CompileStringFlags(code_str, pyFileName, Py_file_input, NULL);

    if (codeObject != NULL) {
        // ��һ���ļ�����д��
        FILE* myfile = fopen(pycFileName, "wb");
        if (!myfile) {
            fprintf(stderr, "Can't create %s\n", pycFileName);
            exit(-1);
        }

        // д���ļ�
        int magic = 0x00000000;
        int stamp = 0x00000000;
        fwrite(&magic, 4, 1, myfile);
        fwrite(&stamp, 4, 1, myfile);
        PyMarshal_WriteObjectToFile(codeObject, myfile, 0);
        // ����
        free(code_str);
        fclose(myfile);
        Py_DECREF(codeObject);
    }
    else {
        // �������ʧ�ܣ���ӡ������Ϣ
        PyErr_Print();
        exit(-1);
    }
    Py_Finalize();
}