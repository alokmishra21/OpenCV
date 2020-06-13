#include "CaptureAndSend.h"
#include "Config.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctime>
using namespace cv;


/** Global variables */
//CascadeClassifier face_cascade;
//CascadeClassifier eyes_cascade;


SOCKET sock;

int CaptureVideo::capture_video()
{
    try
    {
        WORD wVersionRequested;
        WSADATA wsaData;

        wVersionRequested = MAKEWORD(2, 2);              // Request WinSock v2.0
        if (WSAStartup(wVersionRequested, &wsaData) != 0) {  // Load WinSock 
            printf("Failed.Error Code : % d ", WSAGetLastError());
            return -1;
        }
        if ((sock = socket(PF_INET, AF_INET, 0)) < 0) {
            std::cout << "Socket creation failed (socket())\n";
            return -1;
        }
        db("socket successfully created");

        int bindStatus  = setLocalPort();
        if (bindStatus == -1)
        {
           std::cout << "Socket binding failed (socket())\n";
            return -1;
        }

        int jpegqual = ENCODE_QUALITY;

        std::vector < uchar > encoded;
        Mat frame, send;

        VideoCapture capture(0);
        namedWindow("send", WINDOW_AUTOSIZE);

        if (!capture.isOpened())
        {
            std::cout << "--(!)Error opening video capture\n";
            return -1;
        }

        clock_t last_cycle = clock();
        while (1)
        {
            capture >> frame;
            if (frame.empty())
            {
                std::cout << "--(!) No captured frame -- Break!\n";
                break;
            }
            resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);
            std::vector < int > compression_params;
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(jpegqual);

            imencode(".jpg", send, encoded, compression_params);
            imshow("send", send);
            int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;

            int ibuf[1];
            ibuf[0] = total_pack;
            sendFrame(ibuf, sizeof(int));

            for (int i = 0; i < total_pack; i++)
                sendFrame(&encoded[i * PACK_SIZE], PACK_SIZE);

            if (waitKey(FRAME_INTERVAL) == 27)
            {
                break; // escape
            }

            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double)CLOCKS_PER_SEC;
            std::cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << std::endl;

            std::cout << next_cycle - last_cycle;
            last_cycle = next_cycle;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    closesocket(sock);
    WSACleanup();

    return 0;
}

int CaptureVideo::setLocalPort()
{
    db("ENTER -- CaptureVideo::setLocalPort");
    // Bind the socket to its port
    sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(m_usPort);

    if (::bind(sock, (sockaddr*)&localAddr, sizeof(sockaddr_in)) < 0) {
        std::cout << "Set of local port failed (bind())" << std::endl;
        return -1;
    }
    db("LEAVE -- CaptureVideo::setLocalPort");
}

void CaptureVideo::sendFrame(const void* buffer, int bufferLen) {
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL, *p;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;    // Internet address

    DWORD dwRetval = getaddrinfo(m_sServAddress.c_str(), (char *)m_usPort, &hints, &result);
    if (dwRetval != 0) {
        // strerror() will not work for gethostbyname() and hstrerror() 
        // is supposedly obsolete
        printf("getaddrinfo failed with error: %d\n", dwRetval);
        WSACleanup();
        exit(-1);
    }
    for (p = result; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }


    // Write out the whole buffer as a single message.
    if (sendto(sock, (char*)buffer, bufferLen, 0,
        p->ai_addr, p->ai_addrlen) != bufferLen) {
        std::cout<< "Send failed (sendto())\n";
        exit(-1);
    }
}



///** @function detectAndDisplay */
//void CaptureVideo::detectAndDisplay(Mat frame)
//{
//    Mat frame_gray;
//    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
//    equalizeHist(frame_gray, frame_gray);
//
//    //-- Detect faces
//    std::vector<Rect> faces;
//    face_cascade.detectMultiScale(frame_gray, faces);
//
//    for (size_t i = 0; i < faces.size(); i++)
//    {
//        Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
//        ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4);
//
//        Mat faceROI = frame_gray(faces[i]);
//
//        //-- In each face, detect eyes
//        std::vector<Rect> eyes;
//        eyes_cascade.detectMultiScale(faceROI, eyes);
//
//        for (size_t j = 0; j < eyes.size(); j++)
//        {
//            Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
//            int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
//            circle(frame, eye_center, radius, Scalar(255, 0, 0), 4);
//        }
//    }
//
//    //-- Show what you got
//    imshow("Capture - Face detection", frame);
//}
