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
using namespace rapidjson;

string vector2string(vector<vector<string>> &file, int id1, int id2)
{
	string out = "{";
	long unsigned i, j;
	for (i = id1; i < file.size() && i <= id2; i++)
	{
		string outp = "{";
		for (j = 0; j < file[i].size(); j++)
		{
			outp += to_string(j) + ":" + file[i][j];
			if (j != file[i].size() - 1)
			{
				outp += ",";
			}
		}
		outp += "}";
		out += outp;
		if (i < id2)
		{
			out += ",";
		}
	}
	out += "}";
	cout << "vector2string " << out << endl;
	return out;
}

struct art_store
{
	vector<uint32_t>::size_type chunkoffsetsize;
	vector<uint32_t> y;
} __attribute__((packed));


void work(vector<vector<string>> &file)
{
	bool config_flag = true;
	int rc = 1;
	char temp_char;
	char temp_send[] = "R";

	// art_store vecTemp;
	// vector<uint32_t> y{5};
	// vecTemp = (art_store){y.size(), y};

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
		/* after polling the completion we have the message in the client buffer too */
		fprintf(stdout, "Message is: '%s', %ld\n", rdma_mg_1->res->buf, strlen(rdma_mg_1->res->buf));
		char in_chars[30];
		int id1, id2;
		strcpy(in_chars, rdma_mg_1->res->buf);
		char *sub_chars = strtok(in_chars, ",");
		fprintf(stdout, "order name: %s\n", sub_chars);
		if (0 == strcmp(sub_chars, "read"))
		{
			// scanf("%d", &id1);
			// id1 = stoi(strtok(NULL, ","));
			// id2 = stoi(strtok(NULL, ","));
			// fprintf(stdout, "Message is: '%s', %d, %d\n", sub_chars, id1, id2);
			// string out_string = "write " + vector2string(file, id1, id2);
			// char *out_chars = const_cast<char *>(out_string.c_str());
			// strcpy(rdma_mg_2->res->buf, out_chars);

			// fprintf(stdout, "Now replacing it with: '%s'\n", rdma_mg_2->res->buf);
			// ifstream isData("data.dat", ios_base::in | ios_base::binary);
			// isData.read((char *)(rdma_mg_2->res->buf), sizeof(id1));
			// isData.close();
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

			// fprintf(stdout, "Contents of server's buffer: %s %ld\n", rdma_mg_2->res->buf, strlen(rdma_mg_2->res->buf));
			// uint64_t a;
			// memcpy(&a, rdma_mg_2->res->buf, sizeof(uint64_t));
			// printf("%ld\n", a);
			// printf("%ld\n",sizeof(size_t));
			// size_t b, c;
			// memcpy(&b, rdma_mg_2->res->buf + a + sizeof(a) - sizeof(size_t), sizeof(size_t));
			// printf("%ld\n", b);
			// memcpy(&c, rdma_mg_2->res->buf + a + sizeof(a) - sizeof(size_t) * 2, sizeof(size_t));
			// printf("%ld\n", c);
			// art_store vecTemp2;
			// memcpy(&vecTemp2, (rdma_mg_2->res->buf),sizeof(rdma_mg_2->res->buf));
			// fprintf(stdout, "Contents of server's buffer: %ld, %d\n", vecTemp2.chunkoffsetsize, vecTemp2.y[0]);
			// uint64_t length;
			// auto temp_pointer = rdma_mg_2->res->buf;
			// int x;

			// memcpy(&length, temp_pointer, sizeof(length));
			// temp_pointer += sizeof(length);
			// printf("length: %d\n", length);

			// memcpy(&x, temp_pointer, sizeof(x));
			// printf("x: %d\n", x);
			// temp_pointer += sizeof(x);
			// vector<uint32_t> y(x);
			// memcpy(y.data(), temp_pointer, sizeof(y.data()));
			// printf("y[0]: %d\n", y[0]);

			// ofstream osData("data.dat", ios_base::out | ios_base::binary);
			// osData.write((char *)(rdma_mg_2->res->buf), sizeof(length)+length);
			// cout << sizeof(rdma_mg_2->res->buf) << endl;
			// osData.close();
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
	// return rc;
}
#include <map>

void csv_data_load(char *filename, vector<vector<string>> &file)
{
	ifstream csv_data(filename);
	string line;
	if (!csv_data.is_open())
	{
		cout << "Error: opening file fail" << endl;
		exit(1);
	}
	istringstream sin; //将整行字符串line读入到字符串istringstream中
	// vector<vector<string>> file;
	vector<string> words; //声明一个字符串向量
	string word;

	// 读取标题行
	// getline(csv_data, line);
	// 读取数据
	while (getline(csv_data, line))
	{
		sin.clear();
		sin.str(line);
		words.clear();
		while (getline(sin, word, ',')) //将字符串流sin中的字符读到field字符串中，以逗号为分隔符
		{
			words.push_back(word); //将每一格中的数据逐个push
								   // cout << word;
								   // cout << atol(word.c_str());
		}
		file.push_back(words);
		// cout << endl;
		// do something。。。
	}
	csv_data.close();
}

void csv_write()
{
	// char *fn = "madedata.csv";
	ofstream outFile;
	outFile.open("loaddata.csv", ios::out | ios::trunc);
	for (int x = 1; x <= 10000; x++)
	{
		std::string str = "";
		for (int i = 1; i <= 50; i++)
		{
			int flag;
			flag = rand() % 2;						   //随机使flag为1或0，为1就是大写，为0就是小写
			if (flag == 1)							   //如果flag=1
				str += rand() % ('Z' - 'A' + 1) + 'A'; //追加大写字母的ascii码
			else
				str += rand() % ('z' - 'a' + 1) + 'a'; //如果flag=0，追加为小写字母的ascii码
		}
		outFile << str << "," << to_string(x * 10000 + x) << endl;
	}
	outFile.close();
}

int mainp(int argc, char *argv[])
{
	// csv_write();
	vector<vector<string>> file;
	csv_data_load("loaddata.csv", file);
	work(file);
	// work(file);
	return 0;
}