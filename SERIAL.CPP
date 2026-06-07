#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include "APPINFO.H"
#include "SERIAL.H"
#include "UTILS.H"

#define UART_RBR 0
#define UART_THR 0
#define UART_DLL 0
#define UART_IER 1
#define UART_DLM 1
#define UART_IIR 2
#define UART_FCR 2
#define UART_LCR 3
#define UART_MCR 4
#define UART_LSR 5
#define UART_SCR 7

#define LSR_DATA_READY 0x01
#define LSR_THR_EMPTY 0x20

#define LCR_DLAB 0x80
#define LCR_8N1 0x03
#define MCR_DTR_RTS_OUT2 0x0B

#define UART_TYPE_NONE 0
#define UART_TYPE_8250 1
#define UART_TYPE_16450 2
#define UART_TYPE_16550 3
#define UART_TYPE_16550A 4

#define MSD_REPORT_FILE "REPORT.TXT"
#define ROUTE_FILE "ZMROUTES.TXT"
#define DEFAULT_DISCOVERY_BAUD 1200
#define MAX_DISCOVERY_BAUD 1200
#define DISCOVERY_CYCLES 6
#define DISCOVERY_SLOT_COUNT 10
#define DISCOVERY_SLOT_TICKS 1000
#define DISCOVERY_PACKET_SIZE 512
#define MAX_DISCOVERY_MACHINES 16
#define DISCOVERY_BEGIN_MARKER "<<<"
#define DISCOVERY_END_MARKER ">>>\r\n"

static int read_config_value_local(const char *key, char *value, int max_len)
{
    FILE *file;
    char line[180];
    int key_len = strlen(key);

    value[0] = '\0';
    file = fopen("ZMMGR.CFG", "r");
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

static int get_config_ports_local(int *ports, int max_ports)
{
    char portconfig[MAX_CONFIG_SIZE] = {"\0"};
    char *token;
    int count = 0;

    if (read_config_value_local("PORTS", portconfig, sizeof(portconfig)) == FALSE)
    {
        ports[0] = 1;
        return 1;
    }

    token = strtok(portconfig, ",");
    while (token != NULL && count < max_ports)
    {
        ports[count] = atoi(token);
        if (ports[count] >= 1 && ports[count] <= 4)
            count++;
        token = strtok(NULL, ",");
    }

    if (count == 0)
    {
        ports[0] = 1;
        count = 1;
    }

    return count;
}

static int get_discovery_baud_local(void)
{
    char options[MAX_CONFIG_SIZE] = {"\0"};
    char *baud;
    int value;

    if (read_config_value_local("ZMOPTIONS", options, sizeof(options)) == FALSE)
        return DEFAULT_DISCOVERY_BAUD;

    baud = strstr(options, "-b");
    if (baud == NULL)
        baud = strstr(options, "-B");
    if (baud == NULL)
        return DEFAULT_DISCOVERY_BAUD;

    value = atoi(baud + 2);
    if (value <= 0)
        return DEFAULT_DISCOVERY_BAUD;
    if (value > MAX_DISCOVERY_BAUD)
        return MAX_DISCOVERY_BAUD;

    return value;
}

static int get_discovery_slot(const char *computername)
{
    int i;
    int hash = 0;

    for (i = 0; computername[i] != '\0'; i++)
        hash += ((i + 1) * computername[i]);

    if (hash < 0)
        hash = -hash;

    return hash % 10;
}

static void trim_token(char *value)
{
    char *start;
    char *end;

    start = value;
    while (*start == ' ')
        start++;
    if (start != value)
        memmove(value, start, strlen(start) + 1);

    end = value + strlen(value);
    while (end > value && (*(end - 1) == ' ' || *(end - 1) == '\n' || *(end - 1) == '\r'))
    {
        *(end - 1) = '\0';
        end--;
    }
}

static int find_machine(char names[][MAX_FILENAME_LENGTH], int count, const char *name)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (strcmp(names[i], name) == 0)
            return i;
    }

    return -1;
}

static int add_or_update_route(char names[][MAX_FILENAME_LENGTH], int *ports, int *count, const char *name, int port)
{
    int index;
    char cleanname[MAX_FILENAME_LENGTH] = {"\0"};

    strncpy(cleanname, name, MAX_FILENAME_LENGTH - 1);
    cleanname[MAX_FILENAME_LENGTH - 1] = '\0';
    trim_token(cleanname);
    to_upper_case(cleanname);

    if (cleanname[0] == '\0')
        return FALSE;

    index = find_machine(names, *count, cleanname);
    if (index >= 0)
    {
        if (ports[index] == 0)
            return FALSE;
        if (ports[index] != port)
        {
            ports[index] = port;
            return TRUE;
        }
        return FALSE;
    }

    if (*count >= MAX_DISCOVERY_MACHINES)
        return FALSE;

    strcpy(names[*count], cleanname);
    ports[*count] = port;
    (*count)++;
    return TRUE;
}

