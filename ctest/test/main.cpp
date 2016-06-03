#include "cepollclient.h"
int main(int argc, char *argv[])
{
	CEpollClient *pCEpollClient = new CEpollClient(1000, "192.168.0.73", 8888);
	if(NULL == pCEpollClient)
	{
		cout<<"[epollclient error]:main init"<<"Init CEpollClient fail"<<endl;
	}
	pCEpollClient->RunFun();
	if(NULL != pCEpollClient)
	{
		delete pCEpollClient;
		pCEpollClient = NULL;
	}
	return 0;
}
