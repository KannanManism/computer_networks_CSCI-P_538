int main(int argc, char **argv){
char value = 'A';
char *address_of_value = &value;
char value2;
printf("Value stored at 0x%x has value %c\n", address_of_value, value);

value2 = *address_of_value;
printf("Value is also stored here 0x%x --> %c\n", &value2, value2);
return 0;
}