static void sort_routes(char names[][MAX_FILENAME_LENGTH], int *ports, int count)
{
    int i;
    int j;

    for (i = 0; i < count - 1; i++)
    {
        for (j = i + 1; j < count; j++)
        {
            if (strcmp(names[i], names[j]) > 0)
            {
                char tempname[MAX_FILENAME_LENGTH];
                int tempport;

                strcpy(tempname, names[i]);
                strcpy(names[i], names[j]);
                strcpy(names[j], tempname);

                tempport = ports[i];
                ports[i] = ports[j];
                ports[j] = tempport;
            }
        }
    }
}

static void build_machine_list(char names[][MAX_FILENAME_LENGTH], int *ports, int count, char *output, int output_len)
{
    int i;

    sort_routes(names, ports, count);
    output[0] = '\0';

    for (i = 0; i < count; i++)
    {
        if (i > 0)
            strncat(output, ",", output_len - strlen(output) - 1);
        strncat(output, names[i], output_len - strlen(output) - 1);
    }
}

static void discard_stream_bytes(char *buffer, int *length, int count)
{
    if (count <= 0)
        return;
    if (count >= *length)
    {
        memset(buffer, 0, DISCOVERY_PACKET_SIZE);
        *length = 0;
        return;
    }

    memmove(buffer, buffer + count, *length - count);
    *length -= count;
    buffer[*length] = '\0';
    memset(buffer + *length + 1, 0, DISCOVERY_PACKET_SIZE - *length - 1);
}

static void build_iam_frame(const char *computername, char *packet)
{
    sprintf(packet, "<<<IAM::%s>>>\r\n", computername);
}

static void build_know_frame(const char *computername, const char *knownname, char *packet)
{
    sprintf(packet, "<<<KNOW::%s::%s>>>\r\n", computername, knownname);
}

static int read_discovery_frame(char *buffer, int *length, char *frame, int frame_len)
{
    int i;
    int j;
    int start = -1;
    int begin_len = strlen(DISCOVERY_BEGIN_MARKER);
    int end_len = strlen(DISCOVERY_END_MARKER);
    int end = -1;
    int restart = -1;
    int body_len;

    while (*length > 0)
    {
        start = -1;
        for (i = 0; i <= *length - begin_len; i++)
        {
            if (strncmp(buffer + i, DISCOVERY_BEGIN_MARKER, begin_len) == 0)
            {
                start = i;
                break;
            }
        }

        if (start < 0)
        {
            if (*length > begin_len - 1)
                discard_stream_bytes(buffer, length, *length - (begin_len - 1));
            return FALSE;
        }

        if (start > 0)
            discard_stream_bytes(buffer, length, start);

        restart = -1;
        for (j = begin_len; j <= *length - begin_len; j++)
        {
            if (strncmp(buffer + j, DISCOVERY_BEGIN_MARKER, begin_len) == 0)
            {
                restart = j;
                break;
            }
        }

        end = -1;
        for (j = begin_len; j <= *length - end_len; j++)
        {
            if (strncmp(buffer + j, DISCOVERY_END_MARKER, end_len) == 0)
            {
                end = j;
                break;
            }
        }

        if (restart >= 0 && (end < 0 || restart < end))
        {
            println("DROP partial frame, resync");
            discard_stream_bytes(buffer, length, restart);
            continue;
        }

        if (end < 0)
            return FALSE;

        body_len = end - begin_len;
        if (body_len <= 0 || body_len >= frame_len)
        {
            println("DROP bad frame length %d", body_len);
            discard_stream_bytes(buffer, length, end + end_len);
            continue;
        }

        strncpy(frame, buffer + begin_len, body_len);
        frame[body_len] = '\0';
        discard_stream_bytes(buffer, length, end + end_len);
        return TRUE;
    }

    return FALSE;
}

static void print_discovery_rx_debug(int port, unsigned char value, char *buffer, int length)
{
    char preview[49];
    int preview_len = 0;
    int start;
    int i;
    char printable;

    start = length - 48;
    if (start < 0)
        start = 0;

    for (i = start; i < length && preview_len < (int)sizeof(preview) - 1; i++)
    {
        unsigned char ch = (unsigned char)buffer[i];
        if (ch >= 32 && ch <= 126)
            preview[preview_len] = (char)ch;
        else
            preview[preview_len] = '.';
        preview_len++;
    }
    preview[preview_len] = '\0';

    if (value >= 32 && value <= 126)
        printable = (char)value;
    else
        printable = '.';

    println("RX COM%d len=%03d byte=%02X '%c' buf=[%s]",
            port,
            length,
            (unsigned int)value,
            printable,
            preview);
}

