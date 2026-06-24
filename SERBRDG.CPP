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
#define PUMP_CHUNK 256

typedef struct
{
    char data[PUMP_CHUNK];
    int len;
} BridgePending;

typedef struct
{
    unsigned long a_to_b_bytes;
    unsigned long b_to_a_bytes;
    unsigned long a_to_b_stalled;
    unsigned long b_to_a_stalled;
    unsigned long a_to_b_partial;
    unsigned long b_to_a_partial;
    unsigned long a_to_b_idle;
    unsigned long b_to_a_idle;
    unsigned long cts_low_a;
    unsigned long cts_low_b;
    int cts_ever_high_a;
    int cts_ever_high_b;
} BridgeStats;

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
        case SER_ERR_IRQ_IN_USE: return "IRQ already in use";
    }

    return "serial error";
}

static const char *bridge_handshaking_label(int handshaking)
{
    if (handshaking == SER_HANDSHAKING_NONE)
        return "none/direct null modem";
    if (handshaking == SER_HANDSHAKING_RTSCTS)
        return "hardware RTS/CTS";
    return "custom";
}

static int bridge_open_serial(int port_number, int serial_port, long baud, int handshaking)
{
    int result;

    result = serial_open(serial_port, baud, 8, 'n', 1, handshaking);
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

static void bridge_init_stats(BridgeStats *stats)
{
    memset(stats, 0, sizeof(BridgeStats));
}

static int flush_pending(int to,
                         BridgePending *pending,
                         unsigned long *byte_count,
                         unsigned long *stalled_count,
                         unsigned long *partial_count)
{
    int written;

    if (pending->len <= 0)
        return SER_SUCCESS;

    written = serial_write_buffered(to, pending->data, pending->len);
    if (written < 0)
        return written;

    if (written > 0)
    {
        *byte_count += written;
        if (written < pending->len)
        {
            memmove(pending->data, pending->data + written, pending->len - written);
            *partial_count += 1;
        }
        pending->len -= written;
    }
    else
    {
        *stalled_count += 1;
    }

    return SER_SUCCESS;
}

static int pump_direction(int from,
                          int to,
                          BridgePending *pending,
                          unsigned long *byte_count,
                          unsigned long *stalled_count,
                          unsigned long *partial_count,
                          unsigned long *idle_count)
{
    int rc;
    int n;
    int written;

    rc = flush_pending(to, pending, byte_count, stalled_count, partial_count);
    if (rc != SER_SUCCESS || pending->len > 0)
        return rc;

    n = serial_read(from, pending->data, PUMP_CHUNK);
    if (n < 0)
        return n;
    if (n == 0)
    {
        *idle_count += 1;
        return SER_SUCCESS;
    }

    written = serial_write_buffered(to, pending->data, n);
    if (written < 0)
        return written;
    if (written > 0)
        *byte_count += written;

    if (written < n)
    {
        memmove(pending->data, pending->data + written, n - written);
        pending->len = n - written;
        if (written > 0)
            *partial_count += 1;
        else
            *stalled_count += 1;
    }

    return SER_SUCCESS;
}

static int bridge_pump_once(int serial_a,
                            int serial_b,
                            BridgePending *pending_a_to_b,
                            BridgePending *pending_b_to_a,
                            BridgeStats *stats,
                            int handshaking)
{
    int rc;

    if (handshaking == SER_HANDSHAKING_RTSCTS)
    {
        if (!serial_get_cts(serial_a))
            stats->cts_low_a++;
        else
            stats->cts_ever_high_a = TRUE;

        if (!serial_get_cts(serial_b))
            stats->cts_low_b++;
        else
            stats->cts_ever_high_b = TRUE;
    }

    rc = pump_direction(serial_a, serial_b, pending_a_to_b,
                        &stats->a_to_b_bytes,
                        &stats->a_to_b_stalled,
                        &stats->a_to_b_partial,
                        &stats->a_to_b_idle);
    if (rc != SER_SUCCESS)
        return rc;

    rc = pump_direction(serial_b, serial_a, pending_b_to_a,
                        &stats->b_to_a_bytes,
                        &stats->b_to_a_stalled,
                        &stats->b_to_a_partial,
                        &stats->b_to_a_idle);
    return rc;
}

static int bridge_has_pending(int serial_a,
                              int serial_b,
                              BridgePending *pending_a_to_b,
                              BridgePending *pending_b_to_a)
{
    return pending_a_to_b->len > 0 ||
           pending_b_to_a->len > 0 ||
           serial_get_tx_buffered(serial_a) > 0 ||
           serial_get_tx_buffered(serial_b) > 0 ||
           serial_get_rx_buffered(serial_a) > 0 ||
           serial_get_rx_buffered(serial_b) > 0;
}

static void bridge_print_stats(const char *label,
                               int port_a,
                               int port_b,
                               int serial_a,
                               int serial_b,
                               BridgePending *pending_a_to_b,
                               BridgePending *pending_b_to_a,
                               BridgeStats *stats,
                               int handshaking)
{
    int leftover_a;
    int leftover_b;

    leftover_a = serial_get_rx_buffered(serial_a) + serial_get_tx_buffered(serial_a);
    leftover_b = serial_get_rx_buffered(serial_b) + serial_get_tx_buffered(serial_b);

    println("");
    println("%s bridge byte counters:", label);
    println("  COM%d->COM%d bytes=%lu pending=%d partial=%lu stalled=%lu",
            port_a, port_b, stats->a_to_b_bytes, pending_a_to_b->len,
            stats->a_to_b_partial, stats->a_to_b_stalled);
    println("  COM%d->COM%d bytes=%lu pending=%d partial=%lu stalled=%lu",
            port_b, port_a, stats->b_to_a_bytes, pending_b_to_a->len,
            stats->b_to_a_partial, stats->b_to_a_stalled);
    if (handshaking == SER_HANDSHAKING_RTSCTS)
    {
        println("  CTS low iters COM%d=%lu COM%d=%lu",
                port_a, stats->cts_low_a, port_b, stats->cts_low_b);
        println("  CTS ever high COM%d=%s COM%d=%s",
                port_a, stats->cts_ever_high_a ? "yes" : "NEVER",
                port_b, stats->cts_ever_high_b ? "yes" : "NEVER");
    }

    if (leftover_a > 0 || leftover_b > 0 || pending_a_to_b->len > 0 || pending_b_to_a->len > 0)
    {
        println("WARNING: buffered bytes remain: COM%d=%d COM%d=%d pending=%d/%d",
                port_a, leftover_a, port_b, leftover_b,
                pending_a_to_b->len, pending_b_to_a->len);
    }
}

int SerialManager::passthrough(int port_a, int port_b, int watch_zmodem, int handshaking)
{
    int serial_a = bridge_serial_port(port_a);
    int serial_b = bridge_serial_port(port_b);
    long bridge_baud;
    int bridge_result = SUCCESS;
    BridgePending pending_a_to_b;
    BridgePending pending_b_to_a;
    BridgeStats stats;
    int rc;

    if (serial_a < 0 || serial_b < 0 || serial_a == serial_b)
    {
        println("Invalid COM port selection");
        return ERROR;
    }

    bridge_baud = get_bridge_baud();
    if (handshaking != SER_HANDSHAKING_RTSCTS && handshaking != SER_HANDSHAKING_NONE)
        handshaking = SER_HANDSHAKING_RTSCTS;

    if (bridge_open_serial(port_a, serial_a, bridge_baud, handshaking) == ERROR)
        return ERROR;
    if (bridge_open_serial(port_b, serial_b, bridge_baud, handshaking) == ERROR)
    {
        bridge_close_serial(serial_a);
        return ERROR;
    }

    pending_a_to_b.len = 0;
    pending_b_to_a.len = 0;
    bridge_init_stats(&stats);

    clrscr();
    println("Pass-through mode: COM%d <-> COM%d (%ld Baud)", port_a, port_b, bridge_baud);
    println("Foreground serial pump bridge");
    println("Bridge handshaking: %s", bridge_handshaking_label(handshaking));
    println("Press X to exit");
    println("");
    if (watch_zmodem == TRUE)
        println("ZModem watch is disabled in bridge mode");

    while (1)
    {
        if (kbhit())
        {
            char key = getch();
            if (key == 'x' || key == 'X')
                break;
        }

        rc = bridge_pump_once(serial_a, serial_b, &pending_a_to_b, &pending_b_to_a, &stats, handshaking);
        if (rc != SER_SUCCESS)
        {
            println("Bridge pump error: %s (%d)", bridge_serial_error(rc), rc);
            bridge_result = ERROR;
            break;
        }
        delay(1);
    }

    bridge_print_stats("Pass-through", port_a, port_b, serial_a, serial_b,
                       &pending_a_to_b, &pending_b_to_a, &stats, handshaking);
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
    BridgePending pending_a_to_b;
    BridgePending pending_b_to_a;
    BridgeStats stats;
    unsigned long last_total = 0;
    unsigned long current_total;
    int rc;

    if (serial_a < 0 || serial_b < 0 || serial_a == serial_b)
    {
        println("Invalid COM port selection");
        return ERROR;
    }

    if (idle_seconds <= 0) idle_seconds = 30;
    if (max_wait_seconds <= 0) max_wait_seconds = 30;

    bridge_baud = get_bridge_baud();
    if (bridge_open_serial(port_a, serial_a, bridge_baud, SER_HANDSHAKING_RTSCTS) == ERROR)
        return ERROR;
    if (bridge_open_serial(port_b, serial_b, bridge_baud, SER_HANDSHAKING_RTSCTS) == ERROR)
    {
        bridge_close_serial(serial_a);
        return ERROR;
    }

    pending_a_to_b.len = 0;
    pending_b_to_a.len = 0;
    bridge_init_stats(&stats);
    idle_limit = idle_seconds * 1000;
    wait_limit = max_wait_seconds * 1000;

    clrscr();
    println("Express bridge mode: COM%d <-> COM%d (%ld Baud)", port_a, port_b, bridge_baud);
    println("Foreground serial pump bridge");
    println("Bridge handshaking: hardware RTS/CTS");
    println("Press X to exit\n");
    if (watch_zmodem == TRUE)
        println("ZModem watch is disabled in bridge mode");

    while (1)
    {
        int has_pending;

        if (kbhit())
        {
            char key = getch();
            if (key == 'x' || key == 'X')
                break;
        }

        rc = bridge_pump_once(serial_a, serial_b, &pending_a_to_b, &pending_b_to_a, &stats, SER_HANDSHAKING_RTSCTS);
        if (rc != SER_SUCCESS)
        {
            println("Bridge pump error: %s (%d)", bridge_serial_error(rc), rc);
            bridge_result = ERROR;
            break;
        }

        current_total = stats.a_to_b_bytes + stats.b_to_a_bytes;
        has_pending = bridge_has_pending(serial_a, serial_b, &pending_a_to_b, &pending_b_to_a);

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
                if (idle_ticks >= idle_limit)
                    break;
            }
            else
            {
                wait_ticks++;
                if (wait_ticks >= wait_limit)
                {
                    println("No traffic, timing out");
                    break;
                }
            }
        }
    }

    println("Express bridge mode ended");
    bridge_print_stats("Express", port_a, port_b, serial_a, serial_b,
                       &pending_a_to_b, &pending_b_to_a, &stats, SER_HANDSHAKING_RTSCTS);
    bridge_close_serial(serial_b);
    bridge_close_serial(serial_a);
    return bridge_result;
}
