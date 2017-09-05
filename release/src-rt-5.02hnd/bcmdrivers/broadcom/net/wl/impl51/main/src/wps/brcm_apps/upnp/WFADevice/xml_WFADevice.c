/*
 * Broadcom WPS module (for libupnp), xml_WFADevice.c
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2016,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: xml_WFADevice.c 551899 2015-04-24 11:55:46Z $
 */

char xml_WFADevice[] =
	"<?xml version=\"1.0\"?>\r\n"
	"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n"
	"\t<specVersion>\r\n"
	"\t\t<major>1</major>\r\n"
	"\t\t<minor>0</minor>\r\n"
	"\t</specVersion>\r\n"
	"\t<device>\r\n"
	"\t\t<deviceType>urn:schemas-wifialliance-org:device:WFADevice:1</deviceType>\r\n"
	"\t\t<friendlyName>ASUS WPS Router</friendlyName>\r\n"
	"\t\t<manufacturer>ASUSTeK Corporation</manufacturer>\r\n"
	"\t\t<manufacturerURL>http://www.asus.com</manufacturerURL>\r\n"
	"\t\t<modelDescription>ASUS WPS Router</modelDescription>\r\n"
	"\t\t<modelName>WPS Router</modelName>\r\n"
	"\t\t<modelNumber>X1</modelNumber>\r\n"
	"\t\t<serialNumber>0000001</serialNumber>\r\n"
	"\t\t<UDN>uuid:00010203-0405-0607-0809-0a0b0c0d0ebb</UDN>\r\n"
	"\t\t<serviceList>\r\n"
	"\t\t\t<service>\r\n"
	"\t\t\t\t<serviceType>urn:schemas-wifialliance-org:service:WFAWLANConfig:1</serviceType>\r\n"
	"\t\t\t\t<serviceId>urn:wifialliance-org:serviceId:WFAWLANConfig1</serviceId>\r\n"
	"\t\t\t\t<SCPDURL>x_wfawlanconfig.xml</SCPDURL>\r\n"
	"\t\t\t\t<controlURL>control?WFAWLANConfig</controlURL>\r\n"
	"\t\t\t\t<eventSubURL>event?WFAWLANConfig</eventSubURL>\r\n"
	"\t\t\t</service>\r\n"
	"\t\t</serviceList>\r\n"
	"\t</device>\r\n"
	"</root>\r\n"
	"\r\n";