static void print_discovery_buffer_debug(const char *label, int port, char *buffer, int length)
{
    char preview[49];
    int preview_len = 0;
    int start;
    int i;

    start = length - 48;
    if (start < 0)
        start = 0;

    for (i = start; i < length && preview_len < (int)sizeof(preview) - 1; i++)
    {
        unsigned char ch = (unsigned char)buffer[i];
        if (ch >= 32 && ch <= 126)
            preview[preview_len] = (char)ch;
        else
            preview[preview_len] = '.';
        preview_len++;
    }
    preview[preview_len] = '\0';

    println("%s COM%d len=%03d buf=[%s]", label, port, length, preview);
}

static int frame_has_expected_shape(char *frame)
{
    if (strncmp(frame, "IAM::", 5) == 0)
        return TRUE;
    if (strncmp(frame, "KNOW::", 6) == 0)
        return TRUE;
    return FALSE;
}

static int handle_discovery_frame(char *frame,
                                  char names[][MAX_FILENAME_LENGTH],
                                  int *ports,
                                  int *count,
                                  const char *computername,
                                  int port)
{
    char framecopy[DISCOVERY_PACKET_SIZE];
    char *verb;
    char *from;
    char *known;
    int changed = FALSE;

    strncpy(framecopy, frame, sizeof(framecopy) - 1);
    framecopy[sizeof(framecopy) - 1] = '\0';

    if (frame_has_expected_shape(framecopy) == FALSE)
    {
        println("DROP frame [%s]", framecopy);
        return FALSE;
    }

    verb = strtok(framecopy, ":");
    if (verb == NULL)
        return FALSE;

    if (strcmp(verb, "IAM") == 0)
    {
        from = strtok(NULL, ":");
        if (from != NULL)
        {
            trim_token(from);
            to_upper_case(from);
            if (from[0] != '\0' && strcmp(from, computername) != 0)
            {
                changed = add_or_update_route(names, ports, count, from, port);
                if (changed == TRUE)
                    println("Heard %-12s on COM%d", from, port);
            }
        }
    }
    else if (strcmp(verb, "KNOW") == 0)
    {
        from = strtok(NULL, ":");
        known = strtok(NULL, ":");
        if (from != NULL)
        {
            trim_token(from);
            to_upper_case(from);
            if (from[0] != '\0' && strcmp(from, computername) != 0)
            {
                changed = add_or_update_route(names, ports, count, from, port);
                if (changed == TRUE)
                    println("Heard %-12s on COM%d", from, port);
            }
        }
        if (known != NULL)
        {
            trim_token(known);
            to_upper_case(known);
            if (known[0] != '\0' && strcmp(known, computername) != 0)
            {
                if (add_or_update_route(names, ports, count, known, port) == TRUE)
                {
                    changed = TRUE;
                    println("Heard %-12s on COM%d", known, port);
                }
            }
        }
    }

    return changed;
}

static int machine_list_matches(char *received_list, char names[][MAX_FILENAME_LENGTH], int *ports, int count)
{
    char local_list[DISCOVERY_PACKET_SIZE] = {"\0"};
    char received_names[MAX_DISCOVERY_MACHINES][MAX_FILENAME_LENGTH];
    int received_ports[MAX_DISCOVERY_MACHINES];
    int received_count = 0;
    char listcopy[DISCOVERY_PACKET_SIZE];
    char *token;
    int i;

    memset(received_names, 0, sizeof(received_names));
    for (i = 0; i < MAX_DISCOVERY_MACHINES; i++)
        received_ports[i] = 0;

    strncpy(listcopy, received_list, sizeof(listcopy) - 1);
    listcopy[sizeof(listcopy) - 1] = '\0';
    token = strtok(listcopy, ",");
    while (token != NULL)
    {
        add_or_update_route(received_names, received_ports, &received_count, token, 1);
        token = strtok(NULL, ",");
    }

    build_machine_list(names, ports, count, local_list, sizeof(local_list));
    build_machine_list(received_names, received_ports, received_count, received_list, DISCOVERY_PACKET_SIZE);

    return (strcmp(local_list, received_list) == 0);
}

