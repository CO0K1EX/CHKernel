#include "stdint.h"

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEY_ENTER 0x1C

char *video_memory = (char*)VIDEO_MEMORY;

#define INPUT_BUFFER_SIZE 256
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;

unsigned char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*',  0,   ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};


static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ __volatile__("inb %1, %0" : "=a" (value) : "d" (port));
    return value;
}

char read_character()
{
    char scancode = 0;

    // Чтение сканкода с порта клавиатуры (0x60)
    __asm__ __volatile__ ("inb %1, %0" : "=a"(scancode) : "Nd"(0x60));

    //draw_text(input_buffer, 0, 24, 0x07); // Последняя строка экрана
    
    // Проверяем, не является ли это сканкодом отпускания клавиши (high bit == 1)
    if (scancode & 0x80)
    {
        // Игнорируем сканкоды отпускания
        return 0;
    }
    else
    {
        // Возвращаем соответствующий ASCII-символ
        return scancode_to_ascii[scancode];
    }
}

int strcmp(const char* str1,const char* str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }

    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int is_enter_pressed()
{
    unsigned char status;

    status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01)
    {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);
        if (scancode == KEY_ENTER) // Код клавиши Enter
        {
            return 1;
        }
    }

    return 0;
}

void clear_input_buffer()
{
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++)
    {
        input_buffer[i] = '\0';
    }
    input_index = 0;
}

// Функция для очистки экрана
void clear_screen()
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = 0x07;
    }
}

// Функция для отображения текста в видеопамяти
void draw_text(const char *text, int x, int y, uint8_t color)
{
    int i = 0;
    int current_x = x;
    int current_y = y;

    while (text[i] != '\0')
    {
        if (text[i] == '\n')
        {
            current_x = 0;
            current_y++;

            if (current_y >= SCREEN_HEIGHT)
            {
                current_y = 0;
            }
        }
        else
        {
            video_memory[((current_y * SCREEN_WIDTH + current_x) * 2)] = text[i];
            video_memory[((current_y * SCREEN_WIDTH + current_x) * 2) + 1] = color;
            current_x++;

            if (current_x >= SCREEN_WIDTH)
            {
                current_x = 0;
                current_y++;

                if (current_y >= SCREEN_HEIGHT)
                {
                    current_y = 0;
                }
            }
        }
        i++;
    }
}

const char *greeting_text = "Welcome to Marsh OS\nPress ENTER to continue...";

// Функция отображения приветственного экрана
void display_welcome_screen(void)
{
    draw_text(greeting_text, 10, 5, 0x07);

    while (1)
    {
        if (is_enter_pressed())
        {
            break; // Выход из цикла при нажатии клавиши Enter
        }
    }
}

// Ожидание нажатия клавиши Enter
void wait_for_enter()
{
    while (1)
    {
        if (is_enter_pressed())  // Ожидаем нажатие клавиши Enter
        {
            break;
        }
    }
}

void delay(int count)
{
    for (int i = 0; i < count * 1000000; i++) { // Увеличиваем задержку для замедления
        asm volatile("nop");
    }
}

void handle_command(const char* command)
{
    // Отладочный вывод команды
    draw_text("Command: ", 0, 23, 0x07);
    draw_text(command, 9, 23, 0x07); // Печать введённой команды на экран

    if (strcmp(command, "clear") == 0)
    {
        clear_screen();
    }
    else if (strcmp(command, "help") == 0)
    {
        draw_text("Available commands:", 0, 0, 0x07);
        draw_text("clear - Clear the screen\n", 0, 1, 0x07);
        draw_text("reboot - Reboot the system\n", 0, 2, 0x07);
        draw_text("about - Show information about the OS\n", 0, 3, 0x07);
    }
    else if (strcmp(command, "about") == 0)
    {
        draw_text("Marsh OS v0.0.1 by CO0K1E\n", 0, 0, 0x07);
    }
    else if (strcmp(command, "reboot") == 0)
    {
        // reboot_system();
    }
    else
    {
        draw_text("Unknown command\n", 0, 0, 0x07);
    }
}

void kernelMain(void)
{
    clear_screen();  // Очистка экрана
    draw_text("Welcome to Marsh OS\nPress ENTER to continue...", 10, 5, 0x07); // Показываем приветственный экран
    wait_for_enter();  // Ждем нажатие клавиши Enter
    clear_screen();
    draw_text("> ",0,1,0x07);

    int cursor_x = 2;
    int cursor_y = 0;

    while (1)
{
    char c = read_character();

    if (c)
    {
        // Проверяем клавишу Enter
        if (c == '\n')
        {
            input_buffer[input_index] = '\0'; // Завершаем строку
            handle_command(input_buffer);    // Обрабатываем команду
            clear_input_buffer();            // Очищаем буфер
            input_index = 0;

            // Переходим на следующую строку
            cursor_x = 0;
            cursor_y++;
            draw_text("> ", cursor_x, cursor_y, 0x07);
            cursor_x = 2;
        }
        else if (c == '\b') // Обработка Backspace
        {
            if (input_index > 0)
            {
                input_index--;
                cursor_x--;

                if (cursor_x < 2)
                {
                    cursor_x = SCREEN_WIDTH - 1;
                    cursor_y--;
                }

                draw_text(" ", cursor_x, cursor_y, 0x07);
            }
        }
        else if (input_index < INPUT_BUFFER_SIZE - 1)
        {
            input_buffer[input_index++] = c;
            draw_text(&c, cursor_x++, cursor_y, 0x07);

            if (cursor_x >= SCREEN_WIDTH)
            {
                cursor_x = 0;
                cursor_y++;
            }
        }
    }
}
}
