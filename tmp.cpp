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