static int write_route_file(char names[][MAX_FILENAME_LENGTH], int *ports, int count)
{
    FILE *file;
    int i;

    file = fopen(ROUTE_FILE, "w");
    if (file == NULL)
        return FALSE;

    fprintf(file, "# MACHINE=COMPORT\n");
    for (i = 0; i < count; i++)
    {
        if (ports[i] == 0)
            fprintf(file, "%s=LOCAL\n", names[i]);
        else
            fprintf(file, "%s=%d\n", names[i], ports[i]);
    }

    fclose(file);
    return TRUE;
}

unsigned int SerialManager::get_com_base(int port_number)
{
    unsigned int far *com_table = (unsigned int far *)MK_FP(0x0040, 0x0000);

    if (port_number < 1 || port_number > 4)
        return 0;

    return com_table[port_number - 1];
}

int SerialManager::uart_present(unsigned int base)
{
    unsigned char original;
    unsigned char testvalue;

    if (base == 0)
        return FALSE;

    original = inportb(base + UART_SCR);
    outportb(base + UART_SCR, 0x55);
    testvalue = inportb(base + UART_SCR);
    outportb(base + UART_SCR, original);

    if (testvalue == 0x55)
        return TRUE;

    return FALSE;
}

int SerialManager::detect_uart_type(unsigned int base)
{
    unsigned char original_scratch;
    unsigned char interrupt_id;

    if (base == 0)
        return UART_TYPE_NONE;

    original_scratch = inportb(base + UART_SCR);
    outportb(base + UART_SCR, 0x55);
    if (inportb(base + UART_SCR) != 0x55)
        return UART_TYPE_8250;

    outportb(base + UART_SCR, 0xAA);
    if (inportb(base + UART_SCR) != 0xAA)
    {
        outportb(base + UART_SCR, original_scratch);
        return UART_TYPE_8250;
    }
    outportb(base + UART_SCR, original_scratch);

    outportb(base + UART_FCR, 0x01);
    interrupt_id = inportb(base + UART_IIR);
    outportb(base + UART_FCR, 0x00);

    if ((interrupt_id & 0xC0) == 0xC0)
        return UART_TYPE_16550A;
    if ((interrupt_id & 0x80) == 0x80)
        return UART_TYPE_16550;

    return UART_TYPE_16450;
}

int SerialManager::get_standard_irq(int port_number)
{
    if (port_number == 1 || port_number == 3)
        return 4;
    if (port_number == 2 || port_number == 4)
        return 3;

    return 0;
}

int SerialManager::get_recommended_baud(int uart_type)
{
    if (uart_type == UART_TYPE_16550A)
        return 115200;
    if (uart_type == UART_TYPE_16550)
        return 57600;
    if (uart_type == UART_TYPE_16450)
        return 19200;
    if (uart_type == UART_TYPE_8250)
        return 9600;

    return 0;
}

const char *SerialManager::get_uart_type_name(int uart_type)
{
    if (uart_type == UART_TYPE_8250)
        return "8250";
    if (uart_type == UART_TYPE_16450)
        return "16450";
    if (uart_type == UART_TYPE_16550)
        return "16550 FIFO bad";
    if (uart_type == UART_TYPE_16550A)
        return "16550A FIFO";

    return "None";
}

const char *SerialManager::get_baud_note(int uart_type)
{
    if (uart_type == UART_TYPE_16550A)
        return "max 115200, test cable/device";
    if (uart_type == UART_TYPE_16550)
        return "FIFO unreliable, test above 57600";
    if (uart_type == UART_TYPE_16450)
        return "no FIFO, 19200 safer";
    if (uart_type == UART_TYPE_8250)
        return "old UART, 9600 safer";

    return "not detected";
}

int SerialManager::data_ready(unsigned int base)
{
    return ((inportb(base + UART_LSR) & LSR_DATA_READY) != 0);
}

int SerialManager::transmit_ready(unsigned int base)
{
    return ((inportb(base + UART_LSR) & LSR_THR_EMPTY) != 0);
}

void SerialManager::initialize_port(unsigned int base, int baud)
{
    unsigned int divisor;

    if (base == 0)
        return;
    if (baud <= 0)
        baud = DEFAULT_DISCOVERY_BAUD;

    divisor = (unsigned int)(115200L / baud);
    if (divisor == 0)
        divisor = 1;

    outportb(base + UART_IER, 0x00);
    outportb(base + UART_LCR, LCR_DLAB);
    outportb(base + UART_DLL, (unsigned char)(divisor & 0xFF));
    outportb(base + UART_DLM, (unsigned char)((divisor >> 8) & 0xFF));
    outportb(base + UART_LCR, LCR_8N1);
    outportb(base + UART_FCR, 0x07);
    outportb(base + UART_MCR, MCR_DTR_RTS_OUT2);
}

