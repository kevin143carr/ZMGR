/* Serial Library 1.4 (22-Jun-2000) (c) 1998 Karl Stenerud
 *
 * Renamed ZMGR-facing header for DOS-Serial-Library to avoid colliding with
 * ZMGR's own SERIAL.H on DOS case-insensitive filesystems.
 */

#ifndef SERIAL__HEADER
#define SERIAL__HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* COM Ports */
#define COM_1                           0
#define COM_2                           1
#define COM_3                           2
#define COM_4                           3

/* Handshaking Modes */
#define SER_HANDSHAKING_NONE            0
#define SER_HANDSHAKING_XONXOFF         1
#define SER_HANDSHAKING_RTSCTS          2
#define SER_HANDSHAKING_DTRDSR          3

/* Error Codes */
#define SER_SUCCESS                     0
#define SER_ERR_UNKNOWN                 -1
#define SER_ERR_NOT_OPEN                -2
#define SER_ERR_ALREADY_OPEN            -3
#define SER_ERR_NO_UART                 -4
#define SER_ERR_INVALID_COMPORT         -5
#define SER_ERR_INVALID_BASE            -6
#define SER_ERR_INVALID_IRQ             -7
#define SER_ERR_INVALID_BPS             -8
#define SER_ERR_INVALID_DATA_BITS       -9
#define SER_ERR_INVALID_PARITY          -10
#define SER_ERR_INVALID_STOP_BITS       -11
#define SER_ERR_INVALID_HANDSHAKING     -12
#define SER_ERR_INVALID_FIFO_THRESHOLD  -13
#define SER_ERR_NULL_PTR                -14
#define SER_ERR_IRQ_NOT_FOUND           -15
#define SER_ERR_LOCK_MEM                -16
#define SER_ERR_IRQ_IN_USE              -17

int  serial_open(int com, long bps, int data_bits, char parity, int stop_bits, int handshaking);
int  serial_close(int com);
int  serial_read(int com, char* data, int len);
int  serial_write(int com, const char* data, int len);
int  serial_write_buffered(int com, const char* data, int len);
int  serial_read_direct(int com, char* data, int len);
int  serial_write_direct_ready(int com, const char* data, int len);
int  serial_disable_interrupts(int com);
int  serial_set(int com, long bps, int data_bits, char parity, int stop_bits, int handshaking);
int  serial_set_bps(int com, long bps);
int  serial_set_parity(int com, char parity);
int  serial_set_data(int com, int data);
int  serial_set_stop(int com, int stop);
int  serial_set_handshaking(int com, int handshaking);
int  serial_set_fifo_threshold(int com, int threshold);
int  serial_set_base(int com, int base);
int  serial_set_irq(int com, int base);
int  serial_set_rts(int comport, int rts);
int  serial_set_dtr(int comport, int dtr);
int  serial_set_mcr(int comport, int mcr);
int  serial_get_base(int com);
int  serial_get_irq(int com);
long serial_get_bps(int com);
char serial_get_parity(int com);
int  serial_get_data(int com);
int  serial_get_stop(int com);
int  serial_get_handshaking(int com);
int  serial_get_rts(int comport);
int  serial_get_dtr(int comport);
int  serial_get_cts(int comport);
int  serial_get_dsr(int comport);
int  serial_get_mcr(int comport);
int  serial_get_msr(int comport);
int  serial_get_lsr(int comport);
int  serial_get_tx_buffered(int comport);
int  serial_get_rx_buffered(int comport);
int  serial_clear_tx_buffer(int comport);
int  serial_clear_rx_buffer(int comport);


#ifdef __cplusplus
}
#endif

#endif
