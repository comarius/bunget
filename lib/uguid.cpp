/**
    Copyright: zirexix 2016

    This program is distributed
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    The library or code from is restricted to be used in commercial products
    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org.

*/

#include "uguid.h"



uint128_t  GAT_MASK = {0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x80,0x00,0x00,0x80,0x5F,0x9B,0x34,0xFB};



int bt_uuid16_create(bt_uuid_t *btuuid, uint16_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = bt_uuid_t::BT_UUID16;
	btuuid->value.u16 = value;

	return 0;
}

int bt_uuid32_create(bt_uuid_t *btuuid, uint32_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = bt_uuid_t::BT_UUID32;
	btuuid->value.u32 = value;

	return 0;
}

int bt_uuid128_create(bt_uuid_t *btuuid, uint128_t value)
{
	memset(btuuid, 0, sizeof(bt_uuid_t));
	btuuid->type = bt_uuid_t::BT_UUID128;
	btuuid->value.u128 = value;

	return 0;
}

static inline int is_uuid128(const char *suid)
{
	return (strlen(suid) == 36);
}

static inline int is_base_uuid128(const char *suid)
{
	uint16_t uuid;
	char dummy[2];

	if (!is_uuid128(suid))
		return 0;

	return sscanf(suid,
		"0000%04hx-0000-1000-8000-00805%1[fF]9%1[bB]34%1[fF]%1[bB]",
		&uuid, dummy, dummy, dummy, dummy) == 5;
}

static inline int is_uuid32(const char *suid)
{
	return (strlen(suid) == 8 || strlen(suid) == 10);
}

static inline int is_uuid16(const char *suid)
{
	return (strlen(suid) == 4 || strlen(suid) == 6);
}

static int bt_string_to_uuid16(bt_uuid_t *uuid, const char *suid)
{
	uint16_t u16;
	char *endptr = NULL;

	u16 = strtol(suid, &endptr, 16);
	if (endptr && (*endptr == '\0' || *endptr == '-')) {
		bt_uuid16_create(uuid, u16);
		return 0;
	}

	return -EINVAL;
}

static int bt_string_to_uuid32(bt_uuid_t *uuid, const char *suid)
{
	uint32_t u32;
	char *endptr = NULL;

	u32 = strtol(suid, &endptr, 16);
	if (endptr && *endptr == '\0') {
		bt_uuid32_create(uuid, u32);
		return 0;
	}

	return -EINVAL;
}

static int bt_string_to_uuid128(bt_uuid_t *uuid, const char *suid)
{
	uint32_t data0, data4;
	uint16_t data1, data2, data3, data5;
	uint128_t u128;
	uint8_t *val = (uint8_t *) &u128;

	if (sscanf(suid, "%08x-%04hx-%04hx-%04hx-%08x%04hx",
				&data0, &data1, &data2,
				&data3, &data4, &data5) != 6)
		return -EINVAL;

	data0 = htonl(data0);
	data1 = htons(data1);
	data2 = htons(data2);
	data3 = htons(data3);
	data4 = htonl(data4);
	data5 = htons(data5);

	memcpy(&val[0], &data0, 4);
	memcpy(&val[4], &data1, 2);
	memcpy(&val[6], &data2, 2);
	memcpy(&val[8], &data3, 2);
	memcpy(&val[10], &data4, 4);
	memcpy(&val[14], &data5, 2);

	bt_uuid128_create(uuid, u128);

	return 0;
}

int bt_string_to_uuid(bt_uuid_t *uuid, const char *suid)
{
	//if (is_base_uuid128(suid))
	//	return bt_string_to_uuid16(uuid, suid + 4);
	//else
	if (is_uuid128(suid))
		return bt_string_to_uuid128(uuid, suid);
	else if (is_uuid32(suid))
		return bt_string_to_uuid32(uuid, suid);
	else if (is_uuid16(suid))
		return bt_string_to_uuid16(uuid, suid);

	return -EINVAL;
}
