static void vga_print(const char *s, unsigned char color) {
    volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
    unsigned int i = 0;

    while (s[i] != '\0') {
        vga[i] = (unsigned short)color << 8 | (unsigned short)s[i];
        i++;
    }
}

void kmain(void) {
    vga_print("Hello from kernel.c", 0x0F);

    for (;;) {
    }
}