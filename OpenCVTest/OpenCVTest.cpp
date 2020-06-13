#include "CaptureAndSend.h"

/** Function Headers */
//void detectAndDisplay(Mat frame);

/** @function main */
int main(int argc, const char** argv)
{
    CaptureVideo caputure_video_obj(argc, argv);
    caputure_video_obj.capture_video();

    return 0;
}


