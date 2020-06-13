#pragma once
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

class CaptureVideo
{
public:
	CaptureVideo(int argc, const char **argv)
	{
		if ((argc < 3) || (argc > 3)) { // Test for correct number of arguments
			std::cerr << "Usage: " << argv[0] << " <Server> <Server Port>\n";
			exit(1);
		}

		m_sServAddress = argv[1]; // First arg: server address
		m_usPort = atoi(argv[2]);
	}
	int capture_video();
	//void detectAndDisplay(Mat frame);
private:
	unsigned short m_usPort;
	std::string m_sServAddress;

	int setLocalPort();
	void sendFrame(const void* buffer, int bufferLen);
};

