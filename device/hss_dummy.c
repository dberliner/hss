#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/hss.h>
#include <net/hss.h>

static struct hss_packet *resp_pkt;
static const char *resp_msg = "Const Resp\n\n";
//NOTE: The packet intentinally has no NULL terminator. We're not passing a C string
#define packet_len sizeof(struct hss_packet_hdr) + strlen(resp_msg) 
static void *proxy_ctx;
static struct hss_packet_hdr close_pkt = {
	.opcode = HSS_OP_CLOSE,
};
static struct hss_packet ack_pkt = {
	.hdr = {
		.opcode = HSS_OP_ACK,
		.payload_len = 3
	}
};

void hss_cmd(char *cmd , size_t size, void* context)
{
	struct hss_packet *pkt = (struct hss_packet *)cmd;
	(void)context;
	print_hex_dump(KERN_INFO, "CMD ", DUMP_PREFIX_NONE, 16, 1,
		cmd, size, true);

	/* Tell the sender that the the sent command was a success */
	ack_pkt.hdr.sock_id = (pkt->hdr.opcode == HSS_OP_OPEN) ? pkt->open.handle : pkt->hdr.sock_id;
	ack_pkt.hdr.msg_id = pkt->hdr.msg_id;
	ack_pkt.ack.orig_opcode = pkt->hdr.opcode;
	ack_pkt.ack.code = HSS_E_SUCCESS;
	hss_proxy_rcv_cmd((struct hss_packet*)&ack_pkt, sizeof(ack_pkt), proxy_ctx);
}
void hss_transfer(struct hss_packet_hdr *hdr, char* msg, size_t size, void* context)
{
	(void)context;

	
	print_hex_dump(KERN_INFO, "TRN1 ", DUMP_PREFIX_NONE, 16, 1,
		hdr, sizeof(struct hss_packet_hdr), true);
	print_hex_dump(KERN_INFO, "TRN2 ", DUMP_PREFIX_NONE, 16, 1,
		msg, size, true);

	/* Tell the sender that the TRANSMIT was a success */
	ack_pkt.hdr.sock_id = hdr->sock_id;
	ack_pkt.hdr.msg_id = hdr->msg_id;
	ack_pkt.ack.orig_opcode = HSS_OP_TRANSMIT;
	ack_pkt.ack.code = HSS_E_SUCCESS;
	hss_proxy_rcv_cmd((struct hss_packet *)&ack_pkt, sizeof(ack_pkt), proxy_ctx);

	/* Send a message back */
	resp_pkt->hdr.sock_id = hdr->sock_id;
	hss_proxy_rcv_data((struct hss_packet *)&resp_pkt, packet_len, proxy_ctx);

	/* Close the socket */
	close_pkt.msg_id = hdr->msg_id;
	close_pkt.sock_id = hdr->sock_id;
	//hss_proxy_rcv_cmd((struct hss_packet *)&close_pkt,
	//		sizeof(close_pkt), proxy_ctx); 
}

static struct hss_usb_descriptor hss_usb_intf = {
	.hss_cmd=hss_cmd,
	.hss_transfer=hss_transfer
};


/**
 * Cleanup and unregister registred types
 */
static int __init load_dummy_module(void)
{
        printk("%s INIT\n", __func__);
	proxy_ctx = hss_proxy_init(NULL, &hss_usb_intf);
	
	resp_pkt = kzalloc(packet_len, GFP_KERNEL);
	if (resp_pkt) {
		resp_pkt->hdr.opcode = HSS_OP_TRANSMIT;
		resp_pkt->hdr.payload_len = strlen(resp_msg);
		strcpy(resp_pkt->hss_payload_none, resp_msg);
	}

	printk(KERN_INFO "HSS Dummy Interface loaded\n");
	
        return 0;
}

static void __exit unload_dummy_module(void)
{
        printk("%s ENTER\n", __func__);
	hss_proxy_unregister(NULL);
	kfree(resp_pkt);
        printk("%s EXIT\n", __func__);
}

module_init(load_dummy_module);
module_exit(unload_dummy_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Daniel Berliner");
MODULE_DESCRIPTION("HSS Driver Dummy Handler");
MODULE_VERSION("0.0.1");
