
// Main should be declared in application
extern int main();

// C code entry point
int __g_main()
{
	int returnCode;

	returnCode = main();

	return returnCode;	
}

// Called at program exit
int __g_exit()
{

}

