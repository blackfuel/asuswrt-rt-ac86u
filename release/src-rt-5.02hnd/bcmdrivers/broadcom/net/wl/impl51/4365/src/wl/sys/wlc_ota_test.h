/*
* WLOTA feature.
* On a high level there are two modes of operation
* 1. Non Tethered Mode
* 2. Tethered Mode
*
* IN non tethered mode, a cmd flow file which contains encoded test
*	information is downloaded to device.
*	Format of the cmd flow file is pre defined. Host interprest the cmd flow file
*	and passes down a test "structure" to dongle.
*	Reading and parsing can be done by brcm wl utility or any host software which can do
*	the same operation.
*
*	Once cmd flow file is downloaded, a "trigger" cmd is
*	called to put the device into testing mode. It will wait for a sync packet from
* 	tester as a part of handshake mechanism. if device successfully decodes sync packet
*	from an expected mac address, device is good to start with the test sequece.
*	Right now only two kinds of test are downloaded to device.
*		ota_tx
*		ota_rx
*
*	ota_tx/ota_rx takes in arguments as
*	test chan bandwidth contrlchan rates stf txant rxant tx_ifs tx_len num_pkt pwrctrl
*		start:delta:end
*
*	Cmd flow file should have a test setup information like various mac address, sycn timeout.
*	Format is:  synchtimeoout(seconds) synchbreak/loop synchmac txmac rxmac
*
* In tethered mode, test flow is passed down in form of wl iovars through batching mode
*	Sequence of operation is
*	test_stream start	[start batching mode operation]
*	test_stream test_setup  [where test_setup is of the same format in cmd flow file]
*	test_stream test_cmd	[should be of same format of ota_tx /ota_rx in cmd_flow file]
*	test_stream stop	[stops batching mode operation and downloads the file to dongle]
*$Id: wlc_ota_test.h 467328 2014-04-03 01:23:40Z $
* Broadcom Proprietary and Confidential. Copyright (C) 2016,
* All Rights Reserved.
* 
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom.
*/

#ifndef _wlc_ota_test_h_
#define _wlc_ota_test_h_
extern ota_test_info_t *  wlc_ota_test_attach(wlc_info_t* wlc);
extern void wlc_ota_test_detach(ota_test_info_t * ota_info);
#endif /* _wlc_ota_test_h_ */
