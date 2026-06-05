#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include <STRING.H>
#include "SERIAL.H"
#include "UTILS.H"

#define UART_RBR 0
#define UART_THR 0
#define UART_IER 1
#define UART_LSR 5
#define UART_SCR 7

#define LSR_DATA_READY 0x01
#define LSR_THR_EMPTY 0x20

#define MSD_REPORT_FILE "REPORT.TXT"

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

int SerialManager::data_ready(unsigned int base)
{
    return ((inportb(base + UART_LSR) & LSR_DATA_READY) != 0);
}

int SerialManager::transmit_ready(unsigned int base)
{
    return ((inportb(base + UART_LSR) & LSR_THR_EMPTY) != 0);
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

        if (base == 0)
        {
            println("COM%d: Not listed by BIOS", i);
        }
        else
        {
            println("COM%d: BIOS base 0x%04X, UART %s",
                    i,
                    base,
                    uart_present(base) == TRUE ? "responds" : "not confirmed");
        }
    }

    println("");
    print_irq_vectors();
    println("");

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
