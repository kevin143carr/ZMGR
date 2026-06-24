#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include "DEFINES.H"

class SerialManager
{
private:
    unsigned int get_com_base(int port_number);
    int uart_present(unsigned int base);
    int detect_uart_type(unsigned int base);
    int get_standard_irq(int port_number);
    long get_recommended_baud(int uart_type);
    const char *get_uart_type_name(int uart_type);
    const char *get_baud_note(int uart_type);
    int data_ready(unsigned int base);
    int transmit_ready(unsigned int base);
    void initialize_port(unsigned int base, long baud);
    void flush_receive(unsigned int base);
    void shutdown_port(unsigned int base);
    void write_byte(unsigned int base, unsigned char value);
    unsigned char read_byte(unsigned int base);
    int mouse_driver_present(void);
    int get_mouse_type_irq(int *mouse_type, int *irq, int *major_version, int *minor_version);
    int infer_mouse_com_from_irq(int irq);
    void print_irq_vectors(void);
    int get_dos_major_version(void);
    const char *get_mouse_type_name(int mouse_type);
    int run_msd_report(const char *report_file);
    int parse_msd_report(const char *report_file);
    void show_msd_fallback(void);

public:
    void show_diagnostics(void);
    void run_who_is_there(void);
    int passthrough(int port_a, int port_b, int watch_zmodem, int handshaking);
    int passthrough_until_idle_after_traffic(int port_a, int port_b, int watch_zmodem, int idle_seconds, int max_wait_seconds);
};

#endif
