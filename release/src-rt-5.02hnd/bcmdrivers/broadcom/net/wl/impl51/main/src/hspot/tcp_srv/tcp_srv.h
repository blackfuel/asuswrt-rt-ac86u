/*
 * TCP socket for running sigma.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id:$
 */

typedef  void (*tcpHandlerT)(void *handle, char *rxData, char *txData);

int tcpSrvCreate(int port);
void tcpSrvDestroy(void);
int tcpSetSelectFds(fd_set *rfds);
int tcpProcessSelect(fd_set *rfds);
char * TcpGetClientBuffer(void);
void TcpSendBuffer(char*, char *);
void tcpSubscribeTcpHandler(void *handle, tcpHandlerT tcpReceiveHandler);
void tcpSetLockRead(int);
