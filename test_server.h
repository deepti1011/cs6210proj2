RING_BUFFER_SIZE = 20


struct Request
{
	int requestID;
	int x;
	int p;
	int status;
};

enum REQUEST_STATUS 
{
	FREE = 1,
	REQUESTED = 2,
	RESPONSE = 3 
};

Request[RING_BUFFER_SIZE] ringBuffer;
const char fname[] = "./requestFile";