void SerialManager::flush_receive(unsigned int base)
{
    int guard = 0;

    while (data_ready(base) == TRUE && guard < DISCOVERY_PACKET_SIZE)
    {
        read_byte(base);
        guard++;
    }
}

unsigned char SerialManager::read_byte(unsigned int base)
{
    return inportb(base + UART_RBR);
}

void SerialManager::write_byte(unsigned int base, unsigned char value)
{
    outportb(base + UART_THR, value);
}

void SerialManager::update_zmodem_watch(char *buffer, int *length, unsigned char value, const char *direction)
{
    int i;

    if (*length >= 31)
    {
        for (i = 0; i < 30; i++)
            buffer[i] = buffer[i + 1];
        *length = 30;
    }

    buffer[*length] = (char)value;
    (*length)++;
    buffer[*length] = '\0';

    if (strstr(buffer, "rz") != NULL ||
        strstr(buffer, "sz") != NULL ||
        strstr(buffer, "**B") != NULL ||
        strstr(buffer, "ZRQINIT") != NULL ||
        strstr(buffer, "ZFILE") != NULL)
    {
        println("Possible ZModem traffic detected on %s", direction);
        *length = 0;
        buffer[0] = '\0';
    }
}

int SerialManager::mouse_driver_present(void)
{
    union REGS regs;

    regs.x.ax = 0;
    int86(0x33, &regs, &regs);

    if (regs.x.ax == 0xFFFF)
        return TRUE;

    return FALSE;
}

int SerialManager::get_mouse_type_irq(int *mouse_type, int *irq, int *major_version, int *minor_version)
{
    union REGS regs;

    *mouse_type = 0;
    *irq = 0;
    *major_version = 0;
    *minor_version = 0;

    regs.x.ax = 0x0024;
    int86(0x33, &regs, &regs);

    *major_version = regs.h.bh;
    *minor_version = regs.h.bl;
    *mouse_type = regs.h.ch;
    *irq = regs.h.cl;

    if (*mouse_type == 0 && *irq == 0 && *major_version == 0 && *minor_version == 0)
        return FALSE;

    return TRUE;
}

const char *SerialManager::get_mouse_type_name(int mouse_type)
{
    if (mouse_type == 1)
        return "Bus";
    if (mouse_type == 2)
        return "Serial";
    if (mouse_type == 3)
        return "InPort";
    if (mouse_type == 4)
        return "PS/2";
    if (mouse_type == 5)
        return "SmartMouse";

    return "Unknown";
}

int SerialManager::infer_mouse_com_from_irq(int irq)
{
    if (irq == 4)
    {
        int com1 = get_com_base(1) != 0;
        int com3 = get_com_base(3) != 0;

        if (com1 == TRUE && com3 == FALSE)
            return 1;
        if (com1 == FALSE && com3 == TRUE)
            return 3;
    }

    if (irq == 3)
    {
        int com2 = get_com_base(2) != 0;
        int com4 = get_com_base(4) != 0;

        if (com2 == TRUE && com4 == FALSE)
            return 2;
        if (com2 == FALSE && com4 == TRUE)
            return 4;
    }

    return 0;
}

void SerialManager::print_irq_vectors(void)
{
    unsigned int far *ivt = (unsigned int far *)MK_FP(0x0000, 0x0000);
    unsigned int irq3_offset;
    unsigned int irq3_segment;
    unsigned int irq4_offset;
    unsigned int irq4_segment;
    unsigned long irq3_addr;
    unsigned long irq4_addr;

    irq3_offset = ivt[0x0B * 2];
    irq3_segment = ivt[(0x0B * 2) + 1];
    irq4_offset = ivt[0x0C * 2];
    irq4_segment = ivt[(0x0C * 2) + 1];

    irq3_addr = (unsigned long)irq3_segment * 16L + irq3_offset;
    irq4_addr = (unsigned long)irq4_segment * 16L + irq4_offset;

    println("IRQ3 / INT 0Bh vector: %04X:%04X linear %08lX",
            irq3_segment,
            irq3_offset,
            irq3_addr);
    println("IRQ4 / INT 0Ch vector: %04X:%04X linear %08lX",
            irq4_segment,
            irq4_offset,
            irq4_addr);
}

int SerialManager::get_dos_major_version(void)
{
    union REGS regs;

    regs.h.ah = 0x30;
    intdos(&regs, &regs);

    return regs.h.al;
}

