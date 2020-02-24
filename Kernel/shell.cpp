#include "shell.h"
#include "shitio.h"
#include "paging.h"
#include "port.h"
#include "memory.h"

/* prototypes */

void version();

void clr();

void startpage();

void help();

void reboot();

void shutdown();

void print(const char *str);

void test();

void add_command(const char base_name[10], int args, int size);

int command_parse(char argument[5][10], int start, const char *input);

/* globals */

using namespace standardout;
using namespace shell;

command arg[2];

const char *command_list[] = { "version", "clr", "pagebegin", "shutdown", "reboot", "test", "help" };
const char *arg_command[] = { "print" };

typedef void (*command_functions)();
command_functions comm_func[] = { version, clr, startpage, shutdown, reboot, test, help };

void command_handler(const char *input) {
	static bool set_up = false;

	if(!set_up) {
		add_command("print", 1, 20);
		set_up = true;
	}

	char arguments[5][10];

	bool commandFound = false;

	if(end_of_terminal())
		clr();

	for(long unsigned int i = 0; i < sizeof command_list/sizeof *command_list; i++) {
		if(strcmp(input, command_list[i]) == 0) {
			comm_func[i]();
			commandFound = true;
		}
		if(commandFound)
			break;
	}

	if(!commandFound) {

		char base[10];

		int length = strlen(input);
		int break_point = 0;

		for(int i = 0; i < length; i++) {
			if(input[i] == ' ') {
				break_point = i;
				break;
			}
			else
				base[i] = input[i];
		}

		for(int i = 0; i < 2; i++) { //toDo: make this nicer

			if(strcmp(arg[i].name, base) == 0) {

				commandFound = true;

				int argsize = command_parse(arguments, break_point, input);

				char sendstr[256];
				for(int i = 0; i < arg[i].arguments_num; i++) // stripper
					for(int j = 0; j < argsize; j++)
						sendstr[j] = arguments[i][j];


				print(sendstr);

				memset(sendstr, 0, 256);

				break;
			}
		}
	}

	if(!commandFound && strlen(input) != 0)
		k_print("\n%s commnad not found\n", input);

	k_print("> ");
}


void print(const char *str) {
	int length = strlen(str);
	putchar('\n');
	for(int i = 0; i < length; i++)
		putchar(str[i]);
}

void version() {
	k_print("\ncrepOS beta 1.1\n");
}

void clr() {
	clear_screen();
}

void startpage() {
	paging_init();
}

void help() {
	putchar('\n');
	for(long unsigned int i = 0; i < sizeof command_list / sizeof *command_list - 1; i++)
		k_print("%s ", command_list[i]);
	putchar('\n');
	for(long unsigned int i = 0; i < sizeof arg_command / sizeof *arg_command; i++)
		k_print("%s ", arg_command[i]);
	putchar('\n');
}

void reboot() {
	uint8_t check = 0x02;
	while (check & 0x02)
		check = inb(0x64);
    	outb(0x64, 0xFE);
	asm volatile("hlt");
}

/* not APIC, just qemu bochs and virtual machine */

void shutdown(void) {
	asm volatile ("cli");
	while(1) {
		outw (0xB004, 0x2000);

      		for (const char *s = "Shutdown"; *s; ++s)
        		outb (0x8900, *s);

      		asm volatile ("cli; hlt");
    	}
}

void add_command(const char base_name[10], int args, int size) {
	static int counter = 0;

	if(counter == 2) {
		t_print("fatal: increase size of struct array to add another command");
		return;
	}

	strcpy(arg[counter].name, base_name);
	arg[counter].arguments_num = args;
	arg[counter].assumed_size = size;

	counter++;
}

int command_parse(char argument[5][10], int start, const char *input) {

	int arg_count = 0;
	int counter = 0;

	int argsize = 0;

	int strle = strlen(input);
	bool got_base;

	for(int j = start+1; j < strle; j++) {
		if(input[j-1] == ' ')
			got_base = true;

		if(got_base) {
			if(input[j] == ' ') {
				arg_count++;
				counter = 0;
			}
			else if(counter < strle) {
				argument[arg_count][counter++] = input[j];
				argsize++;
			}
			else {
				t_print("Woah there you, you almost casued a seg fault");
				break;
			}
		}
	}
	return argsize;
}

void test() {
	k_print("\nOffical Bruh Momment\n");
}








