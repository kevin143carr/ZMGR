#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include "DOSSER.H"
#include "SERIAL.H"
#include "UTILS.H"

#define CONFIG_FILE "ZMMGR.CFG"
#define DEFAULT_BRIDGE_BAUD 19200L
#define MIN_BRIDGE_BAUD 110L
#define MAX_BRIDGE_BAUD 115200L

static int read_bridge_config_value(const char *key, char *value, int max_len)
{
    FILE *file;
    char line[180];
    int key_len = strlen(key);

    value[0] = '\0';
    file = fopen(CONFIG_FILE, "r");
    if (file == NULL)
        return FALSE;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] == '#')
            continue;
        if (strncmp(line, key, key_len) == 0)
        {
            char *equal = strchr(line, '=');
            char *start;
            char *newline;
            if (equal == NULL)
                continue;
            start = equal + 1;
            while (*start == ' ')
                start++;
            strncpy(value, start, max_len - 1);
            value[max_len - 1] = '\0';
            newline = strchr(value, '\n');
            if (newline != NULL)
                *newline = '\0';
            newline = strchr(value, '\r');
            if (newline != NULL)
                *newline = '\0';
            fclose(file);
            return TRUE;
        }
    }

    fclose(file);
    return FALSE;
}

static long get_bridge_baud(void)
{
    char options[MAX_CONFIG_SIZE] = {"\0"};
    char *baud;
    long value;

    if (read_bridge_config_value("ZMOPTIONS", options, sizeof(options)) == FALSE)
        return DEFAULT_BRIDGE_BAUD;

    baud = strstr(options, "-b");
    if (baud == NULL)
        baud = strstr(options, "-B");
    if (baud == NULL)
        return DEFAULT_BRIDGE_BAUD;

    value = atol(baud + 2);
    if (value <= 0)
        return DEFAULT_BRIDGE_BAUD;
    if (value < MIN_BRIDGE_BAUD)
        return MIN_BRIDGE_BAUD;
    if (value > MAX_BRIDGE_BAUD)
        return MAX_BRIDGE_BAUD;

    return value;
}

static int bridge_serial_port(int port)
{
    if (port == 1) return COM_1;
    if (port == 2) return COM_2;
    if (port == 3) return COM_3;
    if (port == 4) return COM_4;
    return -1;
}

static const char *bridge_serial_error(int error)
{
    switch (error)
    {
        case SER_ERR_NOT_OPEN: return "port not open";
        case SER_ERR_ALREADY_OPEN: return "port already open";
        case SER_ERR_NO_UART: return "no UART";
        case SER_ERR_INVALID_COMPORT: return "invalid COM port";
        case SER_ERR_INVALID_BASE: return "invalid base";
        case SER_ERR_INVALID_IRQ: return "invalid IRQ";
        case SER_ERR_INVALID_BPS: return "invalid baud";
        case SER_ERR_INVALID_DATA_BITS: return "invalid data bits";
        case SER_ERR_INVALID_PARITY: return "invalid parity";
        case SER_ERR_INVALID_STOP_BITS: return "invalid stop bits";
        case SER_ERR_INVALID_HANDSHAKING: return "invalid handshaking";
        case SER_ERR_INVALID_FIFO_THRESHOLD: return "invalid FIFO threshold";
        case SER_ERR_NULL_PTR: return "null pointer";
        case SER_ERR_IRQ_NOT_FOUND: return "IRQ not found";
        case SER_ERR_LOCK_MEM: return "memory lock failed";
    }

    return "serial error";
}

static int bridge_open_serial(int port_number, int serial_port, long baud)
{
    int result;

    result = serial_open(serial_port, baud, 8, 'n', 1, SER_HANDSHAKING_NONE);
    if (result != SER_SUCCESS)
    {
        println("Could not open COM%d: %s (%d)",
                port_number,
                bridge_serial_error(result),
                result);
        return ERROR;
    }

    return SUCCESS;
}

static void bridge_close_serial(int serial_port)
{
    if (serial_port >= COM_1 && serial_port <= COM_4)
        serial_close(serial_port);
}

static void bridge_print_stats(const char *label, SerialBridgeStats *stats)
{
    println("");
    println("%s bridge byte counters:", label);
    println("  A->B bytes=%lu dropped=%lu peakPending=%d",
            stats->a_to_b_bytes,
            stats->a_to_b_dropped,
            stats->a_to_b_peak_pending);
    println("  B->A bytes=%lu dropped=%lu peakPending=%d",
            stats->b_to_a_bytes,
            stats->b_to_a_dropped,
            stats->b_to_a_peak_pending);
}