int SerialManager::run_msd_report(const char *report_file)
{
    char command[MAX_FILE_SIZE] = {"\0"};

    sprintf(command, "MSD.EXE /P %s", report_file);
    return system(command);
}

int SerialManager::parse_msd_report(const char *report_file)
{
    FILE *file;
    char line[160];
    char *valueptr;
    int found_com = 0;
    int found_irq = 0;
    int in_mouse_section = FALSE;

    file = fopen(report_file, "r");
    if (file == NULL)
    {
        println("Could not open %s", report_file);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        to_upper_case(line);

        if (strstr(line, "-----") != NULL || strstr(line, "------") != NULL)
        {
            if (strstr(line, "MOUSE") != NULL)
                in_mouse_section = TRUE;
            else if (in_mouse_section == TRUE)
                in_mouse_section = FALSE;
        }

        if (in_mouse_section == TRUE)
        {
            if (strstr(line, "MOUSE COM PORT ADDRESS") != NULL)
            {
                valueptr = strchr(line, ':');
                if (valueptr != NULL)
                {
                    if (strstr(valueptr, "03F8") != NULL)
                        found_com = 1;
                    else if (strstr(valueptr, "02F8") != NULL)
                        found_com = 2;
                    else if (strstr(valueptr, "03E8") != NULL)
                        found_com = 3;
                    else if (strstr(valueptr, "02E8") != NULL)
                        found_com = 4;
                }
            }
            else if (strstr(line, "MOUSE COM PORT") != NULL)
            {
                valueptr = strchr(line, ':');
                if (valueptr != NULL)
                {
                    if (strstr(valueptr, "COM1") != NULL)
                        found_com = 1;
                    else if (strstr(valueptr, "COM2") != NULL)
                        found_com = 2;
                    else if (strstr(valueptr, "COM3") != NULL)
                        found_com = 3;
                    else if (strstr(valueptr, "COM4") != NULL)
                        found_com = 4;
                }
            }
            else if (strstr(line, "MOUSE IRQ") != NULL)
            {
                valueptr = strchr(line, ':');
                if (valueptr != NULL)
                {
                    if (strstr(valueptr, "4") != NULL)
                        found_irq = 4;
                    else if (strstr(valueptr, "3") != NULL)
                        found_irq = 3;
                }
            }
        }
    }

    fclose(file);

    if (found_com > 0)
        return found_com;

    if (found_irq > 0)
        return infer_mouse_com_from_irq(found_irq);

    return 0;
}

void SerialManager::show_msd_fallback(void)
{
    int dos_major = get_dos_major_version();
    int msd_com = 0;
    char key;

    if (dos_major < 6)
    {
        println("MSD fallback: DOS %d detected; requires DOS 6.0 or newer", dos_major);
        return;
    }

    println("");
    println("MSD fallback is available for DOS %d.x", dos_major);
    println("Run MSD.EXE /P %s now? Y/N", MSD_REPORT_FILE);
    key = getch();

    if (key != 'y' && key != 'Y')
    {
        println("MSD fallback skipped");
        return;
    }

    if (run_msd_report(MSD_REPORT_FILE) != 0)
    {
        println("MSD.EXE did not complete successfully");
        return;
    }

    msd_com = parse_msd_report(MSD_REPORT_FILE);
    if (msd_com > 0)
        println("MSD report suggests mouse on COM%d", msd_com);
    else
        println("MSD report did not identify a single mouse COM port");
}

void SerialManager::show_diagnostics(void)
{
    int i;
    int mouse_type = 0;
    int mouse_irq = 0;
    int mouse_major = 0;
    int mouse_minor = 0;
    int inferred_com = 0;

    clrscr();
    println("ZMGR Serial Port Diagnostics");
    println("");

    for (i = 1; i <= 4; i++)
    {
        unsigned int base = get_com_base(i);
        int standard_irq = get_standard_irq(i);
        int uart_type = detect_uart_type(base);
        int recommended_baud = get_recommended_baud(uart_type);

        if (base == 0)
        {
            println("COM%d IRQ%d base ---- UART %-12s max baud ----",
                    i,
                    standard_irq,
                    "not listed");
        }
        else
        {
            println("COM%d IRQ%d base %04Xh UART %-12s max baud 115200",
                    i,
                    standard_irq,
                    base,
                    get_uart_type_name(uart_type));
            println("     recommended %-6d %s",
                    recommended_baud,
                    get_baud_note(uart_type));
        }
    }

    if (mouse_driver_present() == TRUE)
    {
        println("Mouse driver: present");
        if (get_mouse_type_irq(&mouse_type, &mouse_irq, &mouse_major, &mouse_minor) == TRUE)
        {
            println("Mouse driver version: %d.%02d", mouse_major, mouse_minor);
            println("Mouse type: %s (%d)", get_mouse_type_name(mouse_type), mouse_type);
            println("Mouse IRQ: %d", mouse_irq);

            inferred_com = infer_mouse_com_from_irq(mouse_irq);
            if (inferred_com > 0)
            {
                println("Likely mouse port: COM%d", inferred_com);
            }
            else if (mouse_irq == 4)
            {
                println("Likely mouse port: COM1 or COM3");
                println("IRQ4 is shared by COM1 and COM3");
                show_msd_fallback();
            }
            else if (mouse_irq == 3)
            {
                println("Likely mouse port: COM2 or COM4");
                println("IRQ3 is shared by COM2 and COM4");
                show_msd_fallback();
            }
            else
            {
                println("Mouse serial port: not inferred from IRQ");
                show_msd_fallback();
            }
        }
        else
        {
            println("Mouse type/IRQ query was not supported");
            show_msd_fallback();
        }
    }
    else
    {
        println("Mouse driver: not detected");
    }

    println("");
    println("Press any key to continue");
    getch();
}

