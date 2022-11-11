/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2AP-PDU-Contents"
 * 	found in "./e2ap-v01.01.asn1"
 * 	`asn1c -pdu=all -fcompound-names -gen-PER -no-gen-OER -no-gen-example -fno-include-deps -fincludes-quoted -D E2AP-v01.01/`
 */

#include "E2AP_E2connectionUpdateRemove-Item.h"

asn_TYPE_member_t asn_MBR_E2AP_E2connectionUpdateRemove_Item_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct E2AP_E2connectionUpdateRemove_Item, tnlInformation),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_E2AP_TNLinformation,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"tnlInformation"
		},
};
static const ber_tlv_tag_t asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_E2AP_E2connectionUpdateRemove_Item_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* tnlInformation */
};
asn_SEQUENCE_specifics_t asn_SPC_E2AP_E2connectionUpdateRemove_Item_specs_1 = {
	sizeof(struct E2AP_E2connectionUpdateRemove_Item),
	offsetof(struct E2AP_E2connectionUpdateRemove_Item, _asn_ctx),
	asn_MAP_E2AP_E2connectionUpdateRemove_Item_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_E2AP_E2connectionUpdateRemove_Item = {
	"E2connectionUpdateRemove-Item",
	"E2connectionUpdateRemove-Item",
	&asn_OP_SEQUENCE,
	asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1,
	sizeof(asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1)
		/sizeof(asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1[0]), /* 1 */
	asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1,	/* Same as above */
	sizeof(asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1)
		/sizeof(asn_DEF_E2AP_E2connectionUpdateRemove_Item_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_E2AP_E2connectionUpdateRemove_Item_1,
	1,	/* Elements count */
	&asn_SPC_E2AP_E2connectionUpdateRemove_Item_specs_1	/* Additional specs */
};