int SerialManager::passthrough(int port_a, int port_b, int watch_zmodem)
{
    int serial_a = bridge_serial_port(port_a);
    int serial_b = bridge_serial_port(port_b);
    long bridge_baud;
    int bridge_result = SUCCESS;
    SerialBridgeStats stats;
    int bridge_start_result;

    if (serial_a < 0 || serial_b < 0)
    {
        println("Invalid COM port selection");
        return ERROR;
    }

    bridge_baud = get_bridge_baud();

    if (bridge_open_serial(port_a, serial_a, bridge_baud) == ERROR)
        return ERROR;
    if (bridge_open_serial(port_b, serial_b, bridge_baud) == ERROR)
    {
        bridge_close_serial(serial_a);
        return ERROR;
    }

    clrscr();
    println("Pass-through mode: COM%d <-> COM%d (%ld Baud)", port_a, port_b, bridge_baud);
    println("DOS Serial Library ISR bridge");
    println("Bridge handshaking: none");
    println("Press X to exit");
    println("");
    if (watch_zmodem == TRUE)
        println("ZModem watch is disabled in ISR bridge mode");

    bridge_start_result = serial_bridge_start(serial_a, serial_b);
    if (bridge_start_result != SER_SUCCESS)
    {
        println("Could not start bridge: %s (%d)",
                bridge_serial_error(bridge_start_result),
                bridge_start_result);
        bridge_close_serial(serial_b);
        bridge_close_serial(serial_a);
        return ERROR;
    }

    while (1)
    {
        if (kbhit())
        {
            char key = getch();
            if (key == 'x' || key == 'X')
                break;
        }

        delay(1);
    }

    serial_bridge_get_stats(serial_a, serial_b, &stats);
    serial_bridge_stop(serial_a, serial_b);
    bridge_print_stats("Pass-through", &stats);
    bridge_close_serial(serial_b);
    bridge_close_serial(serial_a);
    return bridge_result;
}

int SerialManager::passthrough_until_idle_after_traffic(int port_a, int port_b, int watch_zmodem, int idle_seconds, int max_wait_seconds)
{
    int serial_a = bridge_serial_port(port_a);
    int serial_b = bridge_serial_port(port_b);
    int idle_ticks = 0;
    int idle_limit;
    int wait_ticks = 0;
    int wait_limit;
    long bridge_baud;
    int saw_data = FALSE;
    int bridge_result = SUCCESS;
    SerialBridgeStats stats;
    unsigned long last_total = 0;
    unsigned long current_total;
    unsigned long last_dropped = 0;
    unsigned long current_dropped;
    int bridge_start_result;

    if (serial_a < 0 || serial_b < 0)
    {
        println("Invalid COM port selection");
        return ERROR;
    }

    if (idle_seconds <= 0) idle_seconds = 30;
    if (max_wait_seconds <= 0) max_wait_seconds = 30;

    bridge_baud = get_bridge_baud();
    if (bridge_open_serial(port_a, serial_a, bridge_baud) == ERROR)
        return ERROR;
    if (bridge_open_serial(port_b, serial_b, bridge_baud) == ERROR)
    {
        bridge_close_serial(serial_a);
        return ERROR;
    }

    idle_limit = idle_seconds * 100;
    wait_limit = max_wait_seconds * 100;

    clrscr();
    println("Express bridge mode: COM%d <-> COM%d (%ld Baud)", port_a, port_b, bridge_baud);
    println("DOS Serial Library ISR bridge");
    println("Bridge handshaking: none");
    println("Press X to exit\n");
    if (watch_zmodem == TRUE)
        println("ZModem watch is disabled in ISR bridge mode");

    bridge_start_result = serial_bridge_start(serial_a, serial_b);
    if (bridge_start_result != SER_SUCCESS)
    {
        println("Could not start bridge: %s (%d)",
                bridge_serial_error(bridge_start_result),
                bridge_start_result);
        bridge_close_serial(serial_b);
        bridge_close_serial(serial_a);
        return ERROR;
    }

    while (1)
    {
        int has_pending;

        if (kbhit())
        {
            char key = getch();
            if (key == 'x' || key == 'X')
                break;
        }

        serial_bridge_get_stats(serial_a, serial_b, &stats);
        current_total = stats.a_to_b_bytes + stats.b_to_a_bytes;
        current_dropped = stats.a_to_b_dropped + stats.b_to_a_dropped;
        if (current_dropped != last_dropped)
        {
            println("Bridge dropped bytes: %lu", current_dropped);
            last_dropped = current_dropped;
        }

        has_pending = (serial_get_tx_buffered(serial_a) > 0 ||
                       serial_get_tx_buffered(serial_b) > 0 ||
                       serial_get_rx_buffered(serial_a) > 0 ||
                       serial_get_rx_buffered(serial_b) > 0);

        if (current_total != last_total)
        {
            last_total = current_total;
            saw_data = TRUE;
            idle_ticks = 0;
        }
        else if (has_pending == TRUE)
        {
            delay(1);
        }
        else
        {
            delay(1);
            if (saw_data == TRUE)
            {
                idle_ticks++;
                if (idle_ticks >= (idle_limit * 10))
                    break;
            }
            else
            {
                wait_ticks++;
                if (wait_ticks >= (wait_limit * 10))
                {
                    println("No traffic, timing out");
                    break;
                }
            }
        }
    }

    println("Express bridge mode ended");
    serial_bridge_get_stats(serial_a, serial_b, &stats);
    if (stats.a_to_b_dropped > 0 || stats.b_to_a_dropped > 0)
        println("WARNING: bridge dropped bytes during transfer");
    serial_bridge_stop(serial_a, serial_b);
    bridge_print_stats("Express", &stats);
    bridge_close_serial(serial_b);
    bridge_close_serial(serial_a);
    return bridge_result;
}
