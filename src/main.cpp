#include <iostream>
#include <istream>
#include <streambuf>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include "rdma.h"
#include <map>

using namespace std;

void work()
{
	bool config_flag = true;
	int rc = 1;
	char temp_char;
	char temp_send[] = "R";

	shared_ptr<RDMAEngine::RDMA_Manager> rdma_mg_1 = std::shared_ptr<RDMAEngine::RDMA_Manager>();
	rdma_mg_1 = std::make_shared<RDMAEngine::RDMA_Manager>(config_flag);
	shared_ptr<RDMAEngine::RDMA_Manager> rdma_mg_2 = std::shared_ptr<RDMAEngine::RDMA_Manager>();
	rdma_mg_2 = std::make_shared<RDMAEngine::RDMA_Manager>(config_flag);
	if (rdma_mg_1->init())
	{
		fprintf(stderr, "failed to init resources\n");
		rc = 1;
		goto main_exit;
	}

	if (rdma_mg_2->init())
	{
		fprintf(stderr, "failed to init resources\n");
		rc = 1;
		goto main_exit;
	}
	while (1)
	{
		if (rdma_mg_1->RDMA_Receive(MAX_POLL_CQ_TIMEOUT))
		{
			fprintf(stderr, "failed to rdma_mg_1 RDMA_Receive\n");
			goto main_exit;
		}
		fprintf(stdout, "Message is: '%s', %ld\n", rdma_mg_1->res->buf, strlen(rdma_mg_1->res->buf));
		char in_chars[30];
		int id1, id2;
		strcpy(in_chars, rdma_mg_1->res->buf);
		char *sub_chars = strtok(in_chars, ",");
		fprintf(stdout, "order name: %s\n", sub_chars);
		if (0 == strcmp(sub_chars, "read"))
		{
			strcpy(rdma_mg_1->res->buf, "already");
			if (rdma_mg_1->RDMA_Send(MAX_POLL_CQ_TIMEOUT))
			{
				fprintf(stderr, "failed to rdma_mg_1 RDMA_Send\n");
				goto main_exit;
			}
		}
		else if (0 == strcmp(sub_chars, "write"))
		{
			fprintf(stdout, "checkpoint1: %s\n", sub_chars);
			if (rdma_mg_2->RDMA_Write()) /* just send a dummy char back and forth */
			{
				fprintf(stderr, "rdma_mg_2 RDMA_Write ops\n");
				rc = 1;
				goto main_exit;
			}
		}
		else
		{
			fprintf(stdout, "wrong option received\n");
		}

		if (rdma_mg_1->sock_sync_data(rdma_mg_1->res->sock, 1, temp_send, &temp_char)) /* just send a dummy char back and forth */
		{
			fprintf(stderr, "sync error after RDMA ops\n");
			rc = 1;
			goto main_exit;
		}
		if (rdma_mg_2->sock_sync_data(rdma_mg_2->res->sock, 1, temp_send, &temp_char)) /* just send a dummy char back and forth */
		{
			fprintf(stderr, "sync error after RDMA ops\n");
			rc = 1;
			goto main_exit;
		}
	}

	rc = 0;
main_exit:
	if (rdma_mg_1->resources_destroy())
	{
		fprintf(stderr, "failed to destroy resources\n");
		rc = 1;
	}
	if (rdma_mg_2->resources_destroy())
	{
		fprintf(stderr, "failed to destroy resources\n");
		rc = 1;
	}
	if (rdma_mg_1->config.dev_name)
		free((char *)rdma_mg_1->config.dev_name);
	if (rdma_mg_2->config.dev_name)
		free((char *)rdma_mg_2->config.dev_name);
	fprintf(stdout, "\ntest result is %d\n", rc);
}
int main(int argc, char *argv[])
{
	work();
	return 0;
}