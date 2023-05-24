#include <gint/cpu.h>
#include <gint/gdb.h>
#include <gint/usb-ff-bulk.h>
#include <gint/usb.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void gdb_hexlify(char* output_string, const uint8_t* input_buffer, size_t input_size)
{
	const char* hex = "0123456789ABCDEF";
	for (size_t i = 0; i < input_size; i++) {
		uint8_t byte = input_buffer[i];
		output_string[i*2 + 0] = hex[(byte & 0xF0) >> 4];
		output_string[i*2 + 1] = hex[byte & 0x0F];
	}
}

// TODO : bug in fxlibc ? strtoul doesn't support uppercase
static uint32_t gdb_unhexlify(const char* input_string)
{
	size_t input_length = strlen(input_string);
	uint32_t ret = 0;
	for (size_t i = 0; i < input_length; i++) {
		uint8_t nibble_hex = tolower(input_string[i]);
		uint8_t nibble = nibble_hex >= 'a' && nibble_hex <= 'f' ? nibble_hex - 'a' + 10 :
				 nibble_hex >= '0' && nibble_hex <= '9' ? nibble_hex - '0' : 0;
		ret = (ret << 4) | nibble;
	}
	return ret;
}

static bool gdb_started = false;

static void gdb_send(const char *data, size_t size)
{
	usb_fxlink_header_t header;
	usb_fxlink_fill_header(&header, "gdb", "remote", size);

	int pipe = usb_ff_bulk_output();
	usb_write_sync(pipe, &header, sizeof(header), false);
	usb_write_sync(pipe, data, size, false);
	usb_commit_sync(pipe);
}

static char gdb_recv_buffer[1024];
static size_t gdb_recv_buffer_size = 0;
static ssize_t gdb_recv(char *buffer, size_t buffer_size)
{
	if (gdb_recv_buffer_size >= buffer_size) {
		memcpy(buffer, gdb_recv_buffer, buffer_size);
		memmove(gdb_recv_buffer, &gdb_recv_buffer[buffer_size], gdb_recv_buffer_size - buffer_size);
		gdb_recv_buffer_size -= buffer_size;
		return buffer_size;
	}

	usb_fxlink_header_t header;
	while (!usb_fxlink_handle_messages(&header)) {
		sleep();
	}

	// TODO : should we abort or find a way to gracefully shutdown the debugger ?
	if (strncmp(header.application, "gdb", 16) == 0
	     && strncmp(header.type, "remote", 16) == 0) {
		if (header.size > sizeof(gdb_recv_buffer) - gdb_recv_buffer_size) {
			abort();
		}
		usb_read_sync(usb_ff_bulk_input(), &gdb_recv_buffer[gdb_recv_buffer_size], header.size, false);
		gdb_recv_buffer_size += header.size;
		return gdb_recv(buffer, buffer_size);
	} else {
		abort();
	}
}

static ssize_t gdb_recv_packet(char* buffer, size_t buffer_size)
{
	char read_char;

	// Waiting for packet start '$'
	do {
		if (gdb_recv(&read_char, 1) != 1) {
			return -1;
		}
	} while (read_char != '$');

	uint8_t checksum = 0;
	size_t packet_len = 0;
	while (true) {
		if (gdb_recv(&read_char, 1) != 1) {
			return -1;
		}

		if (read_char != '#') {
			// -1 to ensure space for a NULL terminator
			if (packet_len >= (buffer_size - 1)) {
				return -1;
			}
			buffer[packet_len++] = read_char;
			checksum += read_char;
		} else {
			break;
		}
	}
	buffer[packet_len] = '\0';

	char read_checksum_hex[3];
	if (gdb_recv(read_checksum_hex, 2) != 2) {
		return -1;
	}
	read_checksum_hex[2] = '\0';
	uint8_t read_checksum = gdb_unhexlify(read_checksum_hex);

	if (read_checksum != checksum) {
		read_char = '-';
		gdb_send(&read_char, 1);
		return -1;
	} else {
		read_char = '+';
		gdb_send(&read_char, 1);
		return packet_len;
	}
}

static ssize_t gdb_send_packet(const char* packet, size_t packet_length)
{
	if (packet == NULL || packet_length == 0) {
		// Empty packet
		gdb_send("$#00", 4);
		return 4;
	}

	size_t buffer_length = packet_length + 1 + 4;
	// TODO : find if it's more efficient to malloc+copy on each packet or send 3 small fxlink messages
	char* buffer = malloc(buffer_length);

	uint8_t checksum = 0;
	for (size_t i = 0; i < packet_length; i++) {
		checksum += packet[i];
	}

	buffer[0] = '$';
	memcpy(&buffer[1], packet, packet_length);
	snprintf(&buffer[buffer_length - 4], 4, "#%02X", checksum);

	// -1 to not send the NULL terminator of snprintf
	gdb_send(buffer, buffer_length - 1);
	free(buffer);
	return buffer_length;
}

