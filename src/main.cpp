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

struct index_info
{
	char table_name[30];
	uint32_t chunk_id;
	uint8_t index_type;
	vector<uint16_t> column_ids;
	inline bool operator<(const index_info &p) const
	{
		if (0 < strcmp(table_name, p.table_name))
			return true;
		else if (0 == strcmp(table_name, p.table_name) && chunk_id < p.chunk_id)
			return true;
		else if (0 == strcmp(table_name, p.table_name) && chunk_id == p.chunk_id && index_type < p.index_type)
			return true;
		else if (0 == strcmp(table_name, p.table_name) && chunk_id == p.chunk_id && index_type == p.index_type && column_ids.size() < p.column_ids.size())
			return true;
		else if (0 == strcmp(table_name, p.table_name) && chunk_id == p.chunk_id && index_type == p.index_type && column_ids.size() == p.column_ids.size())
		{
			for (int i = 0; i < p.column_ids.size(); i++)
			{
				if (column_ids[i] < p.column_ids[i])
					return true;
				if (column_ids[i] > p.column_ids[i])
					return false;
			}
		}
		return false;
	}
};
map<index_info, char *> memorys;
map<char *, uint64_t> memorys_length;
char *storage, *tail;

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
		// fprintf(stdout, "Message is: '%s', %ld\n", rdma_mg_1->res->buf, strlen(rdma_mg_1->res->buf));
		char in_chars[5];
		int id1, id2;
		uint64_t length = 0ll;
		memcpy(in_chars, rdma_mg_1->res->buf, 5 * sizeof(char));
		length += 5 * sizeof(char);
		// fprintf(stdout, "order name: %s\n", in_chars);
		if (0 == strcmp(in_chars, "read "))
		{
			// fprintf(stdout, "checkpoint1: %s\n", in_chars);
			index_info *now_node = new index_info();
			uint64_t table_name_length;
			memcpy(&table_name_length, (rdma_mg_1->res->buf) + length, sizeof(uint64_t));
			length += sizeof(uint64_t);
			// cout << table_name_length << endl;
			memcpy(now_node->table_name, (rdma_mg_1->res->buf) + length, table_name_length);
			now_node->table_name[table_name_length] = '\0';
			length += table_name_length;
			// printf("%s\n", now_node->table_name);

			memcpy(&(now_node->chunk_id), (rdma_mg_1->res->buf) + length, sizeof(uint32_t));
			length += sizeof(uint32_t);
			memcpy(&(now_node->index_type), (rdma_mg_1->res->buf) + length, sizeof(uint8_t));
			length += sizeof(uint8_t);
			// printf("%d %d\n", now_node->chunk_id, now_node->index_type);

			uint64_t column_ids_length;
			memcpy(&column_ids_length, (rdma_mg_1->res->buf) + length, sizeof(uint64_t));
			length += sizeof(uint64_t);
			now_node->column_ids = vector<uint16_t>(column_ids_length);
			// printf("%ld\n",length);
			memcpy((now_node->column_ids).data(), (rdma_mg_1->res->buf) + length, column_ids_length * sizeof(uint16_t));
			length += column_ids_length * sizeof(uint16_t);

			// for (int ii = 0; ii < now_node->column_ids.size(); ii++)
			// 	printf("columnid %d: %d\n", ii, now_node->column_ids[ii]);

			auto def = memorys.find(*now_node);
			if (def == memorys.end())
			{
				strcpy(rdma_mg_1->res->buf, "failed");
				strcpy(rdma_mg_2->res->buf, "failed");
				// printf("failed to get memory\n");
			}
			else
			{
				uint64_t length = memorys_length.find(def->second)->second;
				memcpy(rdma_mg_2->res->buf, def->second, length);
				strcpy(rdma_mg_1->res->buf, "already");
				// printf("succeeded to get memory\n");
			}
			if (rdma_mg_1->RDMA_Send(MAX_POLL_CQ_TIMEOUT))
			{
				fprintf(stderr, "failed to rdma_mg_1 RDMA_Send\n");
				goto main_exit;
			}
		}
		else if (0 == strcmp(in_chars, "write"))
		{
			// fprintf(stdout, "checkpoint1: %s\n", in_chars);
			index_info *now_node = new index_info();
			uint64_t table_name_length;
			memcpy(&table_name_length, (rdma_mg_1->res->buf) + length, sizeof(uint64_t));
			length += sizeof(uint64_t);
			// cout << table_name_length << endl;
			memcpy(now_node->table_name, (rdma_mg_1->res->buf) + length, table_name_length);
			length += table_name_length;
			// cout << now_node->table_name << endl;
			// printf("%s\n", now_node->table_name);

			memcpy(&now_node->chunk_id, (rdma_mg_1->res->buf) + length, sizeof(uint32_t));
			length += sizeof(uint32_t);
			// printf("%d\n", now_node->chunk_id);
			memcpy(&now_node->index_type, (rdma_mg_1->res->buf) + length, sizeof(uint8_t));
			length += sizeof(uint8_t);
			// printf("%d\n", now_node->index_type);

			uint64_t column_ids_length;
			memcpy(&column_ids_length, (rdma_mg_1->res->buf) + length, sizeof(uint64_t));
			// printf("%ld\n", column_ids_length);
			length += sizeof(uint64_t);
			now_node->column_ids = vector<uint16_t>(column_ids_length);
			memcpy(now_node->column_ids.data(), (rdma_mg_1->res->buf) + length, column_ids_length * sizeof(uint16_t));
			length += column_ids_length * sizeof(uint16_t);
			// for (int ii = 0; ii < now_node->column_ids.size(); ii++)
			// 	printf("columnid %d: %d\n", ii, now_node->column_ids[ii]);
			if (rdma_mg_2->RDMA_Write()) /* just send a dummy char back and forth */
			{
				fprintf(stderr, "rdma_mg_2 RDMA_Write ops\n");
				rc = 1;
				goto main_exit;
			}
			uint64_t r2_length;
			memcpy(&r2_length, rdma_mg_2->res->buf, sizeof(uint64_t));
			r2_length += sizeof(uint64_t);
			printf("r2_length: %ld\n", r2_length);
			memorys.insert(pair<index_info, char *>{*now_node, tail});
			memorys_length.insert(pair<char *, uint64_t>{tail, r2_length});
			// printf("check map find: %d\n", memorys.end() == memorys.find(*now_node));

			memcpy(tail, rdma_mg_2->res->buf, r2_length);
			tail = tail + r2_length;
		}
		else
		{
			fprintf(stdout, "wrong option received\n");
		}

		if (rdma_mg_1->sock_sync_data(rdma_mg_1->res->sock, 1, temp_send, &temp_char)) /* just send a dummy char back and forth */
		{
			fprintf(stderr, "sync 1 error after RDMA ops\n");
			rc = 1;
			goto main_exit;
		}
		if (rdma_mg_2->sock_sync_data(rdma_mg_2->res->sock, 1, temp_send, &temp_char)) /* just send a dummy char back and forth */
		{
			fprintf(stderr, "sync 2 error after RDMA ops\n");
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
	uint64_t SIZE = 1024;
	storage = (char *)malloc(SIZE * SIZE * SIZE * 16);
	tail = storage;
	work();
	return 0;
}