void SerialManager::run_who_is_there(void)
{
    char computername[MAX_FILENAME_LENGTH] = {"\0"};
    char route_names[MAX_DISCOVERY_MACHINES][MAX_FILENAME_LENGTH];
    int route_ports[MAX_DISCOVERY_MACHINES];
    int route_count = 0;
    int ports[MAX_ARGS];
    int port_count;
    char receive_buffers[MAX_ARGS][DISCOVERY_PACKET_SIZE];
    int receive_lengths[MAX_ARGS];
    int cycle;
    int p;
    int converged = FALSE;
    int discovery_baud = get_discovery_baud_local();
    int discovery_slot;

    memset(route_names, 0, sizeof(route_names));
    memset(receive_buffers, 0, sizeof(receive_buffers));
    for (p = 0; p < MAX_DISCOVERY_MACHINES; p++)
        route_ports[p] = 0;
    for (p = 0; p < MAX_ARGS; p++)
        receive_lengths[p] = 0;

    if (read_config_value_local("COMPUTERNAME", computername, sizeof(computername)) == FALSE ||
        computername[0] == '\0')
        strcpy(computername, "UNKNOWN");
    to_upper_case(computername);
    add_or_update_route(route_names, route_ports, &route_count, computername, 0);
    discovery_slot = get_discovery_slot(computername);

    port_count = get_config_ports_local(ports, MAX_ARGS);

    clrscr();
    println("ZMGR Who Is There");
    println("%s", ZMGR_BUILD_LABEL);
    println("");
    println("Local machine: %s", computername);
    println("Discovery baud: %d 8N1", discovery_baud);
    println("Discovery slot: %d", discovery_slot);
    println("Using configured ports:");
    for (p = 0; p < port_count; p++)
    {
        unsigned int base = get_com_base(ports[p]);

        println("  COM%d", ports[p]);
        if (base != 0 && uart_present(base) == TRUE)
        {
            initialize_port(base, discovery_baud);
            flush_receive(base);
        }
    }
    println("");
    println("Sending discovery packets. Press X to stop.");
    println("");

    for (cycle = 0; cycle < DISCOVERY_CYCLES && converged == FALSE; cycle++)
    {
        for (p = 0; p < DISCOVERY_SLOT_COUNT; p++)
        {
            int port_index;
            int listen_tick;
            if (kbhit())
            {
                char key = getch();
                if (key == 'x' || key == 'X')
                    cycle = DISCOVERY_CYCLES;
            }

            if (p == discovery_slot)
            {
                for (port_index = 0; port_index < port_count; port_index++)
                {
                    unsigned int base = get_com_base(ports[port_index]);
                    char packet[DISCOVERY_PACKET_SIZE];
                    int route_index;
                    int know_index;

                    if (base == 0 || uart_present(base) == FALSE)
                        continue;

                    memset(packet, 0, sizeof(packet));
                    build_iam_frame(computername, packet);
                    {
                        char *outptr = packet;
                        while (*outptr != '\0')
                        {
                            int waitcount = 0;
                            while (transmit_ready(base) == FALSE && waitcount < 1000)
                            {
                                delay(1);
                                waitcount++;
                            }
                            if (transmit_ready(base) == TRUE)
                                write_byte(base, (unsigned char)*outptr);
                            outptr++;
                        }
                    }

                    if (route_count > 0)
                    {
                        know_index = cycle % route_count;
                        for (route_index = 0; route_index < route_count; route_index++)
                        {
                            int index = (know_index + route_index) % route_count;
                            if (route_names[index][0] == '\0')
                                continue;
                            memset(packet, 0, sizeof(packet));
                            build_know_frame(computername, route_names[index], packet);
                            {
                                char *outptr = packet;
                                while (*outptr != '\0')
                                {
                                    int waitcount = 0;
                                    while (transmit_ready(base) == FALSE && waitcount < 1000)
                                    {
                                        delay(1);
                                        waitcount++;
                                    }
                                    if (transmit_ready(base) == TRUE)
                                        write_byte(base, (unsigned char)*outptr);
                                    outptr++;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            for (listen_tick = 0; listen_tick < DISCOVERY_SLOT_TICKS; listen_tick++)
            {
                for (port_index = 0; port_index < port_count; port_index++)
                {
                    unsigned int base = get_com_base(ports[port_index]);

                    if (base == 0)
                        continue;

                    while (data_ready(base) == TRUE)
                    {
                        unsigned char value = read_byte(base);
                        int length = receive_lengths[port_index];

                        if (length < DISCOVERY_PACKET_SIZE - 1)
                        {
                            receive_buffers[port_index][length] = (char)value;
                            length++;
                            receive_buffers[port_index][length] = '\0';
                            receive_lengths[port_index] = length;
                            print_discovery_rx_debug(ports[port_index], value, receive_buffers[port_index], receive_lengths[port_index]);
                        }
                        else
                        {
                            receive_lengths[port_index] = 0;
                            receive_buffers[port_index][0] = '\0';
                        }

                        {
                            char frame[DISCOVERY_PACKET_SIZE];

                            memset(frame, 0, sizeof(frame));
                            while (read_discovery_frame(receive_buffers[port_index],
                                                        &receive_lengths[port_index],
                                                        frame,
                                                        sizeof(frame)) == TRUE)
                            {
                                println("FRAME COM%d [%s]", ports[port_index], frame);
                                handle_discovery_frame(frame,
                                                       route_names,
                                                       route_ports,
                                                       &route_count,
                                                       computername,
                                                       ports[port_index]);
                            }
                            print_discovery_buffer_debug("POST", ports[port_index], receive_buffers[port_index], receive_lengths[port_index]);
                        }
                    }
                }
                delay(1);
            }
        }
    }

    sort_routes(route_names, route_ports, route_count);
    println("");
    println("Known machines:");
    for (p = 0; p < route_count; p++)
    {
        if (route_ports[p] == 0)
            println("  %-12s LOCAL", route_names[p]);
        else
            println("  %-12s COM%d", route_names[p], route_ports[p]);
    }

    if (write_route_file(route_names, route_ports, route_count) == TRUE)
        println("Saved %s", ROUTE_FILE);
    else
        println("Could not save %s", ROUTE_FILE);

    if (converged == TRUE)
        println("Discovery converged");
    else
        println("Discovery ended");

    println("");
    println("Press any key to continue");
    getch();
}

int SerialManager::passthrough(int port_a, int port_b, int watch_zmodem)
{
    unsigned int base_a = get_com_base(port_a);
    unsigned int base_b = get_com_base(port_b);
    char watch_a[32] = {"\0"};
    char watch_b[32] = {"\0"};
    int watch_a_len = 0;
    int watch_b_len = 0;

    if (base_a == 0 || base_b == 0)
    {
        println("Invalid COM port selection");
        return ERROR;
    }

    if (uart_present(base_a) == FALSE || uart_present(base_b) == FALSE)
    {
        println("One or both UARTs did not respond");
        println("COM%d base 0x%04X, COM%d base 0x%04X", port_a, base_a, port_b, base_b);
        return ERROR;
    }

    clrscr();
    println("Pass-through mode: COM%d <-> COM%d", port_a, port_b);
    println("Press X to exit");
    println("");

    while (1)
    {
        if (kbhit())
        {
            char key = getch();
            if (key == 'x' || key == 'X')
                break;
        }

        if (data_ready(base_a) && transmit_ready(base_b))
        {
            unsigned char value = read_byte(base_a);
            write_byte(base_b, value);
            if (watch_zmodem == TRUE)
                update_zmodem_watch(watch_a, &watch_a_len, value, "COM A -> COM B");
        }

        if (data_ready(base_b) && transmit_ready(base_a))
        {
            unsigned char value = read_byte(base_b);
            write_byte(base_a, value);
            if (watch_zmodem == TRUE)
                update_zmodem_watch(watch_b, &watch_b_len, value, "COM B -> COM A");
        }
    }

    return SUCCESS;
}
