/*
 * Test harness for encoding and decoding 802.11u GAS packets.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id:$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proto/802.11.h"
#include "test.h"
#include "trace.h"
#include "pktEncodeGas.h"
#include "pktDecodeGas.h"

TEST_DECLARE();

#define BUFFER_SIZE		256
static uint8 buffer[BUFFER_SIZE];
static bcm_pkt_encode_t enc;

/* --------------------------------------------------------------- */

static void testPktGasRequest(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 req[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		req[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasRequest(&enc, 0x11, 4, apie, 128, req),
		"pktEncodeGasRequest failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_REQUEST_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.request.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.request.reqLen == 128, "decode failed");
}

static void testPktGasResponse(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasResponse(&enc, 0x11, 0x1234, 0x5678, 4, apie, 128, rsp),
		"pktEncodeGasResponse failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_RESPONSE_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.response.statusCode == 0x1234, "decode failed");
	TEST(gasDecode.response.comebackDelay == 0x5678, "decode failed");
	TEST(gasDecode.response.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.response.rspLen == 128, "decode failed");
}

static void testPktGasComebackRequest(void)
{
	bcm_pkt_decode_t dec;
	pktGasDecodeT gasDecode;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasComebackRequest(&enc, 0x11),
		"pktEncodeGasComebackRequest failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_COMEBACK_REQUEST_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
}

static void testPktGasComebackResponse(void)
{
	bcm_pkt_decode_t dec;
	uint8 apie[4] = {108, 2, 0, 0};
	uint8 rsp[128];
	int i;
	pktGasDecodeT gasDecode;

	for (i = 0; i < 128; i++)
		rsp[i] = i;

	TEST(bcm_pkt_encode_init(&enc, BUFFER_SIZE, buffer), "bcm_pkt_encode_init failed");

	TEST(pktEncodeGasComebackResponse(&enc, 0x11, 0x1234, 0xaa, 0x5678, 4, apie, 128, rsp),
		"pktEncodeGasComebackResponse failed");

	TEST(bcm_pkt_decode_init(&dec, bcm_pkt_encode_length(&enc),
		bcm_pkt_encode_buf(&enc)), "bcm_pkt_decode_init failed");

	TEST(pktDecodeGas(&dec, &gasDecode), "pktDecodeGas failed");
	TEST(gasDecode.category == DOT11_ACTION_CAT_PUBLIC, "decode failed");
	TEST(gasDecode.action == GAS_COMEBACK_RESPONSE_ACTION_FRAME, "decode failed");
	TEST(gasDecode.dialogToken == 0x11, "decode failed");
	TEST(gasDecode.comebackResponse.statusCode == 0x1234, "decode failed");
	TEST(gasDecode.comebackResponse.fragmentId == 0xaa, "decode failed");
	TEST(gasDecode.comebackResponse.comebackDelay == 0x5678, "decode failed");
	TEST(gasDecode.comebackResponse.apie.protocolId == 0, "decode failed");
	TEST(gasDecode.comebackResponse.rspLen == 128, "decode failed");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	testPktGasRequest();
	testPktGasResponse();
	testPktGasComebackRequest();
	testPktGasComebackResponse();

	TEST_FINALIZE();
	return 0;
}
