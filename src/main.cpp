#include <iostream>
#include <sstream>
#include "rdma.h"

using namespace std;
using namespace rapidjson;

void work(char *X)
{
	bool config_flag = true;
	shared_ptr<RDMAEngine::RDMA_Manager> rdma_mg = std::shared_ptr<RDMAEngine::RDMA_Manager>();
	rdma_mg = std::make_shared<RDMAEngine::RDMA_Manager>(config_flag);
	int rc = 1;

	char temp_char;
	char temp_send[] = "R";
	if (rdma_mg->init())
	{
		fprintf(stderr, "failed to init resources\n");
		goto main_exit;
	}

	strcpy(rdma_mg->res->buf, X);
	fprintf(stdout, "going to send the message: '%s'\n", rdma_mg->res->buf);
	/* let the server post the sr */
	if (rdma_mg->RDMA_Send())
	{
		fprintf(stderr, "failed to post sr\n");
		goto main_exit;
	}

	if (rdma_mg->RDMA_Receive())
	{
		fprintf(stderr, "failed to connect QPs\n");
		goto main_exit;
	}
	/* after polling the completion we have the message in the client buffer too */
	fprintf(stdout, "Message is: '%s', %ld\n", rdma_mg->res->buf, strlen(rdma_mg->res->buf));
	strcpy(rdma_mg->res->buf, "34111111111");
	/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
	if (rdma_mg->sock_sync_data(rdma_mg->res->sock, 1, temp_send, &temp_char)) /* just send a dummy char back and forth */
	{
		fprintf(stderr, "sync error before RDMA ops\n");
		rc = 1;
		goto main_exit;
	}
	/*-------------------------------------------------------------------------------------------------------------------------------------------------------*/
	if (rdma_mg->RDMA_Read("34111111111")) /* just send a dummy char back and forth */
	{
		fprintf(stderr, "RDMA_Read ops\n");
		rc = 1;
		goto main_exit;
	}
	if (rdma_mg->RDMA_Write()) /* just send a dummy char back and forth */
	{
		fprintf(stderr, "RDMA_Write ops\n");
		rc = 1;
		goto main_exit;
	}
	if (rdma_mg->RDMA_Write()) /* just send a dummy char back and forth */
	{
		fprintf(stderr, "RDMA_Write ops\n");
		rc = 1;
		goto main_exit;
	}
	if (rdma_mg->RDMA_Read("34111111111")) /* just send a dummy char back and forth */
	{
		fprintf(stderr, "RDMA_Read ops\n");
		rc = 1;
		goto main_exit;
	}

	rc = 0;
main_exit:
	if (rdma_mg->resources_destroy())
	{
		fprintf(stderr, "failed to destroy resources\n");
		rc = 1;
	}
	if (rdma_mg->config.dev_name)
		free((char *)rdma_mg->config.dev_name);
	fprintf(stdout, "\ntest result is %d\n", rc);
	// return rc;
}

int main(int argc, char *argv[])
{
	work("abcdefg string 1");
	work("xyzxyz string2");
	return 0;
}