static void gdb_send_stop_reply(void)
{
	gdb_send_packet("S05", 3); // SIGTRAP
}

static void gdb_handle_query_packet(const char* packet)
{
	if (strncmp("qSupported", packet, 10) == 0) {
		const char* qsupported_ans = "PacketSize=255;qXfer:memory-map:read";
		gdb_send_packet(qsupported_ans, strlen(qsupported_ans));
	} else if (strncmp("qXfer:memory-map:read::", packet, 23) == 0) {
		/* TODO : Implement qXfer and memory map XML
		 * https://sourceware.org/gdb/onlinedocs/gdb/Memory-Map-Format.html#Memory-Map-Format
		 * Required for enforcing hbreak : https://sourceware.org/gdb/onlinedocs/gdb/Set-Breaks.html
		 */
		gdb_send_packet(NULL, 0);
	} else {
		gdb_send_packet(NULL, 0);
	}
}

static void gdb_handle_read_general_registers(gdb_cpu_state_t* cpu_state)
{
	char reply_buffer[23*8];
	if (!cpu_state) {
		memset(reply_buffer, 'x', sizeof(reply_buffer));
		memcpy(&reply_buffer[offsetof(gdb_cpu_state_t, reg.pc)*2],
		       "A0000000", 8); // pc needs to be set to make GDB happy
	} else {
		gdb_hexlify(reply_buffer, (uint8_t*)cpu_state->regs,
			    sizeof(cpu_state->regs));
	}
	gdb_send_packet(reply_buffer, sizeof(reply_buffer));
}

static void gdb_handle_read_register(gdb_cpu_state_t* cpu_state, const char* packet)
{
	uint8_t register_id = gdb_unhexlify(&packet[1]);
	char reply_buffer[8];
	if (!cpu_state || register_id >= sizeof(cpu_state->regs)/sizeof(uint32_t)) {
		memset(reply_buffer, 'x', sizeof(reply_buffer));
	} else {
		gdb_hexlify(reply_buffer, (uint8_t*)&cpu_state->regs[register_id],
			    sizeof(cpu_state->regs[register_id]));
	}
	gdb_send_packet(reply_buffer, sizeof(reply_buffer));
}

static void gdb_handle_read_memory(const char* packet)
{
	char address_hex[16] = {0}, size_hex[16] = {0};
	void* read_address;
	size_t read_size;

	packet++; // consume 'm'
	for (size_t i = 0; i < sizeof(address_hex); i++) {
		address_hex[i] = *(packet++); // consume address
		if (*packet == ',') break;
	}
	packet++; // consume ','
	for (size_t i = 0; i < sizeof(size_hex); i++) {
		size_hex[i] = *(packet++); // consume size
		if (*packet == '\0') break;
	}

	read_address = (void*) gdb_unhexlify(address_hex);
	read_size = (size_t) gdb_unhexlify(size_hex);

	// TODO : Detect invalid reads and prevent TLB misses
	char *reply_buffer = malloc(read_size * 2);
	gdb_hexlify(reply_buffer, read_address, read_size);
	gdb_send_packet(reply_buffer, read_size * 2);
	free(reply_buffer);
}

static void gdb_main(gdb_cpu_state_t* cpu_state)
{
	if (cpu_state != NULL) {
		gdb_send_stop_reply();
	}
	while (1) {
		char packet_buffer[256];
		ssize_t packet_size = gdb_recv_packet(packet_buffer, sizeof(packet_buffer));
		if (packet_size <= 0) {
			// TODO : Should we break or log on recv error ?
			continue;
		}

		switch (packet_buffer[0]) {
			case '?': // Halt reason
				gdb_send_stop_reply();
				break;
			case 'q':
				gdb_handle_query_packet(packet_buffer);
				break;

			case 'g':
				gdb_handle_read_general_registers(cpu_state);
				break;
			case 'p':
				gdb_handle_read_register(cpu_state, packet_buffer);
				break;
			case 'm':
				gdb_handle_read_memory(packet_buffer);
				break;

			// case 'G': // Write general register
			// case 'P': // Write register
			// case 'M': // Write memory
			// case 'z': // Insert hbreak
			// case 'Z': // Remove hbreak
			// case 'k': // Kill request
			// case 's': // Single step

			case 'c': // Continue
				return;

			default: // Unsupported packet
				gdb_send_packet(NULL, 0);
				break;
		}
	}
}

int gdb_start(void)
{
	if (gdb_started) {
		return GDB_ALREADY_STARTED;
	}

	if (usb_is_open()) {
		return GDB_NO_INTERFACE;
	}

	usb_interface_t const *interfaces[] = { &usb_ff_bulk, NULL };
	if (usb_open(interfaces, GINT_CALL_NULL) < 0) {
		return GDB_USB_ERROR;
	}
	usb_open_wait();

	gdb_started = true;
	gdb_main(NULL);
	return 0;
}
