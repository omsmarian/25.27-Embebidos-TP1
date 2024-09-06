#include "fsl.h"


static volatile char key_pressed = 0;
static volatile int key_flag = 0;


static uint8_t state = ADD_USER;






void init_fsl()
{
	state = ADD_USER;
    print_menu(state);

    if(encoder_Init())
    {
		PRINTF("Encoder initialized\n");
	}
    else
	{
		PRINTF("Error initializing encoder\n");
	}

}

void update_fsl()
{
	update_menu();
}


void update_menu()
{

    int key = read_key();
    if (key == 1) { //LEFT
        if (state == ADD_USER) {
            state = ACCESS_SYSTEM;
        } else {
            state--;
        }
    } else if (key == 3) { //RIGHT
        if (state == ACCESS_SYSTEM) {
            state = ADD_USER;
        } else {
            state++;
        }
    }

    else if (key == 4)
    { //ENTER
            switch (state) {
            case ADD_USER:
                add_user_call();
                break;
            case DELETE_USER:
                delete_user_call();
                break;
            case CHANGE_PASSWORD:
                change_password_call();
                break;
            case ACCESS_SYSTEM:
            	access_system_call();
                break;
            }
    }
	if (key == 1 || key == 3) {
		print_menu(state);
	}
}

void print_menu(enum states_fsl state) {
    switch (state) {
    case ADD_USER:
    	clear_terminal();
        PRINTF(">> Add User\n   Delete User\n   Change Password\n   Access System\n");
        break;
    case DELETE_USER:
    	clear_terminal();
        PRINTF("   Add User\n>> Delete User\n   Change Password\n   Access System\n");
        break;
    case CHANGE_PASSWORD:
    	clear_terminal();
        PRINTF("   Add User\n   Delete User\n>> Change Password\n   Access System\n");
        break;
    case ACCESS_SYSTEM:
    	clear_terminal();
        PRINTF("   Add User\n   Delete User\n   Change Password\n>> Access System\n");
        break;
    }
}

void read_console(void) {
    char c;
    if (scanf(" %c", &c) == 1)
    {
        key_pressed = c;
        key_flag = 1;
    }
}

int read_key(void) {
	uint8_t key = encoderRead();

	switch (key)
	{
	case NONE : 		return 0;
	case LEFT : 		return 1;
	case LONG_CLICK : 	return 2;
	case RIGHT : 		return 3;
	case CLICK : 		return 4;
	}
}

void clear_terminal() {
    system("clear");
}


bool read_from_encoder(char *id) {
    static int digit_index = 0;
    static int current_digit = 0;

    int key = read_key();
    if (key == 0) { // No key pressed
        return false;
    }

    if (key == 1) { //LEFT
        current_digit = (current_digit == 0) ? 9 : current_digit - 1;
        id[digit_index] = '0' + current_digit;
        id[digit_index + 1] = '\0'; // Null-terminate the ID string

        PRINTF("%s\n", id);
    } else if (key == 3) { //RIGHT
        current_digit = (current_digit == 9) ? 0 : current_digit + 1;
        id[digit_index] = '0' + current_digit;
        id[digit_index + 1] = '\0'; // Null-terminate the ID string

        PRINTF("%s\n", id);

    } else if (key == 4) { //ENTER
        id[digit_index] = '0' + current_digit;
        id[digit_index + 1] = '\0'; // Null-terminate the ID string
        digit_index++;
        current_digit = 0;

        PRINTF("%s\n", id);

        if (digit_index == 8) {
            id[digit_index] = '\0'; // Null-terminate the ID string
            digit_index = 0; // Reset for next ID read
            PRINTF("TEST\n");
            return true;
        }
    }
    return false;
}

bool read_from_card(char *id) {
    // Check if card is swiped, if not return false

    // Read card data
    // ...

    return false;
}

void read_id(char *id) {
    int read_successful = 0;

    while (!read_successful) {
        read_successful = read_from_encoder(id);
        if (!read_successful) {
            read_successful = read_from_card(id);
        }
    }
    PRINTF("User id: %s\n", id);
}

//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

void read_password(char *password) {
    int digit_index = 0;
    int current_digit = 0;

    while (digit_index < 4) {
        int key = read_key();
        if (key == 1) { //LEFT
            current_digit = (current_digit == 0) ? 9 : current_digit - 1;
            password[digit_index] = '0' + current_digit;
            PRINTF("%s\n", password);
        } else if (key == 3) { //RIGHT
            current_digit = (current_digit == 9) ? 0 : current_digit + 1;
            password[digit_index] = '0' + current_digit;
            password[digit_index + 1] = '\0'; // Null-terminate the password string
            PRINTF("%s\n", password);
        } else if (key == 4) { //ENTER
            password[digit_index] = '0' + current_digit;
            password[digit_index + 1] = '\0'; // Null-terminate the password string
            digit_index++;
            current_digit = 0;
            PRINTF("%s\n", password);
        }
    }
}

void add_user_call() {
	PRINTF("Adding user\n");
	char id[20];
	char password[20];

	PRINTF("Enter id: ");
	read_id(id);

	PRINTF("Enter password: ");
	read_password(password);
	add_user(id, password);

	print_menu(state);

}

void delete_user_call() {
	PRINTF("Deleting user\n");
	char id[20];
	char password[20];

	read_id(id);
	read_password(password);
	delete_user(id, password);

	print_menu(state);


}

void change_password_call() {
	PRINTF("Changing password\n");
	char id[20];
	char old_password[20];

	read_id(id);
	PRINTF("Enter old password: ");
	read_password(old_password);

	char new_password[20];
	PRINTF("Enter new password: ");
	read_password(new_password);

	change_password(id, old_password, new_password);

	print_menu(state);
}

void access_system_call(void)
{
	PRINTF("Accessing system\n");
	char id[20];
	char password[20];
	read_id(id);
	read_password(password);
	uint8_t access = access_system(id, password);

	if (access)
	{
		PRINTF("Access granted\n");
	}
	else
	{
		PRINTF("Access denied\n");
	}

	print_menu(state);

}

