/*
 * Test harness for encoding and decoding Hotspot2.0 ANQP packets.
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
#include "test.h"
#include "trace.h"
#include "bcm_encode_hspot_anqp.h"
#include "bcm_decode_hspot_anqp.h"

TEST_DECLARE();

#define BUFFER_SIZE		256
static uint8 buffer[BUFFER_SIZE * 4];
static bcm_encode_t enc;

/* --------------------------------------------------------------- */

static void testEncode(void)
{
	uint8 data[8];
	int i;

	for (i = 0; i < 8; i++)
		data[i] = i;

	TEST(bcm_encode_init(&enc, sizeof(buffer), buffer), "bcm_encode_init failed");

	TEST(bcm_encode_hspot_anqp_query_list(&enc, 8, data),
		"bcm_encode_hspot_anqp_query_list failed");
	WL_PRPKT("hotspot query list",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	TEST(bcm_encode_hspot_anqp_capability_list(&enc, 8, data),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot capability list",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		TEST(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "EN", 6, "myname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 2, "FR", 10, "helloworld"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");
		TEST(bcm_encode_hspot_anqp_operator_name_duple(&name, 5, "JAPAN", 6, "yrname"),
			"bcm_encode_hspot_anqp_operator_name_duple failed");

		TEST(bcm_encode_hspot_anqp_operator_friendly_name(&enc,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"bcm_encode_hspot_anqp_operator_friendly_name failed");
		WL_PRPKT("hotspot operator friendly name",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

	TEST(bcm_encode_hspot_anqp_wan_metrics(&enc,
		HSPOT_WAN_LINK_TEST, HSPOT_WAN_SYMMETRIC_LINK, HSPOT_WAN_AT_CAPACITY,
		0x12345678, 0x11223344, 0xaa, 0xbb, 0xcdef),
		"bcm_encode_hspot_anqp_capability_list failed");
	WL_PRPKT("hotspot WAN metrics",
		bcm_encode_buf(&enc), bcm_encode_length(&enc));

	{
		uint8 capBuf[BUFFER_SIZE];
		bcm_encode_t cap;

		TEST(bcm_encode_init(&cap, BUFFER_SIZE, capBuf), "bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 1, 0, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 20, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 22, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 80, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 443, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 1723, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 6, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 5060, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");
		TEST(bcm_encode_hspot_anqp_proto_port_tuple(&cap, 17, 4500, HSPOT_CC_STATUS_OPEN),
			"bcm_encode_hspot_anqp_proto_port_tuple failed");

		TEST(bcm_encode_hspot_anqp_connection_capability(&enc,
			bcm_encode_length(&cap), bcm_encode_buf(&cap)),
			"bcm_encode_hspot_anqp_connection_capability failed");
		WL_PRPKT("hotspot connection capability",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

	{
		uint8 nameBuf[BUFFER_SIZE];
		bcm_encode_t name;

		TEST(bcm_encode_init(&name, BUFFER_SIZE, nameBuf),
			"bcm_encode_init failed");

		TEST(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 0, 5, "hello"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");
		TEST(bcm_encode_hspot_anqp_nai_home_realm_name(&name, 1, 5, "world"),
			"bcm_encode_hspot_anqp_nai_home_realm_name failed");

		TEST(pktEncodeHspotAnqpNaiHomeRealmQuery(&enc, 2,
			bcm_encode_length(&name), bcm_encode_buf(&name)),
			"pktEncodeHspotAnqpNaiHomeRealmQuery failed");
		WL_PRPKT("hotspot NAI home realm query",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

	{
		/* Testing range of operating classes */
		uint8 opClass [35] = {80, 81, 82, 83, 84, 94, 95, 96, 101, 102, 103, 104, 105, 106,
			107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
		122, 123, 124, 125, 126, 127};

		TEST(bcm_encode_hspot_anqp_operating_class_indication(&enc, 35, opClass),
			"bcm_encode_hspot_anqp_operating_class_indication failed");
		WL_PRPKT("hotspot operating class indication",
			bcm_encode_buf(&enc), bcm_encode_length(&enc));
	}

}

static void testDecode(void)
{
	bcm_decode_t dec;
	bcm_decode_hspot_anqp_t hspot;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	TEST(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 7, "bcm_decode_hspot_anqp failed");

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_query_list_t queryList;

		TEST(bcm_decode_init(&ie, hspot.queryListLength,
			hspot.queryListBuffer), "bcm_decode_init failed");
		TEST(bcm_decode_hspot_anqp_query_list(&ie, &queryList),
			"bcm_decode_hspot_anqp_query_list failed");
		TEST(queryList.queryLen == 8, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_capability_list_t capList;

		TEST(bcm_decode_init(&ie, hspot.capabilityListLength,
			hspot.capabilityListBuffer), "bcm_decode_init failed");
		TEST(bcm_decode_hspot_anqp_capability_list(&ie, &capList),
			"bcm_decode_hspot_anqp_capability_list failed");
		TEST(capList.capLen == 8, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_wan_metrics_t wanMetrics;

		TEST(bcm_decode_init(&ie, hspot.wanMetricsLength,
			hspot.wanMetricsBuffer), "bcm_decode_init failed");
		TEST(bcm_decode_hspot_anqp_wan_metrics(&ie, &wanMetrics),
			"pktHspotAnqpDecodeWanMetrics failed");
		TEST(wanMetrics.linkStatus == HSPOT_WAN_LINK_TEST, "invalid data");
		TEST(wanMetrics.symmetricLink == HSPOT_WAN_SYMMETRIC_LINK, "invalid data");
		TEST(wanMetrics.atCapacity == HSPOT_WAN_AT_CAPACITY, "invalid data");
		TEST(wanMetrics.dlinkSpeed == 0x12345678, "invalid data");
		TEST(wanMetrics.ulinkSpeed == 0x11223344, "invalid data");
		TEST(wanMetrics.dlinkLoad == 0xaa, "invalid data");
		TEST(wanMetrics.ulinkLoad == 0xbb, "invalid data");
		TEST(wanMetrics.lmd == 0xcdef, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_operator_friendly_name_t op;

		TEST(bcm_decode_init(&ie, hspot.operatorFriendlyNameLength,
			hspot.operatorFriendlyNameBuffer), "bcm_decode_init failed");

		TEST(bcm_decode_hspot_anqp_operator_friendly_name(&ie, &op),
			"bcm_decode_hspot_anqp_operator_friendly_name failed");
		TEST(op.numName == 3, "invalid data");

		TEST(op.duple[0].langLen == 2, "invalid data");
		TEST(strcmp(op.duple[0].lang, "EN") == 0, "invalid data");
		TEST(op.duple[0].nameLen == 6, "invalid data");
		TEST(strcmp(op.duple[0].name, "myname") == 0, "invalid data");

		TEST(op.duple[1].langLen == 2, "invalid data");
		TEST(strcmp(op.duple[1].lang, "FR") == 0, "invalid data");
		TEST(op.duple[1].nameLen == 10, "invalid data");
		TEST(strcmp(op.duple[1].name, "helloworld") == 0, "invalid data");

		TEST(op.duple[2].langLen == 3, "invalid data");
		TEST(strcmp(op.duple[2].lang, "JAP") == 0, "invalid data");
		TEST(op.duple[2].nameLen == 6, "invalid data");
		TEST(strcmp(op.duple[2].name, "yrname") == 0, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_connection_capability_t cap;

		TEST(bcm_decode_init(&ie, hspot.connectionCapabilityLength,
			hspot.connectionCapabilityBuffer), "bcm_decode_init failed");

		TEST(bcm_decode_hspot_anqp_connection_capability(&ie, &cap),
			"pktDecodeHspotAnqpAnqpConnectionCapability failed");
		TEST(cap.numConnectCap == 10, "invalid data");

		TEST(cap.tuple[0].ipProtocol == 1, "invalid data");
		TEST(cap.tuple[0].portNumber == 0, "invalid data");
		TEST(cap.tuple[0].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[1].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[1].portNumber == 20, "invalid data");
		TEST(cap.tuple[1].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[2].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[2].portNumber == 22, "invalid data");
		TEST(cap.tuple[2].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[3].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[3].portNumber == 80, "invalid data");
		TEST(cap.tuple[3].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[4].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[4].portNumber == 443, "invalid data");
		TEST(cap.tuple[4].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[5].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[5].portNumber == 1723, "invalid data");
		TEST(cap.tuple[5].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[6].ipProtocol == 6, "invalid data");
		TEST(cap.tuple[6].portNumber == 5060, "invalid data");
		TEST(cap.tuple[6].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[7].ipProtocol == 17, "invalid data");
		TEST(cap.tuple[7].portNumber == 500, "invalid data");
		TEST(cap.tuple[7].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[8].ipProtocol == 17, "invalid data");
		TEST(cap.tuple[8].portNumber == 5060, "invalid data");
		TEST(cap.tuple[8].status == HSPOT_CC_STATUS_OPEN, "invalid data");

		TEST(cap.tuple[9].ipProtocol == 17, "invalid data");
		TEST(cap.tuple[9].portNumber == 4500, "invalid data");
		TEST(cap.tuple[9].status == HSPOT_CC_STATUS_OPEN, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_nai_home_realm_query_t realm;

		TEST(bcm_decode_init(&ie, hspot.naiHomeRealmQueryLength,
			hspot.naiHomeRealmQueryBuffer), "bcm_decode_init failed");

		TEST(bcm_decode_hspot_anqp_nai_home_realm_query(&ie, &realm),
			"bcm_decode_hspot_anqp_nai_home_realm_query failed");
		TEST(realm.count == 2, "invalid data");

		TEST(realm.data[0].encoding == 0, "invalid data");
		TEST(realm.data[0].nameLen == 5, "invalid data");
		TEST(strcmp(realm.data[0].name, "hello") == 0, "invalid data");

		TEST(realm.data[1].encoding == 1, "invalid data");
		TEST(realm.data[1].nameLen == 5, "invalid data");
		TEST(strcmp(realm.data[1].name, "world") == 0, "invalid data");
	}

	{
		bcm_decode_t ie;
		bcm_decode_hspot_anqp_operating_class_indication_t opClassList;

		TEST(bcm_decode_init(&ie, hspot.opClassIndicationLength,
			hspot.opClassIndicationBuffer), "bcm_decode_init failed");
		TEST(bcm_decode_hspot_anqp_operating_class_indication(&ie, &opClassList),
			"pktDecodeHspotOperatingClassIndication failed");
		TEST(opClassList.opClassLen == 35, "invalid data");
	}

}

static void testDecodeCorruptLength(void)
{
	bcm_decode_t dec;
	bcm_decode_hspot_anqp_t hspot;
	uint8 *lenPtr;
	uint8 save;

	TEST(bcm_decode_init(&dec, bcm_encode_length(&enc),
		bcm_encode_buf(&enc)), "bcm_decode_init failed");
	WL_PRPKT("decode packet", bcm_decode_buf(&dec), bcm_decode_buf_length(&dec));

	lenPtr = &bcm_decode_buf(&dec)[2];
	save = *lenPtr;
	*lenPtr += 1;
	TEST(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 1, "bcm_decode_hspot_anqp failed");
	*lenPtr = 0x02;
	TEST(bcm_decode_hspot_anqp(&dec, TRUE, &hspot) == 0, "bcm_decode_hspot_anqp failed");
	*lenPtr = save;
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	TRACE_LEVEL_SET(TRACE_ALL);
	TEST_INITIALIZE();

	testEncode();
	testDecode();
	testDecodeCorruptLength();

	TEST_FINALIZE();
	return 0;
}
